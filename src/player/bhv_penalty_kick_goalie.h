
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

namespace AuxFunc {
    void moveToBallPosition(rcsc::PlayerAgent *agent);
    void goToWaitPosition(rcsc::PlayerAgent *agent);
    bool doOneKickShoot(rcsc::PlayerAgent *agent);
    bool doGetBall(rcsc::PlayerAgent *agent);
    bool getShootTarget(const rcsc::PlayerAgent *agent, rcsc::Vector2D *point, double *firstSpeed);
    bool doShoot(rcsc::PlayerAgent *agent);
    bool doDribble(rcsc::PlayerAgent *agent);
}

namespace RSFunc {
    bool isInsidePenaltyArea(const rcsc::BallObject &ball);
    bool isInCatchableDistance(const rcsc::BallObject &ball);
    bool isCatchable(const rcsc::WorldModel &wm);
    bool checkTackle(double prob);
    rcsc::Vector2D calculateBlockPoint(rcsc::Vector2D ballPos);
}

namespace RSOutput {
    bool doCatch(rcsc::PlayerAgent* agent);
    bool doClearBall(rcsc::PlayerAgent* agent);
    bool doTackle(rcsc::PlayerAgent *agent);
}


class BhvPenaltyKickGoalie : rcsc::SoccerBehavior {

private:


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
        explicit GoalieOperation(rcsc::PlayerAgent* _agent) {
            agent = _agent;
        }
        ~GoalieOperation() override = default;
    };

    struct InitialStateGoalie : sc::state<InitialStateGoalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit InitialStateGoalie(my_context ctx) : my_base(ctx) {};
        sc::result react(const Transition&) {
            return transit<SdoCatch>();
        };
    };

    struct SdoCatch : sc::state<SdoCatch, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        SdoCatch(my_context ctx) : my_base(ctx) {
            catchable = RSFunc::isCatchable(agent->world());
        };
        sc::result react(const Transition&){
            return transit<J1Goalie>();
        };
    };

    struct J1Goalie : sc::state<J1Goalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        /*explicit J1Goalie(my_context ctx);*/
        /*~J1Goalie() override = default;*/
        sc::result react(const Transition&) {
            if(catchable && insideGoalieArea) {
                RSOutput::doCatch(agent);
                return transit<JFinal>();
            } else if(!(catchable && insideGoalieArea)) {
                return transit<SClearBall>();
            }

            return transit<UndefinedStateGoalie>();
        };
    };

    struct JFinal : sc::state<JFinal, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit JFinal(my_context ctx) : my_base(ctx) {};
        sc::result react(const Transition&) {
            return transit<FinalStateGoalie>();
        };
    };

    struct FinalStateGoalie : sc::simple_state<FinalStateGoalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit FinalStateGoalie() {
            //log state machine end
            std::cout << "End machine" << std::endl;
        };
        ~FinalStateGoalie() override;
        sc::result react(const Transition&) {
            return terminate();
        };
    };

    struct SClearBall: sc::state<SClearBall, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SClearBall(my_context ctx) : my_base(ctx) {};
        ~SClearBall() override;
        sc::result react(const Transition&) {
            return transit<J2Goalie>();
        };
    };

    struct J2Goalie : sc::state<J2Goalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J2Goalie(my_context ctx) : my_base(ctx) {};
        ~J2Goalie() override;
        sc::result react(const Transition&) {
            if(!agent->world().self().isKickable()) {
                return transit<SDoTackle>();
            } else if(agent->world().self().isKickable()) {
                RSOutput::doClearBall(agent);
                return transit<JFinal>();
            }

            return transit<UndefinedStateGoalie>();
        };
    };

    struct SDoTackle : sc::state<SDoTackle, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SDoTackle(my_context ctx) : my_base(ctx) {
            tacklePossible = RSFunc::checkTackle(agent->world().self().tackleProbability());
        };

        ~SDoTackle() override;
        sc::result react(const Transition&) {
            return transit<J3Goalie>();
        };
    };

    struct J3Goalie: sc::state<J3Goalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J3Goalie(my_context ctx) : my_base(ctx){};
        sc::result react(const Transition&) {
            if(tacklePossible) {
                RSOutput::doTackle(agent);
                return transit<JFinal>();
            } else if(! tacklePossible) {
                return transit<DoMove>();
            }

            return transit<UndefinedStateGoalie>();;
        }
    };

    struct DoMove : sc::state<DoMove, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit DoMove(my_context ctx): my_base(ctx) {
            blockPoint = RSFunc::calculateBlockPoint(agent->world().ball().pos());
        };
        sc::result react(const Transition&) {
            return transit<JFinal>();
        }
    };

    struct UndefinedStateGoalie : sc::state<UndefinedStateGoalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit UndefinedStateGoalie(my_context ctx): my_base(ctx) {};
        sc::result react(const Transition&);
    };
};
