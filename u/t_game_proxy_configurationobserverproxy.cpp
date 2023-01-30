/**
  *  \file u/t_game_proxy_configurationobserverproxy.cpp
  *  \brief Test for game::proxy::ConfigurationObserverProxy
  */

#include "game/proxy/configurationobserverproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/config/userconfiguration.hpp"

using game::config::UserConfiguration;

namespace {
    template<typename T>
    class Receiver {
     public:
        Receiver(int id)
            : m_id(id), m_value(), m_count()
            { }
        void onChange(int id, T value)
            {
                TS_ASSERT_EQUALS(id, m_id);
                m_value = value;
                ++m_count;
            }
        T get() const
            { return m_value; }
        size_t getCount() const
            { return m_count; }
     private:
        const int m_id;
        T m_value;
        size_t m_count;
    };
}

/** Test observation of individual options. */
void
TestGameProxyConfigurationObserverProxy::testIt()
{
    // Environment
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    UserConfiguration& config = h.session().getRoot()->userConfiguration();
    config[UserConfiguration::Backup_Chart].set("a1");
    config[UserConfiguration::Sim_NumThreads].set(3);

    // Testee
    game::test::WaitIndicator ind;
    game::proxy::ConfigurationObserverProxy testee(h.gameSender(), ind);

    // Connect integer option
    Receiver<int32_t> intReceiver(99);
    testee.sig_intOptionChange.add(&intReceiver, &Receiver<int32_t>::onChange);
    testee.observeOption(99, UserConfiguration::Sim_NumThreads);

    // Connect string option
    Receiver<String_t> stringReceiver(77);
    testee.sig_stringOptionChange.add(&stringReceiver, &Receiver<String_t>::onChange);
    testee.observeOption(77, UserConfiguration::Backup_Chart);

    // Verify initial value
    h.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(intReceiver.get(), 3);
    TS_ASSERT_EQUALS(intReceiver.getCount(), 1U);
    TS_ASSERT_EQUALS(stringReceiver.get(), "a1");
    TS_ASSERT_EQUALS(stringReceiver.getCount(), 1U);

    // Modify integer option, verify
    config[UserConfiguration::Sim_NumThreads].set(7);
    h.gameSender().postRequest(&game::Session::notifyListeners);
    h.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(intReceiver.get(), 7);              // changed
    TS_ASSERT_EQUALS(intReceiver.getCount(), 2U);        // changed
    TS_ASSERT_EQUALS(stringReceiver.get(), "a1");        // unchanged
    TS_ASSERT_EQUALS(stringReceiver.getCount(), 1U);     // unchanged

    // Modify string option, verify
    config[UserConfiguration::Backup_Chart].set("qq");
    h.gameSender().postRequest(&game::Session::notifyListeners);
    h.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(intReceiver.get(), 7);              // unchanged
    TS_ASSERT_EQUALS(intReceiver.getCount(), 2U);        // unchanged
    TS_ASSERT_EQUALS(stringReceiver.get(), "qq");        // changed
    TS_ASSERT_EQUALS(stringReceiver.getCount(), 2U);     // changed
}

