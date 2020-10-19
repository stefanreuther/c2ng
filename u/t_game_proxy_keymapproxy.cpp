/**
  *  \file u/t_game_proxy_keymapproxy.cpp
  *  \brief Test for game::proxy::KeymapProxy
  */

#include "game/proxy/keymapproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/session.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "util/keymapinformation.hpp"
#include "util/simplerequestdispatcher.hpp"

/** Test synchronous operations: getDescription(), getKey(). */
void
TestGameProxyKeymapProxy::testGetInfo()
{
    // Setup
    game::test::SessionThread h;

    // - Add some keymaps
    util::KeymapTable& keymaps = h.session().world().keymaps();
    util::KeymapRef_t a = keymaps.createKeymap("A");
    util::KeymapRef_t b = keymaps.createKeymap("B");
    a->addParent(*b);

    // - Add a command
    util::AtomTable& atomTable = h.session().world().atomTable();
    b->addKey('x', atomTable.getAtomFromString("usekeymap c"), 0);

    // - WaitIndicator
    game::test::WaitIndicator ind;

    // Object under test
    game::proxy::KeymapProxy proxy(h.gameSender(), ind);
    proxy.setKeymapName("A");

    // Test getDescription
    util::KeymapInformation mapInfo;
    proxy.getDescription(ind, mapInfo);
    TS_ASSERT_EQUALS(mapInfo.size(), 2U);

    // Test getKey
    game::proxy::KeymapProxy::Info keyInfo;
    proxy.getKey(ind, 'x', keyInfo);
    TS_ASSERT_EQUALS(keyInfo.result, game::proxy::KeymapProxy::Normal);
    TS_ASSERT_EQUALS(keyInfo.keymapName, "B");
    TS_ASSERT_EQUALS(keyInfo.command, "usekeymap c");
    TS_ASSERT_EQUALS(keyInfo.alternateKeymapName, "C");
    TS_ASSERT_EQUALS(keyInfo.origin, "");
}

/** Test asynchronous operations: listener. */
void
TestGameProxyKeymapProxy::testListener()
{
    // Setup
    CxxTest::setAbortTestOnFail(true);
    game::test::SessionThread h;

    // - Add a keymap and some keys
    util::KeymapTable& keymaps = h.session().world().keymaps();
    util::AtomTable& atomTable = h.session().world().atomTable();
    util::KeymapRef_t a = keymaps.createKeymap("A");
    a->addKey('x', atomTable.getAtomFromString("a"), 0);
    a->addKey('y', atomTable.getAtomFromString("b"), 0);

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
        TS_ASSERT_EQUALS(disp.wait(1000), true);
    }

    // Verify
    TS_ASSERT(t.m_keys.find('x') != t.m_keys.end());
    TS_ASSERT(t.m_keys.find('y') != t.m_keys.end());
    TS_ASSERT(t.m_keys.find('a') == t.m_keys.end());
}

