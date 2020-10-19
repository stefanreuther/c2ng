/**
  *  \file u/t_game_proxy_mutexlistproxy.cpp
  *  \brief Test for game::proxy::MutexListProxy
  */

#include "game/proxy/mutexlistproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "interpreter/mutexcontext.hpp"
#include "interpreter/world.hpp"

using game::proxy::MutexListProxy;

void
TestGameProxyMutexListProxy::testIt()
{
    // Session thread
    game::test::SessionThread s;

    // Two processes with a mutex, one without
    interpreter::World& w = s.session().world();
    interpreter::Process& p1 = s.session().processList().create(w, "p1");
    p1.pushNewContext(new interpreter::MutexContext(w.mutexList().create("M1", "note 1", &p1)));

    interpreter::Process& p2 = s.session().processList().create(w, "p2");
    p2.pushNewContext(new interpreter::MutexContext(w.mutexList().create("M2", "note 2", &p2)));

    interpreter::Process& p3 = s.session().processList().create(w, "p3");

    TS_ASSERT_DIFFERS(p1.getProcessId(), p2.getProcessId());
    TS_ASSERT_DIFFERS(p1.getProcessId(), p3.getProcessId());
    TS_ASSERT_DIFFERS(p3.getProcessId(), p2.getProcessId());

    // Testee
    MutexListProxy testee(s.gameSender());
    game::test::WaitIndicator ind;

    // Get list of all mutexes
    {
        MutexListProxy::Infos_t result;
        testee.enumMutexes(ind, result);

        TS_ASSERT_EQUALS(result.size(), 2U);
        bool ok1 = false, ok2 = false;
        for (size_t i = 0; i < result.size(); ++i) {
            if (result[i].processId == p1.getProcessId()) {
                TS_ASSERT_EQUALS(result[i].name, "M1");
                ok1 = true;
            } else if (result[i].processId == p2.getProcessId()) {
                TS_ASSERT_EQUALS(result[i].name, "M2");
                ok2 = true;
            } else {
                TS_ASSERT(false);
            }
        }
        TS_ASSERT(ok1);
        TS_ASSERT(ok2);
    }

    // Get list of mutexes of p1
    {
        MutexListProxy::Infos_t result;
        testee.enumMutexes(ind, result, p1.getProcessId());

        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0].name, "M1");
        TS_ASSERT_EQUALS(result[0].processId, p1.getProcessId());
    }

    // Get list of mutexes of p3
    {
        MutexListProxy::Infos_t result;
        testee.enumMutexes(ind, result, p3.getProcessId());

        TS_ASSERT_EQUALS(result.size(), 0U);
    }
}

