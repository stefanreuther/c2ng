/**
  *  \file test/game/interface/friendlycodepropertytest.cpp
  *  \brief Test for game::interface::FriendlyCodeProperty
  */

#include "game/interface/friendlycodeproperty.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/playerlist.hpp"
#include "game/spec/friendlycode.hpp"
#include "interpreter/test/valueverifier.hpp"

using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewString;

AFL_TEST("game.interface.FriendlyCodeProperty", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList players;
    players.create(5)->setName(game::Player::ShortName, "Pirate");

    // Some friendly codes
    game::spec::FriendlyCode fc1("fc1", "sb,First", tx);
    game::spec::FriendlyCode fc2("fc2", "sca,Second", tx);
    game::spec::FriendlyCode fc3("fc3", "u,Third", tx);
    game::spec::FriendlyCode fc4("fc4", "x,Fourth", tx);
    game::spec::FriendlyCode fc5("fc5", "sr+5,For %5 only", tx);

    // Verify
    verifyNewString(a("fc1 name"), getFriendlyCodeProperty(fc1, game::interface::ifpName, players, tx), "fc1");
    verifyNewString(a("fc2 name"), getFriendlyCodeProperty(fc2, game::interface::ifpName, players, tx), "fc2");
    verifyNewString(a("fc3 name"), getFriendlyCodeProperty(fc3, game::interface::ifpName, players, tx), "fc3");
    verifyNewString(a("fc4 name"), getFriendlyCodeProperty(fc4, game::interface::ifpName, players, tx), "fc4");
    verifyNewString(a("fc5 name"), getFriendlyCodeProperty(fc5, game::interface::ifpName, players, tx), "fc5");

    verifyNewString(a("fc1 description"), getFriendlyCodeProperty(fc1, game::interface::ifpDescription, players, tx), "First");
    verifyNewString(a("fc2 description"), getFriendlyCodeProperty(fc2, game::interface::ifpDescription, players, tx), "Second");
    verifyNewString(a("fc3 description"), getFriendlyCodeProperty(fc3, game::interface::ifpDescription, players, tx), "Third");
    verifyNewString(a("fc4 description"), getFriendlyCodeProperty(fc4, game::interface::ifpDescription, players, tx), "Fourth");
    verifyNewString(a("fc5 description"), getFriendlyCodeProperty(fc5, game::interface::ifpDescription, players, tx), "For Pirate only");

    verifyNewString(a("fc1 flags"), getFriendlyCodeProperty(fc1, game::interface::ifpFlags, players, tx), "sb");
    verifyNewString(a("fc2 flags"), getFriendlyCodeProperty(fc2, game::interface::ifpFlags, players, tx), "sca");
    verifyNewString(a("fc3 flags"), getFriendlyCodeProperty(fc3, game::interface::ifpFlags, players, tx), "u");
    verifyNewString(a("fc4 flags"), getFriendlyCodeProperty(fc4, game::interface::ifpFlags, players, tx), "x");
    verifyNewString(a("fc5 flags"), getFriendlyCodeProperty(fc5, game::interface::ifpFlags, players, tx), "sr");

    verifyNewInteger(a("fc1 races"), getFriendlyCodeProperty(fc1, game::interface::ifpRaces, players, tx), -1);
    verifyNewInteger(a("fc2 races"), getFriendlyCodeProperty(fc2, game::interface::ifpRaces, players, tx), -1);
    verifyNewInteger(a("fc3 races"), getFriendlyCodeProperty(fc3, game::interface::ifpRaces, players, tx), -1);
    verifyNewInteger(a("fc4 races"), getFriendlyCodeProperty(fc4, game::interface::ifpRaces, players, tx), -1);
    verifyNewInteger(a("fc5 races"), getFriendlyCodeProperty(fc5, game::interface::ifpRaces, players, tx), 1 << 5);
}
