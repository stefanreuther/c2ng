/**
  *  \file test/game/proxy/keymapproxytest.cpp
  *  \brief Test for game::proxy::KeymapProxy
  */

#include "game/proxy/keymapproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "util/keymapinformation.hpp"
#include "util/simplerequestdispatcher.hpp"

/** Test synchronous operations: getDescription(), getKey(). */
AFL_TEST("game.proxy.KeymapProxy:synchronous", a)
{
    // Setup
    game::test::SessionThread h;

    // - Add some keymaps
    util::KeymapTable& keymaps = h.session().world().keymaps();
    util::KeymapRef_t ka = keymaps.createKeymap("A");
    util::KeymapRef_t kb = keymaps.createKeymap("B");
    ka->addParent(*kb);

    // - Add a command
    util::AtomTable& atomTable = h.session().world().atomTable();
    kb->addKey('x', atomTable.getAtomFromString("usekeymap c"), 0);

    // - WaitIndicator
    game::test::WaitIndicator ind;

    // Object under test
    game::proxy::KeymapProxy proxy(h.gameSender(), ind);
    proxy.setKeymapName("A");

    // Test getDescription
    util::KeymapInformation mapInfo;
    proxy.getDescription(ind, mapInfo);
    a.checkEqual("01. size", mapInfo.size(), 2U);

    // Test getKey
    game::proxy::KeymapProxy::Info keyInfo;
    proxy.getKey(ind, 'x', keyInfo);
    a.checkEqual("11. result", keyInfo.result, game::proxy::KeymapProxy::Normal);
    a.checkEqual("12. keymapName", keyInfo.keymapName, "B");
    a.checkEqual("13. command", keyInfo.command, "usekeymap c");
    a.checkEqual("14. alternateKeymapName", keyInfo.alternateKeymapName, "C");
    a.checkEqual("15. origin", keyInfo.origin, "");
}

/** Test asynchronous operations: listener. */
AFL_TEST("game.proxy.KeymapProxy:listener", a)
{
    // Setup
    game::test::SessionThread h;

    // - Add a keymap and some keys
    util::KeymapTable& keymaps = h.session().world().keymaps();
    util::AtomTable& atomTable = h.session().world().atomTable();
    util::KeymapRef_t ka = keymaps.createKeymap("A");
    ka->addKey('x', atomTable.getAtomFromString("a"), 0);
    ka->addKey('y', atomTable.getAtomFromString("b"), 0);

    // - Listener
    class TestListener : public game::proxy::KeymapProxy::Listener {
     public:
        TestListener()
            : m_keys(), m_ok(false)
            { }

        virtual void updateKeyList(util::KeySet_t& keys)
            {
                m_keys = keys;
                m_ok = true;
            }

        util::KeySet_t m_keys;
        bool m_ok;
    };
    TestListener t;

    // Object under test
    util::SimpleRequestDispatcher disp;
    game::proxy::KeymapProxy testee(h.gameSender(), disp);
    testee.setListener(t);
    testee.setKeymapName("A");

    // Wait for completion
    while (!t.m_ok) {
        a.checkEqual("01. wait", disp.wait(1000), true);
    }

    // Verify
    a.check("11. key x", t.m_keys.find('x') != t.m_keys.end());
    a.check("12. key y", t.m_keys.find('y') != t.m_keys.end());
    a.check("13. key a", t.m_keys.find('a') == t.m_keys.end());
}
