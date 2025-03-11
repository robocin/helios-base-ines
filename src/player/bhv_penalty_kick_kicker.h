
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/result.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/transition.hpp>
#include <rcsc/player/soccer_action.h>
#include <boost/statechart/termination.hpp>
#include <rcsc/player/world_model.h>

namespace sc = boost::statechart;

class BhvPenaltyKickKicker : rcsc::SoccerBehavior {
   public:
    bool execute(rcsc::PlayerAgent* agent) override;

    struct InitialStateKicker;
    struct Transition : sc::event<Transition> {};

    struct KickerOperation : sc::state_machine<KickerOperation, InitialStateKicker> {
        explicit KickerOperation(rcsc::PlayerAgent* agent) : mAgent(agent) {}
        ~KickerOperation() override = default;

        [[nodiscard]] rcsc::PlayerAgent* getAgent() const { return mAgent; }
        [[nodiscard]] bool kickable() const { return isKickable; }
        [[nodiscard]] bool canShootToGoal() const { return canShoot; }

       public:
        static inline bool stopTransition {false};
        rcsc::PlayerAgent* mAgent;

        private:
            bool isKickable;
            bool canShoot;
    };

    struct InitialStateKicker : sc::state<InitialStateKicker, KickerOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit InitialStateKicker(my_context ctx);
        ~InitialStateKicker() override;
        sc::result react(const Transition&);
    };

    struct FinalStateKicker : sc::state<FinalStateKicker, KickerOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit FinalStateKicker(my_context ctx);
        ~FinalStateKicker() override;
        sc::result react(const Transition&);
    };

    struct SGoToBall : sc::state<SGoToBall, KickerOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SGoToBall(my_context ctx);
        ~SGoToBall() override;
        sc::result react(const Transition&);
    };

    struct J0Kicker : sc::state<J0Kicker, KickerOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J0Kicker(my_context ctx);
        ~J0Kicker() override;
        sc::result react(const Transition&);
    };

    struct SShoot : sc::state<SShoot, KickerOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SShoot(my_context ctx);
        ~SShoot() override;
        sc::result react(const Transition&);
    };

    struct J3Kicker : sc::state<J3Kicker, KickerOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J3Kicker(my_context ctx);
        ~J3Kicker() override;
        sc::result react(const Transition&);
    };

    struct SDribble : sc::state<SDribble, KickerOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit SDribble(my_context ctx);
        ~SDribble() override;
        sc::result react(const Transition&);
    };

    struct J4Kicker : sc::state<J4Kicker, KickerOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit J4Kicker(my_context ctx);
        ~J4Kicker() override;
        sc::result react(const Transition&);
    };

    struct UndefinedStateKicker : sc::state<UndefinedStateKicker, KickerOperation> {
        using reactions = sc::custom_reaction<Transition>;
        explicit UndefinedStateKicker(my_context ctx);
        ~UndefinedStateKicker() override;
        sc::result react(const Transition&);
    };
};