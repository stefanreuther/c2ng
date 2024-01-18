/**
  *  \file test/game/proxy/configurationobserverproxytest.cpp
  *  \brief Test for game::proxy::ConfigurationObserverProxy
  */

#include "game/proxy/configurationobserverproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"

using game::config::UserConfiguration;

namespace {
    template<typename T>
    class Receiver {
     public:
        Receiver(afl::test::Assert a, int id)
            : m_assert(a), m_id(id), m_value(), m_count()
            { }
        void onChange(int id, T value)
            {
                m_assert.checkEqual("onChange", id, m_id);
                m_value = value;
                ++m_count;
            }
        T get() const
            { return m_value; }
        size_t getCount() const
            { return m_count; }
     private:
        afl::test::Assert m_assert;
        const int m_id;
        T m_value;
        size_t m_count;
    };
}

/** Test observation of individual options. */
AFL_TEST("game.proxy.ConfigurationObserverProxy", a)
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
    Receiver<int32_t> intReceiver(a("intReceiver"), 99);
    testee.sig_intOptionChange.add(&intReceiver, &Receiver<int32_t>::onChange);
    testee.observeOption(99, UserConfiguration::Sim_NumThreads);

    // Connect string option
    Receiver<String_t> stringReceiver(a("stringReceiver"), 77);
    testee.sig_stringOptionChange.add(&stringReceiver, &Receiver<String_t>::onChange);
    testee.observeOption(77, UserConfiguration::Backup_Chart);

    // Verify initial value
    h.sync();
    ind.processQueue();
    a.checkEqual("01. get", intReceiver.get(), 3);
    a.checkEqual("02. getCount", intReceiver.getCount(), 1U);
    a.checkEqual("03. get", stringReceiver.get(), "a1");
    a.checkEqual("04. getCount", stringReceiver.getCount(), 1U);

    // Modify integer option, verify
    config[UserConfiguration::Sim_NumThreads].set(7);
    h.gameSender().postRequest(&game::Session::notifyListeners);
    h.sync();
    ind.processQueue();

    a.checkEqual("11. get", intReceiver.get(), 7);                   // changed
    a.checkEqual("12. getCount", intReceiver.getCount(), 2U);        // changed
    a.checkEqual("13. get", stringReceiver.get(), "a1");             // unchanged
    a.checkEqual("14. getCount", stringReceiver.getCount(), 1U);     // unchanged

    // Modify string option, verify
    config[UserConfiguration::Backup_Chart].set("qq");
    h.gameSender().postRequest(&game::Session::notifyListeners);
    h.sync();
    ind.processQueue();

    a.checkEqual("21. get", intReceiver.get(), 7);                   // unchanged
    a.checkEqual("22. getCount", intReceiver.getCount(), 2U);        // unchanged
    a.checkEqual("23. get", stringReceiver.get(), "qq");             // changed
    a.checkEqual("24. getCount", stringReceiver.getCount(), 2U);     // changed
}
