/**
  *  \file game/proxy/configurationeditorproxy.hpp
  *  \brief Class game::proxy::ConfigurationEditorProxy
  */
#ifndef C2NG_GAME_PROXY_CONFIGURATIONEDITORPROXY_HPP
#define C2NG_GAME_PROXY_CONFIGURATIONEDITORPROXY_HPP

#include <vector>
#include "afl/base/signal.hpp"
#include "game/config/configurationeditor.hpp"
#include "game/proxy/configurationeditoradaptor.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Configuration editor proxy.
        Bidirectional proxy to a game::config::ConfigurationEditor object.
        The underlying set of configuration options is defined by a ConfigurationEditorAdaptor.

        Synchronous:
        - retrieve current status (loadValues(), getValues()).
          Unlike most other proxies, ConfigurationEditorProxy stores a local copy of the received data.
          This simplifies most clients.

        Asynchronous:
        - modify configuration (setSource(), toggleValue(), setValue())
        - receive changes (sig_itemChange)

        Like ConfigurationEditor, this proxy only offers basic modification operations.
        The configuration can also be modified using other means (e.g., ConfigurationProxy).
        Those other changes are picked up by ConfigurationEditorProxy. */
    class ConfigurationEditorProxy {
     public:
        /** List of descriptions. */
        typedef std::vector<game::config::ConfigurationEditor::Info> Infos_t;

        /** Constructor.
            @param adaptorSender Adaptor to access underlying objects
            @param reply         RequestDispatcher to receive replies on*/
        ConfigurationEditorProxy(util::RequestSender<ConfigurationEditorAdaptor> adaptorSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~ConfigurationEditorProxy();

        /** Retrieve current values and store them locally.
            This applies a baseline for further sig_itemChange callbacks.
            Values can be accessed by getValues().
            @param [in,out] ind     WaitIndicator for UI synchronisation
            @see game::config::ConfigurationEditor::loadValues()
            @see game::config::ConfigurationEditor::Node::describe() */
        void loadValues(WaitIndicator& ind);

        /** Access previously-loaded values.
            Returns the values previously loaded using loadValues().
            These values are kept up-to-date when a change is processed.
            @return values */
        const Infos_t& getValues() const;

        /** Set source (storage location) of an option.
            If this is an actual change, this will eventually produce a sig_itemChange callback.
            @param index Index into loadValues() result
            @param src   New storage location
            @see game::config::ConfigurationEditor::Node::setSource() */
        void setSource(size_t index, game::config::ConfigurationOption::Source src);

        /** Toggle value of an option.
            If this is an actual change, this will eventually produce a sig_itemChange callback.
            @param index Index into loadValues() result for an IntegerOption
            @see game::config::ConfigurationEditor::Node::toggleValue() */
        void toggleValue(size_t index);

        /** Set value of an option.
            If this is an actual change, this will eventually produce a sig_itemChange callback.
            @param index Index into loadValues() result
            @param value New value
            @see game::config::ConfigurationEditor::Node::setSource() */
        void setValue(size_t index, String_t value);

        /** Signal: change.
            Upon change, called for each individual changed node.
            Each call updates one element of the loadValues() result.
            @param index Index
            @param info  New status */
        afl::base::Signal<void(size_t, const game::config::ConfigurationEditor::Info&)> sig_itemChange;

     private:
        class Trampoline;
        class TrampolineFromAdaptor;

        Infos_t m_infos;
        util::RequestReceiver<ConfigurationEditorProxy> m_receiver;
        util::RequestSender<Trampoline> m_sender;

        void emitItemChange(size_t index, game::config::ConfigurationEditor::Info info);
    };

} }

#endif
