
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

class BhvPenaltyKickKicker : rcsc::SoccerBehavior {
   public:
    bool execute(rcsc::PlayerAgent* agent) override;

    struct InitialStateKicker;
    struct Transition : sc::event<Transition> {};

    struct KickerStm : sc::state_machine<KickerStm, InitialStateKicker> {
        explicit KickerStm(rcsc::PlayerAgent* agent) : mAgent(agent) {}
        ~KickerStm() override = default;

        [[nodiscard]] rcsc::PlayerAgent* getAgent() const { return mAgent; }

       public:
        static inline bool stopTransition {false};
        rcsc::PlayerAgent* mAgent;
    };

    struct InitialStateKicker : sc::state<InitialStateKicker, KickerStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit InitialStateKicker(my_context ctx);
        ~InitialStateKicker() override;
        sc::result react(const Transition&);
    };

    struct SUpdateWorldModelKicker : sc::state<SUpdateWorldModelKicker, KickerStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SUpdateWorldModelKicker(my_context ctx);
        ~SUpdateWorldModelKicker() override;
        sc::result react(const Transition&);
    };

    struct J1Kicker : sc::state<J1Kicker, KickerStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J1Kicker(my_context ctx);
        ~J1Kicker() override;
        sc::result react(const Transition&);
    };

    struct FinalStateKicker : sc::state<FinalStateKicker, KickerStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit FinalStateKicker(my_context ctx);
        ~FinalStateKicker() override;
        sc::result react(const Transition&);
    };

    struct SGoToBall : sc::state<SGoToBall, KickerStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SGoToBall(my_context ctx);
        ~SGoToBall() override;
        sc::result react(const Transition&);
    };

    struct J2Kicker : sc::state<J2Kicker, KickerStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J2Kicker(my_context ctx);
        ~J2Kicker() override;
        sc::result react(const Transition&);
    };

    struct SShoot : sc::state<SShoot, KickerStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SShoot(my_context ctx);
        ~SShoot() override;
        sc::result react(const Transition&);
    };

    struct J3Kicker : sc::state<J3Kicker, KickerStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J3Kicker(my_context ctx);
        ~J3Kicker() override;
        sc::result react(const Transition&);
    };

    struct SDribble : sc::state<SDribble, KickerStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SDribble(my_context ctx);
        ~SDribble() override;
        sc::result react(const Transition&);
    };

    struct J4Kicker : sc::state<J4Kicker, KickerStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J4Kicker(my_context ctx);
        ~J4Kicker() override;
        sc::result react(const Transition&);
    };

    struct UndefinedStateKicker : sc::state<UndefinedStateKicker, KickerStm> {
        using reactions = sc::custom_reaction<Transition>;
        explicit UndefinedStateKicker(my_context ctx);
        ~UndefinedStateKicker() override;
        sc::result react(const Transition&);
    };
};