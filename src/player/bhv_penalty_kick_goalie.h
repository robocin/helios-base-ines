
#include <boost/statechart/custom_reaction.hpp>
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
    bool doCatch(rcsc::PlayerAgent* agent);
    bool isInsidePenaltyArea(const rcsc::BallObject &ball);
    bool isInCatchableDistance(const rcsc::BallObject &ball);
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

    struct GoalieStm : sc::state_machine<GoalieStm, InitialStateGoalie> {
        explicit GoalieStm(rcsc::PlayerAgent* agent) : mAgent(agent) {}
        ~GoalieStm() override = default;

        [[nodiscard]] rcsc::PlayerAgent* getAgent() const { return mAgent; }

       public:
        static inline bool stopTransition {false};
        rcsc::PlayerAgent* mAgent;
        int count;
    };

    struct InitialStateGoalie : sc::state<InitialStateGoalie, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit InitialStateGoalie(my_context ctx) : my_base(ctx) {};
        ~InitialStateGoalie() override = default;
        sc::result react(const Transition&) {
            return transit<SUpdateWorldModelGoalie>();
        };
    };

    struct SUpdateWorldModelGoalie : sc::state<SUpdateWorldModelGoalie, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SUpdateWorldModelGoalie(my_context ctx) : my_base(ctx) {};
        ~SUpdateWorldModelGoalie() override = default;
        sc::result react(const Transition&) {
            return transit<J1Goalie>();
        };
    };

    struct J1Goalie : sc::state<J1Goalie, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J1Goalie(my_context ctx);
        ~J1Goalie() override = default;
        sc::result react(const Transition&) {
            rcsc::PlayerAgent* agent = context<GoalieStm>().getAgent();
            if(agent->world().gameMode().type() != rcsc::GameMode::PenaltyTaken_) {
                return transit<FinalStateGoalie>();
            }

            return transit<SdoCatch>();
        };
    };

    struct FinalStateGoalie : sc::state<FinalStateGoalie, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit FinalStateGoalie(my_context ctx);
        ~FinalStateGoalie() override;
        sc::result react(const Transition&);
    }; 

    struct SdoCatch : sc::state<SdoCatch, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        SdoCatch(my_context ctx) : my_base(ctx) {
            rcsc::PlayerAgent *agent = context<GoalieStm>().getAgent();
        };
        ~SdoCatch() override;
        sc::result react(const Transition&){
            return transit<J2Goalie>();
        };
    };

    struct J2Goalie : sc::state<J2Goalie, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J2Goalie(my_context ctx) : my_base(ctx) {};
        ~J2Goalie() override;
        sc::result react(const Transition&) {
            auto agent = context<GoalieStm>().getAgent();
            auto &ball = agent->world().ball();

            if(RSFunc::isInsidePenaltyArea(ball) && RSFunc::isInCatchableDistance(ball)) {
                RSOutput::doCatch(agent);
                return transit<FinalStateGoalie>();  
            }

            return transit<SClearBall>();
        };
    };

    struct SClearBall: sc::state<SClearBall, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SClearBall(my_context ctx) : my_base(ctx) {};
        ~SClearBall() override;
        sc::result react(const Transition&) {
            return transit<J4Goalie>();
        };
    };

    struct J4Goalie : sc::state<J4Goalie, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J4Goalie(my_context ctx) : my_base(ctx) {};
        ~J4Goalie() override;
        sc::result react(const Transition&) {
            auto agent = context<GoalieStm>().getAgent();
            auto &wm = agent->world();
            if(wm.self().isKickable()) {
                RSOutput::doClearBall(agent);
                return transit<J3Goalie>();  
            }

            return transit<SDoTackle>();
        };
    };

    struct SDoTackle : sc::state<SDoTackle, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SDoTackle(my_context ctx) : my_base(ctx) {};
        ~SDoTackle() override;
        sc::result react(const Transition&) {
            return transit<J5Goalie>();
        };
    };

    struct J5Goalie : sc::state<J5Goalie, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J5Goalie(my_context ctx) : my_base(ctx) {};
        ~J5Goalie() override;
        sc::result react(const Transition&) {
           //TODO: implement this
        };
    };
    

    struct J3Goalie: sc::state<J3Goalie, GoalieStm> { // EXEC JUNCTION
        using reactions = sc::custom_reaction<Transition>;
        explicit J3Goalie(my_context ctx) : my_base(ctx){};
        ~J3Goalie() override = default;
        sc::result react(const Transition&) {
            return transit<FinalStateGoalie>();
        }
    };

    struct UndefinedStateGoalie : sc::state<UndefinedStateGoalie, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit UndefinedStateGoalie(my_context ctx);
        ~UndefinedStateGoalie() override;
        sc::result react(const Transition&);
    };


};
