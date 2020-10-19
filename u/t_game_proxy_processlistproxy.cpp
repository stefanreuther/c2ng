/**
  *  \file u/t_game_proxy_processlistproxy.cpp
  *  \brief Test for game::proxy::ProcessListProxy
  */

#include "game/proxy/processlistproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "interpreter/world.hpp"
#include "util/simplerequestdispatcher.hpp"

using game::interface::ProcessListEditor;
using game::proxy::ProcessListProxy;
using interpreter::Process;

namespace {
    struct ChangeReceiver {
        ProcessListProxy::Infos_t infos;

        void onListChange(const ProcessListProxy::Infos_t& i)
            { infos = i; }
    };
}

/** Test ProcessListProxy.
    A: set up a process list. Invoke ProcessListProxy methods.
    E: verify correct update signalisation; verify correct behaviour. */
void
TestGameProxyProcessListProxy::testIt()
{
    // Session thread with some processes
    CxxTest::setAbortTestOnFail(true);
    game::test::SessionThread s;
    interpreter::World& w = s.session().world();
    interpreter::Process& p1 = s.session().processList().create(w, "p1");
    interpreter::Process& p2 = s.session().processList().create(w, "p2");
    interpreter::Process& p3 = s.session().processList().create(w, "p3");

    // Testee
    util::SimpleRequestDispatcher disp;
    game::test::WaitIndicator ind;
    ProcessListProxy testee(s.gameSender(), disp);

    ChangeReceiver recv;
    testee.sig_listChange.add(&recv, &ChangeReceiver::onListChange);

    // Read initial list
    {
        ProcessListProxy::Infos_t result;
        testee.init(ind, result);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0].processId, p1.getProcessId());
        TS_ASSERT_EQUALS(result[0].name,      "p1");
        TS_ASSERT_EQUALS(result[0].status,    "Suspended");
        TS_ASSERT_EQUALS(result[1].processId, p2.getProcessId());
        TS_ASSERT_EQUALS(result[1].name,      "p2");
        TS_ASSERT_EQUALS(result[1].status,    "Suspended");
        TS_ASSERT_EQUALS(result[2].processId, p3.getProcessId());
        TS_ASSERT_EQUALS(result[2].name,      "p3");
        TS_ASSERT_EQUALS(result[2].status,    "Suspended");
    }

    // Call setAllProcessState(); wait for change
    recv.infos.clear();
    testee.setAllProcessState(ProcessListEditor::Runnable);
    while (recv.infos.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.infos.size(), 3U);
    TS_ASSERT_EQUALS(recv.infos[0].status, "Runnable");
    TS_ASSERT_EQUALS(recv.infos[1].status, "Runnable");
    TS_ASSERT_EQUALS(recv.infos[2].status, "Runnable");

    // Call setProcessState
    recv.infos.clear();
    testee.setProcessState(p2.getProcessId(), ProcessListEditor::Suspended);
    while (recv.infos.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.infos.size(), 3U);
    TS_ASSERT_EQUALS(recv.infos[0].status, "Runnable");
    TS_ASSERT_EQUALS(recv.infos[1].status, "Suspended");
    TS_ASSERT_EQUALS(recv.infos[2].status, "Runnable");

    // Call setProcessPriority
    recv.infos.clear();
    testee.setProcessPriority(p2.getProcessId(), 10);
    while (recv.infos.empty()) {
        TS_ASSERT(disp.wait(1000));
    }
    TS_ASSERT_EQUALS(recv.infos.size(), 3U);
    TS_ASSERT_EQUALS(recv.infos[0].processId, p2.getProcessId());
    TS_ASSERT_EQUALS(recv.infos[1].processId, p1.getProcessId());
    TS_ASSERT_EQUALS(recv.infos[2].processId, p3.getProcessId());

    // Pre-commit: states didn't change yet, but priorities did
    TS_ASSERT_EQUALS(p1.getState(), Process::Suspended);
    TS_ASSERT_EQUALS(p2.getState(), Process::Suspended);
    TS_ASSERT_EQUALS(p3.getState(), Process::Suspended);
    TS_ASSERT_EQUALS(p1.getPriority(), 50);
    TS_ASSERT_EQUALS(p2.getPriority(), 10);
    TS_ASSERT_EQUALS(p3.getPriority(), 50);

    // Commit; verify
    uint32_t pgid = testee.commit(ind);
    TS_ASSERT_EQUALS(p1.getState(), Process::Runnable);
    TS_ASSERT_EQUALS(p2.getState(), Process::Suspended);
    TS_ASSERT_EQUALS(p3.getState(), Process::Runnable);
    TS_ASSERT_EQUALS(p1.getProcessGroupId(), pgid);
    TS_ASSERT_EQUALS(p3.getProcessGroupId(), pgid);
}

