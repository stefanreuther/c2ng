/**
  *  \file test/game/proxy/processlistproxytest.cpp
  *  \brief Test for game::proxy::ProcessListProxy
  */

#include "game/proxy/processlistproxy.hpp"

#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.proxy.ProcessListProxy:basics", a)
{
    // Session thread with some processes
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
        a.checkEqual("01. size", result.size(), 3U);
        a.checkEqual("02. processId", result[0].processId, p1.getProcessId());
        a.checkEqual("03. name",      result[0].name,      "p1");
        a.checkEqual("04. status",    result[0].status,    "Suspended");
        a.checkEqual("05. processId", result[1].processId, p2.getProcessId());
        a.checkEqual("06. name",      result[1].name,      "p2");
        a.checkEqual("07. status",    result[1].status,    "Suspended");
        a.checkEqual("08. processId", result[2].processId, p3.getProcessId());
        a.checkEqual("09. name",      result[2].name,      "p3");
        a.checkEqual("10. status",    result[2].status,    "Suspended");
    }

    // Call setAllProcessState(); wait for change
    recv.infos.clear();
    testee.setAllProcessState(ProcessListEditor::Runnable);
    while (recv.infos.empty()) {
        a.check("11. wait", disp.wait(1000));
    }
    a.checkEqual("12. size", recv.infos.size(), 3U);
    a.checkEqual("13. status", recv.infos[0].status, "Runnable");
    a.checkEqual("14. status", recv.infos[1].status, "Runnable");
    a.checkEqual("15. status", recv.infos[2].status, "Runnable");

    // Call setProcessState
    recv.infos.clear();
    testee.setProcessState(p2.getProcessId(), ProcessListEditor::Suspended);
    while (recv.infos.empty()) {
        a.check("21. wait", disp.wait(1000));
    }
    a.checkEqual("22", recv.infos.size(), 3U);
    a.checkEqual("23", recv.infos[0].status, "Runnable");
    a.checkEqual("24", recv.infos[1].status, "Suspended");
    a.checkEqual("25", recv.infos[2].status, "Runnable");

    // Call setProcessPriority
    recv.infos.clear();
    testee.setProcessPriority(p2.getProcessId(), 10);
    while (recv.infos.empty()) {
        a.check("31. wait", disp.wait(1000));
    }
    a.checkEqual("32. size", recv.infos.size(), 3U);
    a.checkEqual("33. processId", recv.infos[0].processId, p2.getProcessId());
    a.checkEqual("34. processId", recv.infos[1].processId, p1.getProcessId());
    a.checkEqual("35. processId", recv.infos[2].processId, p3.getProcessId());

    // Pre-commit: states didn't change yet, but priorities did
    a.checkEqual("41. getState", p1.getState(), Process::Suspended);
    a.checkEqual("42. getState", p2.getState(), Process::Suspended);
    a.checkEqual("43. getState", p3.getState(), Process::Suspended);
    a.checkEqual("44. getPriority", p1.getPriority(), 50);
    a.checkEqual("45. getPriority", p2.getPriority(), 10);
    a.checkEqual("46. getPriority", p3.getPriority(), 50);

    // Commit; verify
    uint32_t pgid = testee.commit(ind);
    a.checkEqual("51. getState", p1.getState(), Process::Runnable);
    a.checkEqual("52. getState", p2.getState(), Process::Suspended);
    a.checkEqual("53. getState", p3.getState(), Process::Runnable);
    a.checkEqual("54. getProcessGroupId", p1.getProcessGroupId(), pgid);
    a.checkEqual("55. getProcessGroupId", p3.getProcessGroupId(), pgid);
}

/** Test resumeConfirmedProcesses().
    A: set up a process list and a confirmed notification. Invoke resumeConfirmedProcesses().
    E: process status updated correctly. */
AFL_TEST("game.proxy.ProcessListProxy:resumeConfirmedProcesses", a)
{
    // Session thread with some processes
    game::test::SessionThread s;
    interpreter::World& w = s.session().world();
    interpreter::Process& p1 = s.session().processList().create(w, "p1");
    interpreter::Process& p2 = s.session().processList().create(w, "p2");
    interpreter::Process& p3 = s.session().processList().create(w, "p3");

    // Messages
    game::interface::NotificationStore::Message* msg = s.session().notifications().addMessage(p2.getProcessId(), "header", "body", game::Reference());
    s.session().notifications().confirmMessage(msg, true);

    // Testee
    game::test::WaitIndicator ind;
    ProcessListProxy testee(s.gameSender(), ind);
    testee.resumeConfirmedProcesses();

    // Commit; verify
    uint32_t pgid = testee.commit(ind);
    a.checkEqual("01. getState", p1.getState(), Process::Suspended);
    a.checkEqual("02. getState", p2.getState(), Process::Runnable);
    a.checkEqual("03. getState", p3.getState(), Process::Suspended);
    a.checkEqual("04. getProcessGroupId", p2.getProcessGroupId(), pgid);
}
