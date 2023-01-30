/**
  *  \file u/t_game_alliance_phosthandler.cpp
  *  \brief Test for game::alliance::PHostHandler
  */

#include "game/alliance/phosthandler.hpp"

#include "t_game_alliance.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
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
void
TestGameAlliancePHostHandler::testIt()
{
    // Session
    const uint32_t VERSION = MKVERSION(4,0,0);
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(HostVersion(HostVersion::PHost, VERSION)).asPtr());

    // Turn
    game::Turn turn;

    // Create the container
    const int PLAYER = 9;
    Container c;
    c.addNewHandler(new game::alliance::PHostHandler(VERSION, turn, session, PLAYER), tx);

    // Container must contain some levels
    TS_ASSERT(c.getLevels().size() >= 6U);
    Container::Index_t x1 = c.find("phost.ally");
    TS_ASSERT_DIFFERS(x1, Container::nil);
    c.getMutableOffer(x1)->oldOffer.set(3, Offer::Yes);

    // Add some offers
    game::v3::CommandContainer& cc = game::v3::CommandExtra::create(turn).create(PLAYER);
    cc.addCommand(Command::AddDropAlly, 2, "add");
    cc.addCommand(Command::AddDropAlly, 3, "drop");
    cc.addCommand(Command::ConfigAlly, 2, "+c");
    c.postprocess();

    // Verify
    TS_ASSERT_EQUALS(c.getOffer(x1)->newOffer.get(3), Offer::No);
    TS_ASSERT_EQUALS(c.getOffer(x1)->newOffer.get(2), Offer::Yes);

    // Modify
    c.set(x1, 5, Offer::Yes);

    const Command* cmd = cc.getCommand(Command::AddDropAlly, 5);
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(cmd->getArg(), "add");

    // Other commands still there
    TS_ASSERT(cc.getCommand(Command::AddDropAlly, 2) != 0);
    TS_ASSERT(cc.getCommand(Command::AddDropAlly, 3) != 0);
}

/** Test Enemy handling.
    A: create a container with a PHostHandler. Create "enemies" commands and parse them; modify alliances.
    E: parsing the commands must produce expected enemies; changing enemies must produce correct commands */
void
TestGameAlliancePHostHandler::testEnemy()
{
    // Session
    const uint32_t VERSION = MKVERSION(4,1,0);
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(HostVersion(HostVersion::PHost, VERSION)).asPtr());

    // Turn
    game::Turn turn;

    // Create the container
    const int PLAYER = 9;
    Container c;
    c.addNewHandler(new game::alliance::PHostHandler(VERSION, turn, session, PLAYER), tx);

    // Container must contain some levels
    TS_ASSERT(c.getLevels().size() >= 6U);
    Container::Index_t x1 = c.find("phost.enemy");
    TS_ASSERT_DIFFERS(x1, Container::nil);

    // Add some offers
    game::v3::CommandContainer& cc = game::v3::CommandExtra::create(turn).create(PLAYER);
    cc.addCommand(Command::Enemies, 2, "add");
    c.postprocess();

    // Verify
    TS_ASSERT_EQUALS(c.getOffer(x1)->newOffer.get(2), Offer::Yes);
    TS_ASSERT_EQUALS(c.isAny(2, Level::IsEnemy, true), true);

    // Modify
    c.set(x1, 5, Offer::Yes);

    const Command* cmd = cc.getCommand(Command::Enemies, 5);
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(cmd->getArg(), "add");
}
