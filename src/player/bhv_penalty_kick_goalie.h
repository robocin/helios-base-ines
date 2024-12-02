
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/result.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/transition.hpp>
#include <rcsc/player/soccer_action.h>
#include <rcsc/player/world_model.h>

namespace sc = boost::statechart;

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
    };

    struct InitialStateGoalie : sc::state<InitialStateGoalie, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit InitialStateGoalie(my_context ctx);
        ~InitialStateGoalie() override;
        sc::result react(const Transition&);
    };

    struct SUpdateWorldModelKicker : sc::state<SUpdateWorldModelKicker, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SUpdateWorldModelKicker(my_context ctx);
        ~SUpdateWorldModelKicker() override;
        sc::result react(const Transition&);
    };

    struct J1Kicker : sc::state<J1Kicker, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J1Kicker(my_context ctx);
        ~J1Kicker() override;
        sc::result react(const Transition&);
    };

    struct FinalStateKicker : sc::state<FinalStateKicker, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit FinalStateKicker(my_context ctx);
        ~FinalStateKicker() override;
        sc::result react(const Transition&);
    };

    struct SGoToBall : sc::state<SGoToBall, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SGoToBall(my_context ctx);
        ~SGoToBall() override;
        sc::result react(const Transition&);
    };

    struct J2Kicker : sc::state<J2Kicker, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J2Kicker(my_context ctx);
        ~J2Kicker() override;
        sc::result react(const Transition&);
    };

    struct SShoot : sc::state<SShoot, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SShoot(my_context ctx);
        ~SShoot() override;
        sc::result react(const Transition&);
    };

    struct J3Kicker : sc::state<J3Kicker, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J3Kicker(my_context ctx);
        ~J3Kicker() override;
        sc::result react(const Transition&);
    };

    struct SDribble : sc::state<SDribble, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SDribble(my_context ctx);
        ~SDribble() override;
        sc::result react(const Transition&);
    };

    struct J4Kicker : sc::state<J4Kicker, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J4Kicker(my_context ctx);
        ~J4Kicker() override;
        sc::result react(const Transition&);
    };

    struct UndefinedStateKicker : sc::state<UndefinedStateKicker, GoalieStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit UndefinedStateKicker(my_context ctx);
        ~UndefinedStateKicker() override;
        sc::result react(const Transition&);
    };
};
