#ifndef BHV_PENALTY_KICK_GOALIE_H
#define BHV_PENALTY_KICK_GOALIE_H

#include <typeinfo>
#include "rcsc/geom/vector_2d.h"
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/termination.hpp>

#include <boost/statechart/event.hpp>
#include <boost/statechart/result.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/transition.hpp>
#include <rcsc/player/soccer_action.h>
#include <rcsc/player/world_model.h>
#include <rcsc/player/player_agent.h>

namespace sc = boost::statechart;

// namespace AuxFunc {
//     void moveToBallPosition(rcsc::PlayerAgent *agent);
//     void goToWaitPosition(rcsc::PlayerAgent *agent);
//     bool doOneKickShoot(rcsc::PlayerAgent *agent);
//     bool doGetBall(rcsc::PlayerAgent *agent);
//     bool getShootTarget(const rcsc::PlayerAgent *agent, rcsc::Vector2D *point, double *firstSpeed);
//     bool doShoot(rcsc::PlayerAgent *agent);
//     bool doDribble(rcsc::PlayerAgent *agent);
// }

bool isInsidePenaltyArea(const rcsc::PlayerAgent* agent);
bool isCatchable(const rcsc::WorldModel &wm);
bool checkTackle(double prob);
rcsc::Vector2D calculateBlockPoint(rcsc::Vector2D ballPos);


bool doCatch(rcsc::PlayerAgent* agent);
bool doClearBall(rcsc::PlayerAgent* agent);
bool doTackle(rcsc::PlayerAgent *agent);
bool doMove(rcsc::PlayerAgent *agent);


bool doGoalieBasicMove(rcsc::PlayerAgent* agent);
rcsc::Vector2D getGoalieMovePos(const rcsc::Vector2D& ball_pos, const rcsc::Vector2D& my_pos);
bool doGoalieSetup(rcsc::PlayerAgent* agent);
bool doGoalieSlideChase(rcsc::PlayerAgent* agent);

class BhvPenaltyKickGoalie : rcsc::SoccerBehavior {

private:
    int M_penalty_order[11];
    static int M_idx_penalty;

    // old functions
    void definePenaltyOrder();
    void resetPenaltyIndex();
    bool doKickerWait(rcsc::PlayerAgent* agent);
    bool doKickerSetup(rcsc::PlayerAgent* agent);
    bool doKickerReady(rcsc::PlayerAgent* agent);
    bool doKicker(rcsc::PlayerAgent* agent);
    bool doGetBall(rcsc::PlayerAgent* agent);
    // used only for one kick PK
    bool doOneKickShoot(rcsc::PlayerAgent* agent);
    // used only for multi kick PK
    bool doShoot(rcsc::PlayerAgent* agent);
    bool getShootTarget(const rcsc::PlayerAgent* agent, rcsc::Vector2D* point, double* first_speed);
    bool doDribble(rcsc::PlayerAgent* agent);

    bool doGoalieWait(rcsc::PlayerAgent* agent);
    bool doGoalie(rcsc::PlayerAgent* agent);

    
    bool isGoalieBallChaseSituation();
    

public:
// state machine variables
    static bool insideGoalieArea;
    static bool bodyInterceptAct;
    static bool tacklePossible;
    static bool catchable;
    static rcsc::Vector2D blockPoint;

    static rcsc::PlayerAgent* agent;

    bool execute(rcsc::PlayerAgent* agent) override;

    struct InitialStateGoalie;
    struct Transition : sc::event<Transition> {};

    struct GoalieOperation : sc::state_machine<GoalieOperation, InitialStateGoalie> {
        explicit GoalieOperation(rcsc::PlayerAgent* _agent);
        ~GoalieOperation() override = default;
    };

    struct UndefinedStateGoalie : sc::simple_state<UndefinedStateGoalie, GoalieOperation> {
        explicit UndefinedStateGoalie();
    };

    struct InitialStateGoalie : sc::state<InitialStateGoalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit InitialStateGoalie(my_context ctx);
        sc::result react(const Transition&);
    };

    struct SdoCatch : sc::state<SdoCatch, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SdoCatch(my_context ctx);
        sc::result react(const Transition&);
    };

    struct J1Goalie : sc::simple_state<J1Goalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        sc::result react(const Transition&);
    };

    struct JFinal : sc::simple_state<JFinal, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        sc::result react(const Transition&);
    };

    struct FinalStateGoalie : sc::simple_state<FinalStateGoalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit FinalStateGoalie();
        ~FinalStateGoalie() override = default;
        sc::result react(const Transition&);
    };

    struct SClearBall: sc::state<SClearBall, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SClearBall(my_context ctx);
        ~SClearBall() override = default;
        sc::result react(const Transition&);
    };

    struct J2Goalie : sc::simple_state<J2Goalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        sc::result react(const Transition&);
    };

    struct SDoTackle : sc::state<SDoTackle, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SDoTackle(my_context ctx);
        ~SDoTackle() override = default;
        sc::result react(const Transition&);
    };

    struct J3Goalie: sc::simple_state<J3Goalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        sc::result react(const Transition&);
    };

    struct DoMove : sc::state<DoMove, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit DoMove(my_context ctx);
        sc::result react(const Transition&);
    };
};

#endif