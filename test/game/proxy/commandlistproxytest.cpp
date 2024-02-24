/**
  *  \file test/game/proxy/commandlistproxytest.cpp
  *  \brief Test for game::proxy::CommandListProxy
  */

#include "game/proxy/commandlistproxy.hpp"

#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.proxy.CommandListProxy:sequence", a)
{
    // User interface side: Root / Downlink
    game::test::WaitIndicator link;

    // Game side
    const int PLAYER = 8;
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    h.session().setGame(new game::Game());
    game::Game& g = *h.session().getGame();
    CommandContainer& cc = game::v3::CommandExtra::create(g.currentTurn()).create(PLAYER);
    g.setViewpointPlayer(PLAYER);
    g.currentTurn().universe().ships().create(150);
    g.currentTurn().setCommandPlayers(game::PlayerSet_t(PLAYER));

    cc.addNewCommand(new Command(Command::GiveShip, 150, "3"));
    cc.addNewCommand(new Command(Command::GiveShip, 250, "4"));
    cc.addNewCommand(new Command(Command::Filter, 0, "no"));

    // Test
    CommandListProxy testee(h.gameSender());

    // Initialize
    {
        CommandListProxy::Infos_t out;
        CommandListProxy::MetaInfo metaOut;
        bool ok = testee.init(link, out, metaOut);
        a.check("01. ok", ok);
        a.checkEqual("02. size", out.size(), 3U);
        a.checkEqual("03. text", out[0].text, "give ship 150 to 3");
        a.checkEqual("04. text", out[1].text, "give ship 250 to 4");
        a.checkEqual("05. text", out[2].text, "filter no");
        a.checkEqual("06. ref",  out[0].ref, Reference(Reference::Ship, 150));  // target ship exists
        a.checkEqual("07. ref",  out[1].ref, Reference());                      // target ship does not exist
        a.checkEqual("08. ref",  out[2].ref, Reference());                      // no target
        a.checkEqual("09. editable", metaOut.editable, true);
        a.checkEqual("10. playerNr", metaOut.playerNr, PLAYER);
    }

    // Modify
    {
        CommandListProxy::Infos_t out;
        size_t pos = 4444;
        bool ok = testee.addCommand(link, "g s 250 5", out, pos);
        a.check("11. ok", ok);
        a.checkEqual("12. size", out.size(), 3U);
        a.checkEqual("13. text", out[0].text, "give ship 150 to 3");
        a.checkEqual("14. text", out[1].text, "give ship 250 to 5");
        a.checkEqual("15. text", out[2].text, "filter no");
        a.checkEqual("16. pos",  pos, 1U);
    }

    // Add
    {
        CommandListProxy::Infos_t out;
        size_t pos = 4444;
        bool ok = testee.addCommand(link, "re c 444", out, pos);
        a.check("21. ok", ok);
        a.checkEqual("22. size", out.size(), 4U);
        a.checkEqual("23. text", out[0].text, "give ship 150 to 3");
        a.checkEqual("24. text", out[1].text, "give ship 250 to 5");
        a.checkEqual("25. text", out[2].text, "filter no");
        a.checkEqual("26. text", out[3].text, "remote c 444");
        a.checkEqual("27. pos",  pos, 3U);
    }

    // Add failure
    {
        CommandListProxy::Infos_t out;
        size_t pos = 4444;
        bool ok = testee.addCommand(link, "buy a vowel", out, pos);
        a.check("31. ok", !ok);
    }

    // Delete
    {
        CommandListProxy::Infos_t out;
        testee.removeCommand(link, "give ship 250 to 5", out);
        a.checkEqual("41. size", out.size(), 3U);
        a.checkEqual("42. text", out[0].text, "give ship 150 to 3");
        a.checkEqual("43. text", out[1].text, "filter no");
        a.checkEqual("44. text", out[2].text, "remote c 444");
    }

    // Verify game side
    size_t n = 0;
    for (CommandContainer::Iterator_t it = cc.begin(), end = cc.end(); it != end; ++it) {
        ++n;
    }
    a.checkEqual("51. count", n, 3U);
    a.checkEqual("52. getCommand", (*cc.begin())->getCommand(), Command::GiveShip);
}

/** Test CommandListProxy, creation of CommandContainer.
    A: create a UI mock, game session with a CommandExtra but no CommandContainer, request thread. Initialize and add a command.
    E: Initialisation/command must succeed. */
AFL_TEST("game.proxy.CommandListProxy:create", a)
{
    // User interface side: Root / Downlink
    game::test::WaitIndicator link;

    // Game side
    const int PLAYER = 8;
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    h.session().setGame(new game::Game());
    game::Game& g = *h.session().getGame();
    game::v3::CommandExtra::create(g.currentTurn());
    g.setViewpointPlayer(PLAYER);
    g.currentTurn().setCommandPlayers(game::PlayerSet_t(PLAYER));

    // Test
    CommandListProxy testee(h.gameSender());

    // Initialize
    {
        CommandListProxy::Infos_t out;
        CommandListProxy::MetaInfo metaOut;
        bool ok = testee.init(link, out, metaOut);
        a.check("01. ok", ok);
        a.checkEqual("02. size", out.size(), 0U);
        a.checkEqual("03. editable", metaOut.editable, true);
    }

    // Modify
    {
        CommandListProxy::Infos_t out;
        size_t pos = 4444;
        bool ok = testee.addCommand(link, "language english", out, pos);
        a.check("11. ok", ok);
        a.checkEqual("12. size", out.size(), 1U);
        a.checkEqual("13. text", out[0].text, "language english");
        a.checkEqual("14. pos",  pos, 0U);
    }
}

/** Test CommandListProxy, notification of changes.
    A: create a UI mock, game session, request thread. Add/remove commands referring to an object.
    E: check that object receives callbacks. */
AFL_TEST("game.proxy.CommandListProxy:notify", a)
{
    // User interface side: Root / Downlink
    game::test::WaitIndicator link;

    // Game side
    const int PLAYER = 8;
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    h.session().setGame(new game::Game());
    game::Game& g = *h.session().getGame();
    game::v3::CommandExtra::create(g.currentTurn()).create(PLAYER);
    g.setViewpointPlayer(PLAYER);
    g.currentTurn().setCommandPlayers(game::PlayerSet_t(PLAYER));
    game::map::Ship* sh = g.currentTurn().universe().ships().create(150);
    a.check("01. ship created", sh);

    // Ship must be visible
    sh->addShipXYData(game::map::Point(1,2), 3, 4, game::PlayerSet_t(PLAYER));
    sh->internalCheck(game::PlayerSet_t(PLAYER), 15);

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
        a.check("11. ok", ok);
        a.checkEqual("12. count", count, 1);
        a.checkEqual("13. size", out.size(), 1U);
        a.checkEqual("14. text", out[0].text, "remote c 150");
    }

    // Remove
    {
        CommandListProxy::Infos_t out;
        testee.removeCommand(link, "remote c 150", out);
        a.checkEqual("21. count", count, 2);
        a.checkEqual("22. size", out.size(), 0U);
    }
}

/** Test CommandListProxy, failure case: empty session.
    A: create a UI mock, empty game session, request thread. Initialize CommandListProxy.
    E: must report failure (no session present) */
AFL_TEST("game.proxy.CommandListProxy:error:empty-session", a)
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
        CommandListProxy::MetaInfo metaOut;
        bool ok = testee.init(link, out, metaOut);
        a.check("01. ok", !ok);
        a.check("02. editable", !metaOut.editable);
    }

    // Add
    {
        CommandListProxy::Infos_t out;
        size_t pos = 4444;
        bool ok = testee.addCommand(link, "allies add 3", out, pos);
        a.check("11. ok", !ok);
    }
}

/** Test CommandListProxy, failure case: no CommandContainer (not supported by game).
    A: create a UI mock, game session without CommandContainer, request thread. Initialize CommandListProxy.
    E: must report failure (no CommandContainer present) */
AFL_TEST("game.proxy.CommandListProxy:error:unuspported", a)
{
    // User interface side: Root / Downlink
    game::test::WaitIndicator link;

    // Game side
    const int PLAYER = 8;
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    h.session().setGame(new game::Game());
    game::Game& g = *h.session().getGame();
    g.setViewpointPlayer(PLAYER);
    g.currentTurn().setCommandPlayers(game::PlayerSet_t(PLAYER));

    // Test
    CommandListProxy testee(h.gameSender());

    // Initialize
    {
        CommandListProxy::Infos_t out;
        CommandListProxy::MetaInfo metaOut;
        bool ok = testee.init(link, out, metaOut);
        a.check("01. ok", !ok);
        a.check("02. editable", !metaOut.editable);
    }

    // Add
    {
        CommandListProxy::Infos_t out;
        size_t pos = 4444;
        bool ok = testee.addCommand(link, "allies add 3", out, pos);
        a.check("11. ok", !ok);
    }
}

/** Test CommandListProxy, read-only.
    A: set up a game with no setCommandPlayers().
    E: MetaInfo reportes not editable */
AFL_TEST("game.proxy.CommandListProxy:read-only", a)
{
    // User interface side: Root / Downlink
    game::test::WaitIndicator link;

    // Game side
    const int PLAYER = 8;
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
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
    CommandListProxy::Infos_t out;
    CommandListProxy::MetaInfo metaOut;
    bool ok = testee.init(link, out, metaOut);
    a.check("01. ok", ok);
    a.checkEqual("02. size", out.size(), 3U);
    a.checkEqual("03. editable", metaOut.editable, false);
    a.checkEqual("04. playerNr", metaOut.playerNr, PLAYER);
}
