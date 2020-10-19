/**
  *  \file u/t_game_alliance_hosthandler.cpp
  *  \brief Test for game::alliance::HostHandler
  */

#include "game/alliance/hosthandler.hpp"

#include "t_game_alliance.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/alliance/container.hpp"
#include "game/hostversion.hpp"
#include "game/test/simpleturn.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"

using game::alliance::Container;
using game::alliance::Offer;
using game::v3::Command;

/** Test normal case.
    A: create a Container with a HostHandler. Create a command and parse it; modify alliances.
    E: parsing the command must produce expected alliances; changing alliances must produce correct command */
void
TestGameAllianceHostHandler::testIt()
{
    // Create container with handler
    const int PLAYER = 7;
    game::test::SimpleTurn t;
    afl::string::NullTranslator tx;
    Container c;
    c.addNewHandler(new game::alliance::HostHandler(MKVERSION(3,22,46), t.turn(), PLAYER), tx);

    // Container must contain two levels
    TS_ASSERT(c.getLevels().size() >= 2U);
    Container::Index_t x1 = c.find("thost.ally");
    Container::Index_t x2 = c.find("thost.ff");
    TS_ASSERT_DIFFERS(x1, Container::nil);
    TS_ASSERT_DIFFERS(x2, Container::nil);
    c.getMutableOffer(x1)->oldOffer.set(3, Offer::Yes);

    // Add some offers
    game::v3::CommandContainer& cc = game::v3::CommandExtra::create(t.turn()).create(PLAYER);
    cc.addCommand(Command::TAlliance, 0, "ee3FF2");
    c.postprocess();

    // Verify
    TS_ASSERT_EQUALS(c.getOffer(x1)->newOffer.get(3), Offer::No);
    TS_ASSERT_EQUALS(c.getOffer(x2)->newOffer.get(2), Offer::Yes);

    // Modify
    c.set(x1, 5, Offer::Yes);

    const Command* cmd = cc.getCommand(Command::TAlliance, 0);
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(cmd->getArg(), "FF2ee3ff5");
}

/** Test old host.
    A: create a Container with a HostHandler for Host 3.22.007 (first to have alliances).
    E: must not offer strong alliances */
void
TestGameAllianceHostHandler::testOld()
{
    // Create container with handler
    const int PLAYER = 7;
    game::test::SimpleTurn t;
    afl::string::NullTranslator tx;
    Container c;
    c.addNewHandler(new game::alliance::HostHandler(MKVERSION(3,22,7), t.turn(), PLAYER), tx);

    // Container must contain two levels
    TS_ASSERT(c.getLevels().size() >= 1U);
    Container::Index_t x1 = c.find("thost.ally");
    Container::Index_t x2 = c.find("thost.ff");
    TS_ASSERT_DIFFERS(x1, Container::nil);
    TS_ASSERT_EQUALS(x2, Container::nil);
}

