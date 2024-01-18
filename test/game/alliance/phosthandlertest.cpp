/**
  *  \file test/game/alliance/phosthandlertest.cpp
  *  \brief Test for game::alliance::PHostHandler
  */

#include "game/alliance/phosthandler.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/alliance/container.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"

using game::alliance::Container;
using game::alliance::Level;
using game::alliance::Offer;
using game::v3::Command;
using game::HostVersion;

/** Test normal operation.
    A: create a container with a PHostHandler. Create commands and parse them; modify alliances.
    E: parsing the commands must produce expected alliances; changing alliances must produce correct commands */
AFL_TEST("game.alliance.PHostHandler:allies", a)
{
    // Session
    const uint32_t VERSION = MKVERSION(4,0,0);
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Root> root(game::test::makeRoot(HostVersion(HostVersion::PHost, VERSION)));

    // Turn
    game::Turn turn;

    // Create the container
    const int PLAYER = 9;
    Container c;
    c.addNewHandler(new game::alliance::PHostHandler(turn, root, PLAYER), tx);

    // Container must contain some levels
    a.check("01. getLevels", c.getLevels().size() >= 6U);
    Container::Index_t x1 = c.find("phost.ally");
    a.checkDifferent("02. find", x1, Container::nil);
    c.getMutableOffer(x1)->oldOffer.set(3, Offer::Yes);

    // Add some offers
    game::v3::CommandContainer& cc = game::v3::CommandExtra::create(turn).create(PLAYER);
    cc.addCommand(Command::AddDropAlly, 2, "add");
    cc.addCommand(Command::AddDropAlly, 3, "drop");
    cc.addCommand(Command::ConfigAlly, 2, "+c");
    c.postprocess();

    // Verify
    a.checkEqual("11. getOffer", c.getOffer(x1)->newOffer.get(3), Offer::No);
    a.checkEqual("12. getOffer", c.getOffer(x1)->newOffer.get(2), Offer::Yes);

    // Modify
    c.set(x1, 5, Offer::Yes);

    const Command* cmd = cc.getCommand(Command::AddDropAlly, 5);
    a.checkNonNull("21. cmd", cmd);
    a.checkEqual("22. getArg", cmd->getArg(), "add");

    // Other commands still there
    a.checkNonNull("31. getCommand", cc.getCommand(Command::AddDropAlly, 2));
    a.checkNonNull("32. getCommand", cc.getCommand(Command::AddDropAlly, 3));
}

/** Test Enemy handling.
    A: create a container with a PHostHandler. Create "enemies" commands and parse them; modify alliances.
    E: parsing the commands must produce expected enemies; changing enemies must produce correct commands */
AFL_TEST("game.alliance.PHostHandler:enemy", a)
{
    // Session
    const uint32_t VERSION = MKVERSION(4,1,0);
    afl::string::NullTranslator tx;
    afl::base::Ref<game::Root> root(game::test::makeRoot(HostVersion(HostVersion::PHost, VERSION)));

    // Turn
    game::Turn turn;

    // Create the container
    const int PLAYER = 9;
    Container c;
    c.addNewHandler(new game::alliance::PHostHandler(turn, root, PLAYER), tx);

    // Container must contain some levels
    a.check("01. getLevels", c.getLevels().size() >= 6U);
    Container::Index_t x1 = c.find("phost.enemy");
    a.checkDifferent("02. find", x1, Container::nil);

    // Add some offers
    game::v3::CommandContainer& cc = game::v3::CommandExtra::create(turn).create(PLAYER);
    cc.addCommand(Command::Enemies, 2, "add");
    c.postprocess();

    // Verify
    a.checkEqual("11. getOffer", c.getOffer(x1)->newOffer.get(2), Offer::Yes);
    a.checkEqual("12. isAny", c.isAny(2, Level::IsEnemy, true), true);

    // Modify
    c.set(x1, 5, Offer::Yes);

    const Command* cmd = cc.getCommand(Command::Enemies, 5);
    a.checkNonNull("21. cmd", cmd);
    a.checkEqual("22. getArg", cmd->getArg(), "add");
}
