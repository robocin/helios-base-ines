#include "bhv_penalty_kick_kicker.h"

#include "strategy.h"

#include "setplay/bhv_go_to_placed_ball.h"

#include "basic_actions/body_kick_one_step.h"
#include "basic_actions/body_smart_kick.h"
#include "basic_actions/body_dribble.h"

#include "basic_actions/body_go_to_point.h"
#include "basic_actions/body_turn_to_point.h"
#include "basic_actions/neck_turn_to_point.h"
#include "basic_actions/neck_turn_to_relative.h"
#include "basic_actions/bhv_neck_body_to_ball.h"
#include "basic_actions/neck_turn_to_ball.h"
#include "basic_actions/body_turn_to_ball.h"
#include "basic_actions/neck_scan_field.h"

#include "basic_actions/body_intercept.h"
#include "basic_actions/body_stop_ball.h"

#include <rcsc/common/logger.h>
#include <rcsc/player/penalty_kick_state.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/world_model.h>

using namespace rcsc;

#define DEBUG_LOG

namespace
{

    void moveToBallPosition(PlayerAgent *agent)
    {
        AngleDeg placeAngle = 0.0;
        Vector2D goaliePos = (agent->world().ball().pos().x >= 0 ? Vector2D(ServerParam::i().pitchHalfLength(), 0) : Vector2D(-ServerParam::i().pitchHalfLength(), 0));

        if (!Bhv_GoToPlacedBall(placeAngle).execute(agent))
        {
            Body_TurnToPoint(goaliePos).execute(agent);
            agent->setNeckAction(new Neck_TurnToPoint(goaliePos));
        }
    }

    void goToWaitPosition(PlayerAgent *agent)
    {
        static const double DIST_STEP = 1.5;
        static const double SPACING_BETWEEN_PLAYERS = 0.8;
        static const double Y_AXIS_OFFSET = -9.8;
        static const double X_AXIS_OFFSET = 0;
        static const double START_X_POSITION = 7.0;

        const rcsc::WorldModel &wm = agent->world();

        rcsc::Vector2D me = wm.self().pos();
        rcsc::Vector2D ball = wm.ball().pos();

        int role = Strategy::i().roleNumber(wm.self().unum());

        Vector2D waitPos(sign(ball.x) * START_X_POSITION + sign(ball.x) * X_AXIS_OFFSET,
                         (Y_AXIS_OFFSET + DIST_STEP * role) * SPACING_BETWEEN_PLAYERS);

        if (role == 2 || role == 11)
        {
            waitPos.x *= 1.0;
        }
        else if (role == 3 || role == 10)
        {
            waitPos.x *= 0.9;
        }
        else if (role == 4 || role == 9)
        {
            waitPos.x *= 0.8;
        }
        else if (role == 5 || role == 8)
        {
            waitPos.x *= 0.7;
        }
        else if (role == 6 || role == 7)
        {
            waitPos.x *= 0.6;
        }

        if (agent->world().self().pos().dist(waitPos) < 0.7)
        {
            Bhv_NeckBodyToBall().execute(agent);
        }
        else
        {
            Body_GoToPoint(waitPos, 0.3, ServerParam::i().maxDashPower()).execute(agent);
            agent->setNeckAction(new Neck_TurnToRelative(0.0));
        }
    }

    bool doOneKickShoot(PlayerAgent *agent)
    {
        const double BALL_SPEED = agent->world().ball().vel().r();
        // ball is moving --> kick has taken.
        if (!ServerParam::i().penAllowMultKicks() && BALL_SPEED > 0.3)
        {
            return false;
        }

        // go to the ball side
        if (!agent->world().self().isKickable())
        {
            Body_GoToPoint(agent->world().ball().pos(), 0.4, ServerParam::i().maxDashPower())
                .execute(agent);
            agent->setNeckAction(new Neck_TurnToBall());
            return true;
        }

        // turn to the ball to get the maximal kick rate.
        if ((agent->world().ball().angleFromSelf() - agent->world().self().body()).abs() > 3.0)
        {
            Body_TurnToBall().execute(agent);
            const AbstractPlayerObject *oppGoalie = agent->world().getTheirGoalie();
            if (oppGoalie != nullptr)
            {
                agent->setNeckAction(new Neck_TurnToPoint(oppGoalie->pos()));
            }
            else
            {
                Vector2D goalC = ServerParam::i().theirTeamGoalPos();
                agent->setNeckAction(new Neck_TurnToPoint(goalC));
            }
            return true;
        }

        // decide shot target point
        Vector2D shootPoint = ServerParam::i().theirTeamGoalPos();

        const AbstractPlayerObject *oppGoalie = agent->world().getTheirGoalie();
        if (oppGoalie != nullptr)
        {
            shootPoint.y = ServerParam::i().goalHalfWidth() - 1.0;
            if (oppGoalie->pos().absY() > 0.5)
            {
                if (oppGoalie->pos().y > 0.0)
                {
                    shootPoint.y *= -1.0;
                }
            }
            else if (oppGoalie->bodyCount() < 2)
            {
                if (oppGoalie->body().degree() > 0.0)
                {
                    shootPoint.y *= -1.0;
                }
            }
        }

        // enforce one step kick
        Body_KickOneStep(shootPoint, ServerParam::i().ballSpeedMax()).execute(agent);

        return true;
    }

    bool doGetBall(PlayerAgent *agent)
    {

        const WorldModel &wm = agent->world();

        if (!wm.self().isKickable())
        {
            if (!Body_Intercept().execute(agent))
            {
                Body_GoToPoint(wm.ball().pos(), 0.4, ServerParam::i().maxDashPower()).execute(agent);
            }

            if (wm.ball().posCount() > 0)
            {
                agent->setNeckAction(new Neck_TurnToBall());
            }
            else
            {
                const AbstractPlayerObject *oppGoalie = agent->world().getTheirGoalie();
                if (oppGoalie != nullptr)
                {
                    agent->setNeckAction(new Neck_TurnToPoint(oppGoalie->pos()));
                }
                else
                {
                    agent->setNeckAction(new Neck_ScanField());
                }
            }

            return true;
        }

        return false;
    }

    bool getShootTarget(const PlayerAgent *agent, Vector2D *point, double *firstSpeed)
    {

        const int SHOOT_RESOLUTION = 30;
        const WorldModel &wm = agent->world();
        const ServerParam &sp = ServerParam::i();
        const AbstractPlayerObject *oppGoalie = wm.getTheirGoalie();
        Vector2D theirGoal;

        if (oppGoalie->pos().x > 0)
        {
            theirGoal = Vector2D(52.5, 0);
        }
        else
        {
            theirGoal = Vector2D(-52.5, 0);
        }

        if (theirGoal.dist2(wm.ball().pos()) > std::pow(35.0, 2))
        {
#ifdef DEBUG_LOG
            dlog.addText(Logger::TEAM, __FILE__ " (getShootTarget) too far");
#endif
            return false;
        }

        // goalie is not found.
        if (oppGoalie == nullptr)
        {
            Vector2D shotC = sp.theirTeamGoalPos();
            if (point != nullptr)
            {
                *point = shotC;
            }
            if (firstSpeed != nullptr)
            {
                *firstSpeed = sp.ballSpeedMax();
            }

#ifdef DEBUG_LOG
            dlog.addText(Logger::TEAM, __FILE__ " (getShootTarget) no goalie");
#endif
            return true;
        }

        int bestLOrR = 0;
        double bestSpeed = sp.ballSpeedMax() + 1.0;

        double postBuf = 1.0 + std::min(2.0, (sp.pitchHalfLength() - wm.self().pos().absX()) * 0.1);

        // consder only 2 angle
        Vector2D shotL(sp.pitchHalfLength(), -sp.goalHalfWidth() + postBuf);
        Vector2D shotR(sp.pitchHalfLength(), +sp.goalHalfWidth() - postBuf);

        const AngleDeg ANGLE_L = (shotL - wm.ball().pos()).th();
        const AngleDeg ANGLE_R = (shotR - wm.ball().pos()).th();

        // !!! Magic Number !!!
        double goalieMaxSpeed = 1.0;
        // default player speed max * conf decay
        double goalieDistBuf = goalieMaxSpeed * std::min(5, oppGoalie->posCount()) + sp.catchAreaLength() + 0.2;

        const int UNUM_OPPONENT_GOALIE = oppGoalie->unum();
        const PlayerType *playerTypeOpponentGoalie = wm.theirPlayerType(UNUM_OPPONENT_GOALIE);
        if (1 <= UNUM_OPPONENT_GOALIE && UNUM_OPPONENT_GOALIE <= 11 && (playerTypeOpponentGoalie != nullptr))
        {
            goalieMaxSpeed = playerTypeOpponentGoalie->realSpeedMax();

            goalieDistBuf = goalieMaxSpeed * std::min(5, oppGoalie->posCount()) + playerTypeOpponentGoalie->maxCatchableDist() + 0.2;
        }

        const Vector2D GOALIE_NEXT_POS = oppGoalie->pos() + oppGoalie->vel();

        Vector2D bestTarget(sp.pitchHalfLength(), 0.0);

        for (int i = 0; i <= SHOOT_RESOLUTION; ++i)
        {
            double shootRange = 6.0;

            double goalX;
            if (oppGoalie->pos().x > 0)
            {
                goalX = 52.5;
            }
            else
            {
                goalX = -52.5;
            }

            const Vector2D TARGET(goalX,
                                  shootRange - i * ((2.0 * shootRange) / SHOOT_RESOLUTION)); // MAGIC NUMBER
            const AngleDeg ANGLE = (TARGET - wm.ball().pos()).th();

            double dist2goal = wm.ball().pos().dist(TARGET);

            // set minimum speed to reach the goal line
            double tmpFirstSpeed = (dist2goal + 5.0) * (1.0 - sp.ballDecay());
            tmpFirstSpeed = std::max(1.2, tmpFirstSpeed);

            bool overMax = false;
            while (!overMax)
            {
                if (tmpFirstSpeed > sp.ballSpeedMax())
                {
                    overMax = true;
                    tmpFirstSpeed = sp.ballSpeedMax();
                }

                Vector2D ballPos = wm.ball().pos();
                Vector2D ballVel = Vector2D::polar2vector(tmpFirstSpeed, ANGLE);
                ballPos += ballVel;
                ballVel *= sp.ballDecay();

                bool goalieCanReach = false;

                // goalie move at first step is ignored (cycle is set to ZERO),
                // because goalie must look the ball velocity before chasing action.
                double cycle = 0.0;
                while (ballPos.absX() < sp.pitchHalfLength())
                {
                    if (GOALIE_NEXT_POS.dist(ballPos) < goalieMaxSpeed * cycle + goalieDistBuf)
                    {
                        goalieCanReach = true;
                        break;
                    }

                    ballPos += ballVel;
                    ballVel *= sp.ballDecay();
                    cycle += 1.0;
                }

                if (!goalieCanReach)
                {
                    if (tmpFirstSpeed < bestSpeed)
                    {
                        bestTarget = TARGET;
                        bestSpeed = tmpFirstSpeed;
                    }
                    break; // end of this angle
                }
                tmpFirstSpeed += 0.4;
            }
        }
        if (bestSpeed <= sp.ballSpeedMax())
        {
            if (point != nullptr)
            {
                *point = bestTarget;
            }
            if (firstSpeed != nullptr)
            {
                *firstSpeed = bestSpeed;
            }

            return true;
        }

        return false;
    }

    bool doShoot(PlayerAgent *agent)
    {
        const WorldModel &wm = agent->world();
        const PenaltyKickState *state = wm.penaltyKickState();

        if (wm.time().cycle() - state->time().cycle() > ServerParam::i().penTakenWait() - 25)
        {
#ifdef LOG_UP
            dlog.addText(Logger::TEAM,
                         __FILE__
                         " (doShoot) time limit. stateTime=%d spentTime=%d timeThr=%d force shoot.",
                         state->time().cycle(),
                         wm.time().cycle() - state->time().cycle(),
                         ServerParam::i().penTakenWait() - 25);
#endif
            return doOneKickShoot(agent);
        }

        Vector2D shotPoint;
        double shotSpeed;

        if (getShootTarget(agent, &shotPoint, &shotSpeed))
        {
#ifdef D_LOG
            dlog.addText(Logger::TEAM,
                         __FILE__ " (doShoot) shoot to (%.1f %.1f) speed=%f",
                         shot_point.x,
                         shot_point.y,
                         shot_speed);
#endif
            Body_SmartKick(shotPoint, shotSpeed, shotSpeed * 0.96, 2).execute(agent);
            agent->setNeckAction(new Neck_TurnToPoint(shotPoint));
            return true;
        }

        return false;
    }

    bool doDribble(PlayerAgent *agent)
    {
        static const int CONTINUAL_COUNT = 20;
        static int sTargetContinualCount = CONTINUAL_COUNT;
        Vector2D theirGoalPosition;

        const ServerParam &sp = ServerParam::i();
        const WorldModel &wm = agent->world();

        const double PENALTY_ABS_X = ServerParam::i().theirPenaltyAreaLineX();

        const AbstractPlayerObject *opp_goalie = wm.getTheirGoalie();
        const double GOALIE_MAX_SPEED = 1.0;

        if (opp_goalie->pos().x > 0)
        {
            theirGoalPosition = Vector2D(52.5, 0);
        }
        else
        {
            theirGoalPosition = Vector2D(-52.5, 0);
        }

        const Vector2D GOAL_C = theirGoalPosition;

        const double MY_ABS_X = wm.self().pos().absX();

        const double GOALIE_DIST = ((opp_goalie != nullptr) ? (opp_goalie->pos().dist(wm.self().pos()) - GOALIE_MAX_SPEED * std::min(5, opp_goalie->posCount())) : 200.0);
        const double GOALIE_ABS_X = ((opp_goalie != nullptr) ? opp_goalie->pos().absX() : 200.0);

        /////////////////////////////////////////////////
        // dribble parametors

        const double BASE_TARGET_ABS_Y = ServerParam::i().goalHalfWidth() + 4.0;
        Vector2D dribTarget = GOAL_C;
        double dribPower = ServerParam::i().maxDashPower();
        int dribDashes = 6;

        if (GOALIE_ABS_X > MY_ABS_X)
        {
            if (GOALIE_DIST < 4.0)
            {
                if (sTargetContinualCount == 1)
                {
                    sTargetContinualCount = -CONTINUAL_COUNT;
                }
                else if (sTargetContinualCount == -1)
                {
                    sTargetContinualCount = +CONTINUAL_COUNT;
                }
                else if (sTargetContinualCount > 0)
                {
                    sTargetContinualCount--;
                }
                else
                {
                    sTargetContinualCount++;
                }
            }

            if (sTargetContinualCount > 0)
            {
                if (agent->world().self().pos().y < -BASE_TARGET_ABS_Y + 2.0)
                {
                    dribTarget.y = BASE_TARGET_ABS_Y;
                }
                else
                {
                    dribTarget.y = -BASE_TARGET_ABS_Y;
                }
            }
            else
            {
                if (agent->world().self().pos().y > BASE_TARGET_ABS_Y - 2.0)
                {
                    dribTarget.y = -BASE_TARGET_ABS_Y;
                }
                else
                {
                    dribTarget.y = BASE_TARGET_ABS_Y;
                }
            }
            dribTarget.x = opp_goalie->pos().x > 0 ? GOALIE_ABS_X + 1 : -GOALIE_ABS_X - 1; // goalie_abs_x + 1.0;

            if (opp_goalie->pos().x > 0)
            {
                dribTarget.x = min_max(PENALTY_ABS_X - 2.0,
                                       dribTarget.x,
                                       ServerParam::i().pitchHalfLength() - 4.0);
            }
            else
            {
                dribTarget.x = min_max(PENALTY_ABS_X + 2.0,
                                       -dribTarget.x,
                                       ServerParam::i().pitchHalfLength() + 4.0);
                dribTarget.x *= -1;
            }

            double dashes = (agent->world().self().pos().dist(dribTarget) * 0.8 / ServerParam::i().defaultPlayerSpeedMax());
            dribDashes = static_cast<int>(floor(dashes));
            dribDashes = min_max(1, dribDashes, 6);
        }
        else
        {
            dribTarget = GOAL_C;
        }

        if ((opp_goalie != nullptr) && wm.self().pos().x < opp_goalie->pos().x && GOALIE_DIST < 5.0)
        {
            dribDashes = 3; // 6 MAGIC NUMBER
            static int sGoingTowardPlus = 20 * (wm.self().pos().y > 0.0 ? -1.0 : +1.0);
            static int sDecrement = wm.self().pos().y > 0.0 ? +1 : -1;

            if (-sp.goalAreaHalfWidth() < opp_goalie->pos().y && opp_goalie->pos().y < sp.goalAreaHalfWidth())
            {
                dribTarget.x = wm.self().pos().x;

                if (opp_goalie->pos().absY() < agent->world().self().pos().absY())
                {
                    dribTarget.y = opp_goalie->pos().y + sGoingTowardPlus > 0 ? +5.0 : -5.0;
                }
                else
                {
                    dribTarget.y = -1.0 * opp_goalie->pos().y;
                }
            }
            else
            {

                Vector2D focusedPost(sp.pitchHalfLength(),
                                     opp_goalie->pos().y > 0 ? sp.goalWidth() / 2.0 : -sp.goalWidth() / 2.0);

                Vector2D direction = opp_goalie->pos() - focusedPost;

                Vector2D adjustment1 = direction;
                adjustment1.setLength(1.0);
                Vector2D adjustment2 = direction;
                adjustment2.setLength(5.0);
                adjustment2.rotate(sGoingTowardPlus > 0 ? +90.0 : -90.0);

                dribTarget = opp_goalie->pos() + adjustment1 + adjustment2;
            }

            sGoingTowardPlus += sDecrement;
            if (sGoingTowardPlus == -20)
            {
                sDecrement = +1;
            }
            else if (sGoingTowardPlus == 20)
            {
                sDecrement = -1;
            }

#ifdef LOG_UP
            dlog.addText(Logger::TEAM,
                         __FILE__ ":%d dribble. avoid goalie. target=(%.1f, %.1f) s_g_t_p = %d",
                         __LINE__,
                         drib_target.x,
                         drib_target.y,
                         s_going_toward_plus);
#endif
            /*
        }
            */
            dlog.addText(Logger::TEAM,
                         __FILE__ ":%d dribble. goalie near. dashes=%d",
                         __LINE__,
                         dribDashes);
        }

        Vector2D targetRel = dribTarget - agent->world().self().pos();
        double buf = 2.0;
        if (dribTarget.absX() < PENALTY_ABS_X)
        {
            buf += 2.0;
        }

        if (targetRel.absX() < 5.0 && (opp_goalie == nullptr || opp_goalie->pos().dist(dribTarget) > targetRel.r() - buf))
        {
            if ((targetRel.th() - agent->world().self().body()).abs() < 5.0)
            {
                double firstSpeed = calc_first_term_geom_series_last(0.5,
                                                                     targetRel.r(),
                                                                     ServerParam::i().ballDecay());

                firstSpeed = std::min(firstSpeed, ServerParam::i().ballSpeedMax());
                Body_SmartKick(dribTarget, firstSpeed, firstSpeed * 0.96, 3).execute(agent);
            }
            else if ((agent->world().ball().rpos() + agent->world().ball().vel() - agent->world().self().vel())
                         .r() < agent->world().self().playerType().kickableArea() - 0.2)
            {
                Body_TurnToPoint(dribTarget).execute(agent);
            }
            else
            {
                Body_StopBall().execute(agent);
            }
        }
        else
        {

            bool dodgeMode = false;
            Body_Dribble(dribTarget, 2.0, dribPower, dribDashes, dodgeMode).execute(agent);
        }

        if (opp_goalie != nullptr)
        {
            agent->setNeckAction(new Neck_TurnToPoint(opp_goalie->pos()));
        }
        else
        {
            agent->setNeckAction(new Neck_ScanField());
        }

        return true;
    }

} // namespace

// Máquina ter um world model que vai receber o valor do PlayerAgent.
BhvPenaltyKickKicker::SUpdateWorldModelKicker::SUpdateWorldModelKicker(my_context ctx) : my_base(ctx) {}

BhvPenaltyKickKicker::SUpdateWorldModelKicker::~SUpdateWorldModelKicker() = default;

sc::result BhvPenaltyKickKicker::SUpdateWorldModelKicker::react(const Transition & /*unused*/)
{
#ifdef DEBUG_LOG
    dlog.addText(Logger::ACTION, ": SUpdateWorldModelKicker: Transition event received");
#endif
    return transit<J1Kicker>();
}

BhvPenaltyKickKicker::J1Kicker::J1Kicker(my_context ctx) : my_base(ctx) {}

BhvPenaltyKickKicker::J1Kicker::~J1Kicker() = default;

sc::result BhvPenaltyKickKicker::J1Kicker::react(const Transition & /*unused*/)
{
#ifdef DEBUG_LOG
    dlog.addText(Logger::ACTION, ": J1Kicker: Transition event received");
#endif
    return transit<SGoToBall>();
}

BhvPenaltyKickKicker::FinalStateKicker::FinalStateKicker(my_context ctx) : my_base(ctx) {}

BhvPenaltyKickKicker::FinalStateKicker::~FinalStateKicker() = default;

sc::result BhvPenaltyKickKicker::FinalStateKicker::react(const Transition & /*unused*/)
{
#ifdef DEBUG_LOG
    dlog.addText(Logger::ACTION, ": FinalStateKicker: Transition event received");
#endif
    return transit<SGoToBall>();
}

BhvPenaltyKickKicker::SGoToBall::SGoToBall(my_context ctx) : my_base(ctx) {}

BhvPenaltyKickKicker::SGoToBall::~SGoToBall() = default;

sc::result BhvPenaltyKickKicker::SGoToBall::react(const Transition & /*unused*/)
{
#ifdef DEBUG_LOG
    dlog.addText(Logger::ACTION, ": SGoToBall: Transition event received");
#endif
    return transit<J2Kicker>();
}

BhvPenaltyKickKicker::J2Kicker::J2Kicker(my_context ctx) : my_base(ctx) {}

BhvPenaltyKickKicker::J2Kicker::~J2Kicker() = default;

sc::result BhvPenaltyKickKicker::J2Kicker::react(const Transition & /*unused*/)
{
#ifdef DEBUG_LOG
    dlog.addText(Logger::ACTION, ": J2Kicker: Transition event received");
#endif
    auto *agent = context<KickerStm>().getAgent();
    const WorldModel &wm = agent->world();

    if (wm.gameMode().type() == GameMode::PenaltySetup_)
    {
        moveToBallPosition(agent);
        BhvPenaltyKickKicker::KickerStm::stopTransition = true; // temporary
        return transit<J4Kicker>();
    }

    if (wm.gameMode().type() != GameMode::PenaltySetup_)
    {
        return transit<SShoot>();
    }

    return transit<UndefinedStateKicker>();
}

BhvPenaltyKickKicker::SShoot::SShoot(my_context ctx) : my_base(ctx) {}

BhvPenaltyKickKicker::SShoot::~SShoot() = default;

sc::result BhvPenaltyKickKicker::SShoot::react(const Transition & /*unused*/)
{
#ifdef DEBUG_LOG
    dlog.addText(Logger::ACTION, ": SShoot: Transition event received");
#endif
    return transit<J3Kicker>();
}

BhvPenaltyKickKicker::J3Kicker::J3Kicker(my_context ctx) : my_base(ctx) {}

BhvPenaltyKickKicker::J3Kicker::~J3Kicker() = default;

sc::result BhvPenaltyKickKicker::J3Kicker::react(const Transition & /*unused*/)
{
#ifdef DEBUG_LOG
    dlog.addText(Logger::ACTION, ": J3Kicker: Transition event received");
#endif
    auto *agent = context<KickerStm>().getAgent();
    bool canShootToGoal = false;

    if (!ServerParam::i().penAllowMultKicks())
    {
        canShootToGoal |= doOneKickShoot(agent);
    }

    if (doGetBall(agent))
    {
        BhvPenaltyKickKicker::KickerStm::stopTransition = true;
        return transit<SDribble>();
    }

    canShootToGoal |= doShoot(agent);

    if (canShootToGoal)
    {
        BhvPenaltyKickKicker::KickerStm::stopTransition = true;
        return transit<J4Kicker>();
    }

    if (!canShootToGoal)
    {
        return transit<SDribble>();
    }

    return transit<UndefinedStateKicker>();
}

BhvPenaltyKickKicker::SDribble::SDribble(my_context ctx) : my_base(ctx)
{
    if (BhvPenaltyKickKicker::KickerStm::stopTransition)
    {
        return;
    }

    auto *agent = context<KickerStm>().getAgent();
    doDribble(agent);
}

BhvPenaltyKickKicker::SDribble::~SDribble() = default;

sc::result BhvPenaltyKickKicker::SDribble::react(const Transition & /*unused*/)
{
#ifdef DEBUG_LOG
    dlog.addText(Logger::ACTION, ": SDribble: Transition event received");
#endif
    BhvPenaltyKickKicker::KickerStm::stopTransition = true;
    return transit<J4Kicker>();
}

BhvPenaltyKickKicker::J4Kicker::J4Kicker(my_context ctx) : my_base(ctx) {}

BhvPenaltyKickKicker::J4Kicker::~J4Kicker() = default;

sc::result BhvPenaltyKickKicker::J4Kicker::react(const Transition & /*unused*/)
{
#ifdef DEBUG_LOG
    dlog.addText(Logger::ACTION, ": J4Kicker: Transition event received");
#endif
    return transit<SUpdateWorldModelKicker>();
}

BhvPenaltyKickKicker::UndefinedStateKicker::UndefinedStateKicker(my_context ctx) : my_base(ctx) {}

BhvPenaltyKickKicker::UndefinedStateKicker::~UndefinedStateKicker() = default;

sc::result BhvPenaltyKickKicker::UndefinedStateKicker::react(const Transition & /*unused*/)
{
#ifdef DEBUG_LOG
    dlog.addText(Logger::ACTION, ": UndefinedStateKicker: Transition event received");
#endif
    return transit<UndefinedStateKicker>();
}

void trigger(BhvPenaltyKickKicker::KickerStm &machine)
{
    while (!machine.terminated() && !BhvPenaltyKickKicker::KickerStm::stopTransition)
    {
        machine.process_event(BhvPenaltyKickKicker::Transition());
    }
}

bool BhvPenaltyKickKicker::execute(PlayerAgent *agent)
{
#ifdef DEBUG_LOG
    dlog.addText(Logger::ACTION, ": Starting penalty kick execution for kicker");
#endif

    const WorldModel &wm = agent->world();
    const PenaltyKickState *state = wm.penaltyKickState();
    int penaltyTakerOrder = (state->ourTakerCounter() - 1) % 11;
    int currTakerUnum = 11 - penaltyTakerOrder;

    if (state->currentTakerSide() != wm.ourSide() || agent->world().self().unum() != currTakerUnum)
    {
        goToWaitPosition(agent);
        return true;
    }

    static KickerStm kickerMachine(agent);
    static bool initialized = false;

    if (!initialized)
    {
        initialized = true;
        kickerMachine.initiate();
    }

    trigger(kickerMachine); // should be in another thread.
    BhvPenaltyKickKicker::KickerStm::stopTransition = false;
    return true;
}