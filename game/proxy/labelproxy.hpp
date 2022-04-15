/**
  *  \file game/proxy/labelproxy.hpp
  *  \brief Class game::proxy::LabelProxy
  */
#ifndef C2NG_GAME_PROXY_LABELPROXY_HPP
#define C2NG_GAME_PROXY_LABELPROXY_HPP

#include "afl/base/optional.hpp"
#include "afl/base/signal.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Object label proxy.
        Provides access to a Session's game::interface::LabelExtra.

        Bidirectional, synchronous:
        - get current configuration: getConfiguration()

        Bidirectional, asynchronous:
        - change configuration: setConfiguration(), sig_configurationApplied */
    class LabelProxy {
     public:
        /** Label status. */
        struct Status {
            /** Ship error status.
                Nothing on success, otherwise, human-readable error message. */
            afl::base::Optional<String_t> shipError;

            /** Planet error status.
                Nothing on success, otherwise, human-readable error message. */
            afl::base::Optional<String_t> planetError;
        };

        /** Constructor.
            @param gameSender Game sender
            @param receiver   RequestDispatcher to receive updates in this thread */
        LabelProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver);

        /** Get active configuration.
            @param [in,out] ind         WaitIndicator for UI synchronisation
            @param [out]    shipExpr    Ship expression
            @param [out]    planetExpr  Planet expression */
        void getConfiguration(WaitIndicator& ind, String_t& shipExpr, String_t& planetExpr);

        /** Set configuration.
            Will asynchronously update the configuration and return a sig_configurationApplied on success.
            @param shipExpr     New ship expression; Nothing to leave unchanged.
            @param planetExpr   New planet expression; Nothing to leave unchanged. */
        void setConfiguration(afl::base::Optional<String_t> shipExpr, afl::base::Optional<String_t> planetExpr);

        /** Signal: confirm setConfiguration().
            @param st Status */
        afl::base::Signal<void(const Status&)> sig_configurationApplied;

     private:
        class Trampoline;
        class TrampolineFromSession;

        util::RequestReceiver<LabelProxy> m_receiver;
        util::RequestSender<Trampoline> m_sender;
    };

} }

#endif
