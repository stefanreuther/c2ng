/**
  *  \file u/t_game_proxy_commandlistproxy.cpp
  *  \brief Test for game::proxy::CommandListProxy
  */

#include "game/proxy/commandlistproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"

using game::v3::Command;
using game::v3::CommandContainer;
using game::Reference;
using game::proxy::CommandListProxy;

/** Test CommandListProxy, success sequence.
    A: create a UI mock, game session, request thread. Apply a sequence of commands to the proxy.
    E: changes applied to CommandContainer as expected */
void
TestGameProxyCommandListProxy::testIt()
{
    // User interface side: Root / Downlink
    game::test::WaitIndicator link;

    // Game side
    const int PLAYER = 8;
    game::test::SessionThread h;
    h.session().setRoot(new game::test::Root(game::HostVersion()));
    h.session().setGame(new game::Game());
    game::Game& g = *h.session().getGame();
    CommandContainer& cc = game::v3::CommandExtra::create(g.currentTurn()).create(PLAYER);
    g.setViewpointPlayer(PLAYER);
    g.currentTurn().universe().ships().create(150);

    cc.addNewCommand(new Command(Command::GiveShip, 150, "3"));
    cc.addNewCommand(new Command(Command::GiveShip, 250, "4"));
    cc.addNewCommand(new Command(Command::Filter, 0, "no"));

    // Test
    CommandListProxy testee(h.gameSender());

    // Initialize
    {
        CommandListProxy::Infos_t out;
        bool ok = testee.init(link, out);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(out.size(), 3U);
        TS_ASSERT_EQUALS(out[0].text, "give ship 150 to 3");
        TS_ASSERT_EQUALS(out[1].text, "give ship 250 to 4");
        TS_ASSERT_EQUALS(out[2].text, "filter no");
        TS_ASSERT_EQUALS(out[0].ref, Reference(Reference::Ship, 150));  // target ship exists
        TS_ASSERT_EQUALS(out[1].ref, Reference());                      // target ship does not exist
        TS_ASSERT_EQUALS(out[2].ref, Reference());                      // no target
    }

    // Modify
    {
        CommandListProxy::Infos_t out;
        size_t pos = 4444;
        bool ok = testee.addCommand(link, "g s 250 5", out, pos);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(out.size(), 3U);
        TS_ASSERT_EQUALS(out[0].text, "give ship 150 to 3");
        TS_ASSERT_EQUALS(out[1].text, "give ship 250 to 5");
        TS_ASSERT_EQUALS(out[2].text, "filter no");
        TS_ASSERT_EQUALS(pos, 1U);
    }

    // Add
    {
        CommandListProxy::Infos_t out;
        size_t pos = 4444;
        bool ok = testee.addCommand(link, "re c 444", out, pos);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(out.size(), 4U);
        TS_ASSERT_EQUALS(out[0].text, "give ship 150 to 3");
        TS_ASSERT_EQUALS(out[1].text, "give ship 250 to 5");
        TS_ASSERT_EQUALS(out[2].text, "filter no");
        TS_ASSERT_EQUALS(out[3].text, "remote c 444");
        TS_ASSERT_EQUALS(pos, 3U);
    }

    // Add failure
    {
        CommandListProxy::Infos_t out;
        size_t pos = 4444;
        bool ok = testee.addCommand(link, "buy a vowel", out, pos);
        TS_ASSERT(!ok);
    }

    // Delete
    {
        CommandListProxy::Infos_t out;
        testee.removeCommand(link, "give ship 250 to 5", out);
        TS_ASSERT_EQUALS(out.size(), 3U);
        TS_ASSERT_EQUALS(out[0].text, "give ship 150 to 3");
        TS_ASSERT_EQUALS(out[1].text, "filter no");
        TS_ASSERT_EQUALS(out[2].text, "remote c 444");
    }

    // Verify game side
    size_t n = 0;
    for (CommandContainer::Iterator_t it = cc.begin(), end = cc.end(); it != end; ++it) {
        ++n;
    }
    TS_ASSERT_EQUALS(n, 3U);    
    TS_ASSERT_EQUALS((*cc.begin())->getCommand(), Command::GiveShip);
}

/** Test CommandListProxy, creation of CommandContainer.
    A: create a UI mock, game session with a CommandExtra but no CommandContainer, request thread. Initialize and add a command.
    E: Initialisation/command must succeed. */
void
TestGameProxyCommandListProxy::testCreate()
{
    // User interface side: Root / Downlink
    game::test::WaitIndicator link;

    // Game side
    const int PLAYER = 8;
    game::test::SessionThread h;
    h.session().setRoot(new game::test::Root(game::HostVersion()));
    h.session().setGame(new game::Game());
    game::Game& g = *h.session().getGame();
    game::v3::CommandExtra::create(g.currentTurn());
    g.setViewpointPlayer(PLAYER);

    // Test
    CommandListProxy testee(h.gameSender());

    // Initialize
    {
        CommandListProxy::Infos_t out;
        bool ok = testee.init(link, out);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(out.size(), 0U);
    }

    // Modify
    {
        CommandListProxy::Infos_t out;
        size_t pos = 4444;
        bool ok = testee.addCommand(link, "language english", out, pos);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(out.size(), 1U);
        TS_ASSERT_EQUALS(out[0].text, "language english");
        TS_ASSERT_EQUALS(pos, 0U);
    }
}

/** Test CommandListProxy, notification of changes.
    A: create a UI mock, game session, request thread. Add/remove commands referring to an object.
    E: check that object receives callbacks. */
void
TestGameProxyCommandListProxy::testNotify()
{
    // User interface side: Root / Downlink
    game::test::WaitIndicator link;

    // Game side
    const int PLAYER = 8;
    game::test::SessionThread h;
    h.session().setRoot(new game::test::Root(game::HostVersion()));
    h.session().setGame(new game::Game());
    game::Game& g = *h.session().getGame();
    game::v3::CommandExtra::create(g.currentTurn()).create(PLAYER);
    g.setViewpointPlayer(PLAYER);
    game::map::Ship* sh = g.currentTurn().universe().ships().create(150);
    TS_ASSERT(sh);

    // Ship must be visible
    sh->addShipXYData(game::map::Point(1,2), 3, 4, game::PlayerSet_t(PLAYER));
    sh->internalCheck();

    // Change listener
    int count = 0;
    class Counter : public afl::base::Closure<void(int)> {
     public:
        Counter(int& count)
            : m_count(count)
            { }
        virtual void call(int)
            { ++m_count; }
        virtual Counter* clone() const
            { return new Counter(*this); }
     private:
        int& m_count;
    };
    sh->sig_change.addNewClosure(new Counter(count));

    // Test
    CommandListProxy testee(h.gameSender());

    // Add
    {
        CommandListProxy::Infos_t out;
        size_t pos = 4444;
        bool ok = testee.addCommand(link, "re c 150", out, pos);
        TS_ASSERT(ok);
        TS_ASSERT_EQUALS(count, 1);
        TS_ASSERT_EQUALS(out.size(), 1U);
        TS_ASSERT_EQUALS(out[0].text, "remote c 150");
    }

    // Remove
    {
        CommandListProxy::Infos_t out;
        testee.removeCommand(link, "remote c 150", out);
        TS_ASSERT_EQUALS(count, 2);
        TS_ASSERT_EQUALS(out.size(), 0U);
    }
}

/** Test CommandListProxy, failure case: empty session.
    A: create a UI mock, empty game session, request thread. Initialize CommandListProxy.
    E: must report failure (no session present) */
void
TestGameProxyCommandListProxy::testFailureEmptySession()
{
    // User interface side: Root / Downlink
    game::test::WaitIndicator link;

    // Game side
    game::test::SessionThread h;

    // Test
    CommandListProxy testee(h.gameSender());

    // Initialize
    {
        CommandListProxy::Infos_t out;
        bool ok = testee.init(link, out);
        TS_ASSERT(!ok);
    }

    // Add
    {
        CommandListProxy::Infos_t out;
        size_t pos = 4444;
        bool ok = testee.addCommand(link, "allies add 3", out, pos);
        TS_ASSERT(!ok);
    }
}

/** Test CommandListProxy, failure case: no CommandContainer (not supported by game).
    A: create a UI mock, game session without CommandContainer, request thread. Initialize CommandListProxy.
    E: must report failure (no CommandContainer present) */
void
TestGameProxyCommandListProxy::testFailureUnsupported()
{
    // User interface side: Root / Downlink
    game::test::WaitIndicator link;

    // Game side
    const int PLAYER = 8;
    game::test::SessionThread h;
    h.session().setRoot(new game::test::Root(game::HostVersion()));
    h.session().setGame(new game::Game());
    game::Game& g = *h.session().getGame();
    g.setViewpointPlayer(PLAYER);

    // Test
    CommandListProxy testee(h.gameSender());

    // Initialize
    {
        CommandListProxy::Infos_t out;
        bool ok = testee.init(link, out);
        TS_ASSERT(!ok);
    }

    // Add
    {
        CommandListProxy::Infos_t out;
        size_t pos = 4444;
        bool ok = testee.addCommand(link, "allies add 3", out, pos);
        TS_ASSERT(!ok);
    }
}

