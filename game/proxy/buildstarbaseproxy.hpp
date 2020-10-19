/**
  *  \file game/proxy/buildstarbaseproxy.hpp
  *  \brief Class game::proxy::BuildStarbaseProxy
  */
#ifndef C2NG_GAME_PROXY_BUILDSTARBASEPROXY_HPP
#define C2NG_GAME_PROXY_BUILDSTARBASEPROXY_HPP

#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "game/spec/cost.hpp"
#include "game/types.hpp"
#include "util/requestsender.hpp"
#include "util/slaverequestsender.hpp"

namespace game { namespace proxy {

    /** Starbase building proxy.

        Bidirectional, synchronous:
        - set up and retrieve status (init())
        - give command (commit())

        \see game::actions::BuildStarbase */
    class BuildStarbaseProxy {
     public:
        /** Possible results for this combination. */
        enum Mode {
            Error,                        /**< Error (preconditions not fulfilled). */
            CanBuild,                     /**< Starbase can be built. */
            CannotBuild,                  /**< Cannot build starbase due to lacking resources. */
            CanCancel                     /**< Previous build can be cancelled. */
        };

        /** Status of the action. */
        struct Status {
            Mode mode;                    /**< Mode/result. */
            game::spec::Cost available;   /**< Available resources (for mode=CanBuild/CannotBuild). \see game::actions::getAvailableAmountAsCost(). */
            game::spec::Cost cost;        /**< Starbase cost (for mode=CanBuild/CannotBuild). \see game::actions::getCost(). */
            game::spec::Cost remaining;   /**< Remaining resources (for mode=CanBuild/CannotBuild). \see game::actions::getRemainingAmountAsCost(). */
            game::spec::Cost missing;     /**< Missing resources (for mode=CanBuild/CannotBuild). \see game::actions::getMissingAmountAsCost(). */
            String_t errorMessage;        /**< Error message (for mode=Error). */

            Status()
                : mode(Error),
                  available(), cost(), remaining(), missing(), errorMessage()
                { }
        };

        /** Constructor.
            \param gameSender Game sender */
        explicit BuildStarbaseProxy(util::RequestSender<Session> gameSender);

        /** Destructor. */
        ~BuildStarbaseProxy();

        /** Initialize.
            \param [in]  link   WaitIndicator
            \param [in]  id     Planet Id
            \param [out] status Transaction status */
        void init(WaitIndicator& link, Id_t id, Status& status);

        /** Commit.
            Call if the previous init() reported a success status, i.e. CanBuild or CanCancel.
            Call is ignored if preconditions are not fulfilled.
            \param [in]  link   WaitIndicator */
        void commit(WaitIndicator& link);

     private:
        struct Trampoline;
        util::SlaveRequestSender<Session, Trampoline> m_sender;
    };

} }

#endif
