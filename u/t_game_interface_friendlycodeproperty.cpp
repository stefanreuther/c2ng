/**
  *  \file u/t_game_interface_friendlycodeproperty.cpp
  *  \brief Test for game::interface::FriendlyCodeProperty
  */

#include "game/interface/friendlycodeproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/playerlist.hpp"
#include "game/spec/friendlycode.hpp"
#include "interpreter/test/valueverifier.hpp"

using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewString;

void
TestGameInterfaceFriendlyCodeProperty::testIt()
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
    verifyNewString("fc1 name", getFriendlyCodeProperty(fc1, game::interface::ifpName, players, tx), "fc1");
    verifyNewString("fc2 name", getFriendlyCodeProperty(fc2, game::interface::ifpName, players, tx), "fc2");
    verifyNewString("fc3 name", getFriendlyCodeProperty(fc3, game::interface::ifpName, players, tx), "fc3");
    verifyNewString("fc4 name", getFriendlyCodeProperty(fc4, game::interface::ifpName, players, tx), "fc4");
    verifyNewString("fc5 name", getFriendlyCodeProperty(fc5, game::interface::ifpName, players, tx), "fc5");

    verifyNewString("fc1 description", getFriendlyCodeProperty(fc1, game::interface::ifpDescription, players, tx), "First");
    verifyNewString("fc2 description", getFriendlyCodeProperty(fc2, game::interface::ifpDescription, players, tx), "Second");
    verifyNewString("fc3 description", getFriendlyCodeProperty(fc3, game::interface::ifpDescription, players, tx), "Third");
    verifyNewString("fc4 description", getFriendlyCodeProperty(fc4, game::interface::ifpDescription, players, tx), "Fourth");
    verifyNewString("fc5 description", getFriendlyCodeProperty(fc5, game::interface::ifpDescription, players, tx), "For Pirate only");

    verifyNewString("fc1 flags", getFriendlyCodeProperty(fc1, game::interface::ifpFlags, players, tx), "sb");
    verifyNewString("fc2 flags", getFriendlyCodeProperty(fc2, game::interface::ifpFlags, players, tx), "sca");
    verifyNewString("fc3 flags", getFriendlyCodeProperty(fc3, game::interface::ifpFlags, players, tx), "u");
    verifyNewString("fc4 flags", getFriendlyCodeProperty(fc4, game::interface::ifpFlags, players, tx), "x");
    verifyNewString("fc5 flags", getFriendlyCodeProperty(fc5, game::interface::ifpFlags, players, tx), "sr");

    verifyNewInteger("fc1 races", getFriendlyCodeProperty(fc1, game::interface::ifpRaces, players, tx), -1);
    verifyNewInteger("fc2 races", getFriendlyCodeProperty(fc2, game::interface::ifpRaces, players, tx), -1);
    verifyNewInteger("fc3 races", getFriendlyCodeProperty(fc3, game::interface::ifpRaces, players, tx), -1);
    verifyNewInteger("fc4 races", getFriendlyCodeProperty(fc4, game::interface::ifpRaces, players, tx), -1);
    verifyNewInteger("fc5 races", getFriendlyCodeProperty(fc5, game::interface::ifpRaces, players, tx), 1 << 5);
}

