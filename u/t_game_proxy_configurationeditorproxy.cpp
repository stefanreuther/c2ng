/**
  *  \file u/t_game_proxy_configurationeditorproxy.cpp
  *  \brief Test for game::proxy::ConfigurationEditorProxy
  */

#include "game/proxy/configurationeditorproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/waitindicator.hpp"
#include "util/requestthread.hpp"
#include "afl/sys/log.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/booleanvalueparser.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/test/sessionthread.hpp"
#include "game/actions/preconditions.hpp"
#include "game/test/root.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/configurationobserverproxy.hpp"

namespace {
    static const game::config::IntegerOptionDescriptor opt1 = { "o1", &game::config::BooleanValueParser::instance };
    static const game::config::IntegerOptionDescriptor opt2 = { "o2", &game::config::IntegerValueParser::instance };

    /* TestAdaptor: standalone adaptor for testing */
    class TestAdaptor : public game::proxy::ConfigurationEditorAdaptor {
     public:
        TestAdaptor()
            : m_config()
            {
                m_config[opt1].set(1);
                m_config[opt1].setSource(game::config::ConfigurationOption::User);
                m_config[opt2].set(30);
                m_config[opt2].setSource(game::config::ConfigurationOption::Game);
                m_editor.addToggle(0, "toggle 1", opt1);
                m_editor.addGeneric(0, "generic 2", 333, "(value)")
                    .addOption(opt2);
            }
        virtual game::config::Configuration& config()
            { return m_config; }
        virtual game::config::ConfigurationEditor& editor()
            { return m_editor; }
        virtual afl::string::Translator& translator()
            { return m_translator; }
        virtual void notifyListeners()
            { m_config.notifyListeners(); }
     private:
        game::config::Configuration m_config;
        game::config::ConfigurationEditor m_editor;
        afl::string::NullTranslator m_translator;
    };

    /* SessionAdaptor: adaptor for accessing a Session's UserConfiguration */
    class SessionAdaptor : public game::proxy::ConfigurationEditorAdaptor {
     public:
        SessionAdaptor(game::Session& session)
            : m_session(session),
              m_editor()
            {
                m_editor.addToggle(0, "toggle", opt1);
                config()[opt1].set(1);
                notifyListeners();       // flush out changes
            }
        virtual game::config::Configuration& config()
            { return game::actions::mustHaveRoot(m_session).userConfiguration(); }
        virtual game::config::ConfigurationEditor& editor()
            { return m_editor; }
        virtual afl::string::Translator& translator()
            { return m_session.translator(); }
        virtual void notifyListeners()
            { m_session.notifyListeners(); }
     private:
        game::Session& m_session;
        game::config::ConfigurationEditor m_editor;
    };

    /* Converter to create a SessionAdaptor */
    class SessionAdaptorFromSession : public afl::base::Closure<game::proxy::ConfigurationEditorAdaptor*(game::Session&)> {
     public:
        virtual SessionAdaptor* call(game::Session& session)
            { return new SessionAdaptor(session); }
    };

    /* Receiver for ConfigurationEditor changes */
    class ChangeReceiver {
     public:
        ChangeReceiver()
            : m_lastIndex(9999), m_lastInfo()
            { }
        void onItemChange(size_t index, const game::config::ConfigurationEditor::Info& info)
            { m_lastIndex = index; m_lastInfo = info; }
        size_t getLastIndex() const
            { return m_lastIndex; }
        const game::config::ConfigurationEditor::Info& getLastInfo() const
            { return m_lastInfo; }
     private:
        size_t m_lastIndex;
        game::config::ConfigurationEditor::Info m_lastInfo;
    };

    /* Receiver for ConfigurationObserverProxy changes */
    class ObserverReceiver {
     public:
        ObserverReceiver()
            : m_lastIndex(9999), m_lastValue(9999)
            { }
        void onChange(int index, int value)
            { m_lastIndex = index; m_lastValue = value; }
        int getLastIndex() const
            { return m_lastIndex; }
        int getLastValue() const
            { return m_lastValue; }
     private:
        int m_lastIndex;
        int m_lastValue;
    };
}

/** Test behaviour on empty session.
    Adaptor has no way to report unavailability of an object other than throwing.
    Verify that this leads to sensible behaviour of the proxy. */
void
TestGameProxyConfigurationEditorProxy::testEmpty()
{
    // An adaptor that refuses every call
    class NullAdaptor : public game::proxy::ConfigurationEditorAdaptor {
     public:
        virtual game::config::Configuration& config()
            { throw std::runtime_error("nope"); }
        virtual game::config::ConfigurationEditor& editor()
            { throw std::runtime_error("nope"); }
        virtual afl::string::Translator& translator()
            { throw std::runtime_error("nope"); }
        virtual void notifyListeners()
            { }
    };
    NullAdaptor ad;
    game::test::WaitIndicator ind;

    // Use the actual RequestThread because that's what the code is gonna use
    // (In contrast to game::test::WaitIndicator, RequestThread swallows exceptions which is relevant here.)
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    util::RequestThread t("testEmpty", log, tx);
    util::RequestReceiver<game::proxy::ConfigurationEditorAdaptor> recv(t, ad);

    // Test object
    game::proxy::ConfigurationEditorProxy testee(recv.getSender(), ind);

    // Verify sensible operation (no lock-up, no crash)
    // - fire-and-forget
    testee.toggleValue(0);
    testee.setValue(0, "x");

    // - sync
    testee.loadValues(ind);
    TS_ASSERT_EQUALS(testee.getValues().size(), 0U);
}

/** Test normal behaviour.
    Exercise some configuraiton retrieval, modification, and events. */
void
TestGameProxyConfigurationEditorProxy::testNormal()
{
    // Environment
    TestAdaptor ad;
    game::test::WaitIndicator ind;
    util::RequestReceiver<game::proxy::ConfigurationEditorAdaptor> recv(ind, ad);

    // Test object
    game::proxy::ConfigurationEditorProxy testee(recv.getSender(), ind);

    // Query content
    testee.loadValues(ind);
    const game::proxy::ConfigurationEditorProxy::Infos_t& infos = testee.getValues();
    TS_ASSERT_EQUALS(infos.size(), 2U);
    TS_ASSERT_EQUALS(infos[0].name, "toggle 1");
    TS_ASSERT_EQUALS(infos[0].value, "Yes");
    TS_ASSERT_EQUALS(infos[1].name, "generic 2");
    TS_ASSERT_EQUALS(infos[1].value, "(value)");

    // Verify modification / events
    ChangeReceiver cr;
    testee.sig_itemChange.add(&cr, &ChangeReceiver::onItemChange);
    testee.setValue(0, "0");
    ind.processQueue();
    TS_ASSERT_EQUALS(cr.getLastIndex(), 0U);
    TS_ASSERT_EQUALS(cr.getLastInfo().value, "No");

    testee.toggleValue(0);
    ind.processQueue();
    TS_ASSERT_EQUALS(cr.getLastIndex(), 0U);
    TS_ASSERT_EQUALS(cr.getLastInfo().value, "Yes");

    testee.setSource(1, game::config::ConfigurationOption::User);
    ind.processQueue();
    TS_ASSERT_EQUALS(cr.getLastIndex(), 1U);
    TS_ASSERT_EQUALS(cr.getLastInfo().source, game::config::ConfigurationEditor::User);
}

/** Test integration with outside changes.
    Changes done by a ConfigurationProxy must be reported to ConfigurationEditorProxy correctly. */
void
TestGameProxyConfigurationEditorProxy::testIntegration()
{
    // Environment:
    game::test::SessionThread t;
    t.session().setRoot(new game::test::Root(game::HostVersion()));
    game::test::WaitIndicator ind;

    // Set up a ConfigurationEditorProxy
    game::proxy::ConfigurationEditorProxy testee(t.gameSender().makeTemporary(new SessionAdaptorFromSession()), ind);
    testee.loadValues(ind);
    const game::proxy::ConfigurationEditorProxy::Infos_t& infos = testee.getValues();
    TS_ASSERT_EQUALS(infos.size(), 1U);
    TS_ASSERT_EQUALS(infos[0].value, "Yes");

    // Observe changes
    ChangeReceiver cr;
    testee.sig_itemChange.add(&cr, &ChangeReceiver::onItemChange);

    // Use ConfigurationProxy to modify the configuration
    game::proxy::ConfigurationProxy(t.gameSender()).setOption(opt1, 0);
    t.gameSender().postRequest(&game::Session::notifyListeners);           // Must explicitly flush!
    t.sync();
    ind.processQueue();

    // Change must be reported on ConfigurationEditorProxy
    TS_ASSERT_EQUALS(cr.getLastIndex(), 0U);
    TS_ASSERT_EQUALS(cr.getLastInfo().value, "No");
}

/** Test integration with outside observers.
    Changes done by a ConfigurationEditorProxy must be reported to ConfigurationObserverProxy correctly. */
void
TestGameProxyConfigurationEditorProxy::testObserverIntegration()
{
    // Environment:
    game::test::SessionThread t;
    t.session().setRoot(new game::test::Root(game::HostVersion()));
    game::test::WaitIndicator ind;

    // Set up a ConfigurationEditorProxy
    game::proxy::ConfigurationEditorProxy testee(t.gameSender().makeTemporary(new SessionAdaptorFromSession()), ind);

    // Set up a ConfigurationObserverProxy
    game::proxy::ConfigurationObserverProxy observer(t.gameSender(), ind);
    ObserverReceiver recv;
    observer.sig_intOptionChange.add(&recv, &ObserverReceiver::onChange);
    observer.observeOption(42, opt1);

    // Modify using ConfigurationEditorProxy
    testee.toggleValue(0);
    t.sync();
    ind.processQueue();

    // Change must be reported on ConfigurationObserverProxy
    TS_ASSERT_EQUALS(recv.getLastIndex(), 42);
    TS_ASSERT_EQUALS(recv.getLastValue(), 0);
}

