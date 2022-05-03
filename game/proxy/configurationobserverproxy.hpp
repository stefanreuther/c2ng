/**
  *  \file game/proxy/configurationobserverproxy.hpp
  *  \brief Class game::proxy::ConfigurationObserverProxy
  */
#ifndef C2NG_GAME_PROXY_CONFIGURATIONOBSERVERPROXY_HPP
#define C2NG_GAME_PROXY_CONFIGURATIONOBSERVERPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Proxy to access user configuration items, with change notification.
        Extends ConfigurationProxy with the ability to receive configuration asynchronously.

        To use,
        - register listeners
        - call observeOption() for the desired option

        Each observeOption() is associated with an Id that is also reported in the respective change event.
        Ids are allocated by the user and are local to each ConfigurationObserverProxy instance. */
    class ConfigurationObserverProxy : public ConfigurationProxy {
     public:
        /** Constructor.
            @param gameSender Game sender
            @param reply      RequestDispatcher to receive replies on*/
        ConfigurationObserverProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Observe integer option.
            This will immediately report the option's current value in sig_intOptionChange, and whenever it changes.
            @param id   User-defined Id; reported as sig_intOptionChange's first parameter
            @param opt  Option descriptor for UserConfiguration; must be statically allocated */
        void observeOption(int id, const game::config::IntegerOptionDescriptor& opt);

        /** Observe string option.
            This will immediately report the option's current value in sig_stringOptionChange, and whenever it changes.
            @param id   User-defined Id; reported as sig_stringOptionChange's first parameter
            @param opt  Option descriptor for UserConfiguration; must be statically allocated */
        void observeOption(int id, const game::config::StringOptionDescriptor& opt);

        /** Signal: integer option value.
            @param id     User-defined from matching observeOption call
            @param value  Option value */
        afl::base::Signal<void(int, int32_t)> sig_intOptionChange;

        /** Signal: string option value.
            @param id     User-defined from matching observeOption call
            @param value  Option value */
        afl::base::Signal<void(int, String_t)> sig_stringOptionChange;

     private:
        class BaseObserver;
        class Trampoline;
        class TrampolineFromSession;

        template<typename Descriptor, typename Value>
        class ScalarObserver;

        util::RequestReceiver<ConfigurationObserverProxy> m_receiver;
        util::RequestSender<Trampoline> m_sender;

        void emitChange(int id, int32_t value);
        void emitChange(int id, String_t value);
    };

} }

#endif
