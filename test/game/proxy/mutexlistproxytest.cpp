/**
  *  \file test/game/proxy/mutexlistproxytest.cpp
  *  \brief Test for game::proxy::MutexListProxy
  */

#include "game/proxy/mutexlistproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "interpreter/mutexcontext.hpp"
#include "interpreter/world.hpp"

using game::proxy::MutexListProxy;

AFL_TEST("game.proxy.MutexListProxy", a)
{
    // Session thread
    game::test::SessionThread s;

    // Two processes with a mutex, one without
    interpreter::World& w = s.session().world();
    interpreter::Process& p1 = s.session().processList().create(w, "p1");
    p1.pushNewContext(new interpreter::MutexContext("M1", "note 1"));

    interpreter::Process& p2 = s.session().processList().create(w, "p2");
    p2.pushNewContext(new interpreter::MutexContext("M2", "note 2"));

    interpreter::Process& p3 = s.session().processList().create(w, "p3");

    a.checkDifferent("01. getProcessId", p1.getProcessId(), p2.getProcessId());
    a.checkDifferent("02. getProcessId", p1.getProcessId(), p3.getProcessId());
    a.checkDifferent("03. getProcessId", p3.getProcessId(), p2.getProcessId());

    // Testee
    MutexListProxy testee(s.gameSender());
    game::test::WaitIndicator ind;

    // Get list of all mutexes
    {
        MutexListProxy::Infos_t result;
        testee.enumMutexes(ind, result);

        a.checkEqual("11. size", result.size(), 2U);
        bool ok1 = false, ok2 = false;
        for (size_t i = 0; i < result.size(); ++i) {
            if (result[i].processId == p1.getProcessId()) {
                a.checkEqual("12. name 1", result[i].name, "M1");
                ok1 = true;
            } else if (result[i].processId == p2.getProcessId()) {
                a.checkEqual("13. name 2", result[i].name, "M2");
                ok2 = true;
            } else {
                a.check("14. processId", false);
            }
        }
        a.check("15. found 1", ok1);
        a.check("16. found 2", ok2);
    }

    // Get list of mutexes of p1
    {
        MutexListProxy::Infos_t result;
        testee.enumMutexes(ind, result, p1.getProcessId());

        a.checkEqual("21. size", result.size(), 1U);
        a.checkEqual("22. name", result[0].name, "M1");
        a.checkEqual("23. processId", result[0].processId, p1.getProcessId());
    }

    // Get list of mutexes of p3
    {
        MutexListProxy::Infos_t result;
        testee.enumMutexes(ind, result, p3.getProcessId());

        a.checkEqual("31. size", result.size(), 0U);
    }
}
