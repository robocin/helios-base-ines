
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

namespace RSFunc {
    bool isInsidePenaltyArea(const rcsc::BallObject &ball);
    bool isInCatchableDistance(const rcsc::BallObject &ball);
    bool isCatchable(const rcsc::WorldModel &wm);
    bool checkTackle(double prob);
    Point calculateBlockPoint(Point ball, Point goalPos);
}

typedef struct point {
    int x;
    int y;
} Point;

namespace RSOutput {
    bool doCatch(rcsc::PlayerAgent* agent);
    bool doClearBall(rcsc::PlayerAgent* agent);
    bool doTackle(rcsc::PlayerAgent *agent);
    Point getBall(rcsc::PlayerAgent *agent) {
        auto pos = agent->world().ball().pos();
        Point _pos = {pos.x, pos.y};
        return _pos;
    }
}


class BhvPenaltyKickGoalie : rcsc::SoccerBehavior {
   public:
    bool execute(rcsc::PlayerAgent* agent) override;

    struct InitialStateGoalie;
    struct Transition : sc::event<Transition> {};

    struct GoalieOperation : sc::state_machine<GoalieOperation, InitialStateGoalie> {
        explicit GoalieOperation(rcsc::PlayerAgent* agent) : mAgent(agent) {}
        ~GoalieOperation() override = default;

        [[nodiscard]] rcsc::PlayerAgent* getAgent() const { return mAgent; }
        [[nodiscard]] bool* isInsideGoalieArea() const { return insideGoalieArea; }
        [[nodiscard]] bool* isBodyInterceptAct() const { return bodyInterceptAct; }
        [[nodiscard]] bool* isTacklePossible() const { return tacklePossible; }
        [[nodiscard]] bool* isCatchable() const { return catchable; }
        [[nodiscard]] Point* getBlockPoint() const { return blockPoint; }

        public:
            static inline bool stopTransition {false};
            rcsc::PlayerAgent* mAgent;
            int count;
            
        private:
            bool *insideGoalieArea;
            bool *bodyInterceptAct;
            bool *tacklePossible;
            bool *catchable;
            Point *blockPoint;

    };

    struct InitialStateGoalie : sc::state<InitialStateGoalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit InitialStateGoalie(my_context ctx) : my_base(ctx) {};
        ~InitialStateGoalie() override = default;
        sc::result react(const Transition&) {
            return transit<SdoCatch>();
        };
    };

    struct SdoCatch : sc::state<SdoCatch, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        SdoCatch(my_context ctx) : my_base(ctx) {
            rcsc::PlayerAgent *agent = context<GoalieOperation>().getAgent();
            bool* insideGoalieArea = context<GoalieOperation>().isInsideGoalieArea();
            *insideGoalieArea = RSFunc::isInsidePenaltyArea(agent->world().ball());

            bool* catchable = context<GoalieOperation>().isCatchable();
            *catchable = RSFunc::isCatchable(agent->world());
        };
        ~SdoCatch() override;
        sc::result react(const Transition&){
            return transit<J1Goalie>();
        };
    };

    struct J1Goalie : sc::state<J1Goalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J1Goalie(my_context ctx);
        ~J1Goalie() override = default;
        sc::result react(const Transition&) {
            rcsc::PlayerAgent* agent = context<GoalieOperation>().getAgent();
            bool* insideGoalieArea = context<GoalieOperation>().isInsideGoalieArea();
            bool* catchable = context<GoalieOperation>().isCatchable();

            if(*catchable && *insideGoalieArea) {
                RSOutput::doCatch(agent);
                return transit<JFinal>();
            }

            return transit<SClearBall>();
        };
    };

    struct JFinal : sc::state<JFinal, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit JFinal(my_context ctx) : my_base(ctx) {};
        ~JFinal() override;
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
            auto agent = context<GoalieOperation>().getAgent();

            if(!agent->world().self().isKickable()) {
                return transit<SDoTackle>();  
            } else if(agent->world().self().isKickable()) {
                RSOutput::doClearBall(agent);
                return transit<JFinal>();

            }
        };
    };

    struct SDoTackle : sc::state<SDoTackle, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SDoTackle(my_context ctx) : my_base(ctx) {
            rcsc::PlayerAgent *agent = context<GoalieOperation>().getAgent();
            bool* tacklePossible = context<GoalieOperation>().isTacklePossible();
            *tacklePossible = RSFunc::checkTackle(agent->world().self().tackleProbability());
        };
        ~SDoTackle() override;
        sc::result react(const Transition&) {
            return transit<J3Goalie>();
        };
    };

    struct J3Goalie: sc::state<J3Goalie, GoalieOperation> { // EXEC JUNCTION
        using reactions = sc::custom_reaction<Transition>;
        explicit J3Goalie(my_context ctx) : my_base(ctx){};
        ~J3Goalie() override = default;
        sc::result react(const Transition&) {
            rcsc::PlayerAgent* agent = context<GoalieOperation>().getAgent();
            bool *tacklePossible = context<GoalieOperation>().isTacklePossible();

            if(*tacklePossible) {
                RSOutput::doTackle(agent);
                return transit<JFinal>();
            } else if(! *tacklePossible) {
                return transit<DoMove>();    
            }

            return transit<UndefinedStateGoalie>();;
        }
    };

    struct DoMove : sc::state<DoMove, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit DoMove(my_context ctx): my_base(ctx) {
            rcsc::PlayerAgent *agent = context<GoalieOperation>().getAgent();
            Point* blockPoint = context<GoalieOperation>().getBlockPoint();
            *blockPoint = RSFunc::calculateBlockPoint(RSOutput::getBall(agent), Point{0, 0});
        };
        ~DoMove() override;
        sc::result react(const Transition&) {
            return transit<JFinal>();
        }
    };

    struct UndefinedStateGoalie : sc::state<UndefinedStateGoalie, GoalieOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit UndefinedStateGoalie(my_context ctx);
        ~UndefinedStateGoalie() override;
        sc::result react(const Transition&);
    };


};
