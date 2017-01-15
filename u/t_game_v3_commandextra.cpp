/**
  *  \file u/t_game_v3_commandextra.cpp
  *  \brief Test for game::v3::CommandExtra
  */

#include "game/v3/commandextra.hpp"

#include "t_game_v3.hpp"
#include "game/turn.hpp"
#include "game/v3/commandcontainer.hpp"

/** Test event propagation. */
void
TestGameV3CommandExtra::testEvents()
{
    using game::v3::Command;

    // Create
    game::Turn turn;
    game::v3::CommandExtra testee(turn);

    // Add ship, planet, minefield
    game::map::Ship* sh = turn.universe().ships().create(42);
    game::map::Planet* pl = turn.universe().planets().create(23);
    game::map::Minefield* mf = turn.universe().minefields().create(15);
    TS_ASSERT(sh != 0);
    TS_ASSERT(pl != 0);
    TS_ASSERT(mf != 0);

    sh->markClean();
    pl->markClean();
    mf->markClean();

    // Create CommandContainer for one player
    game::v3::CommandContainer& cc = testee.create(3);
    TS_ASSERT(!sh->isDirty());
    TS_ASSERT(!pl->isDirty());
    TS_ASSERT(!mf->isDirty());

    // Create unrelated commands
    cc.addCommand(Command::phc_RemoteControl, 23, "control");
    cc.addCommand(Command::phc_GivePlanet,    42, "9");
    cc.addCommand(Command::phc_AddDropAlly,   15, "add");
    cc.addCommand(Command::phc_ShowPlanet,    15, "7");
    TS_ASSERT(!sh->isDirty());
    TS_ASSERT(!pl->isDirty());
    TS_ASSERT(!mf->isDirty());

    // Ship command
    cc.addCommand(Command::phc_RemoteControl, 42, "allow");
    TS_ASSERT(sh->isDirty());
    TS_ASSERT(!pl->isDirty());
    TS_ASSERT(!mf->isDirty());
    sh->markClean();

    // Planet command
    cc.addCommand(Command::phc_GivePlanet, 23, "11");
    TS_ASSERT(!sh->isDirty());
    TS_ASSERT(pl->isDirty());
    TS_ASSERT(!mf->isDirty());
    pl->markClean();

    // Minefield command
    cc.addCommand(Command::phc_ShowMinefield, 15, "1");
    TS_ASSERT(!sh->isDirty());
    TS_ASSERT(!pl->isDirty());
    TS_ASSERT(mf->isDirty());
    mf->markClean();

    // Clear
    cc.clear();
    TS_ASSERT(sh->isDirty());
    TS_ASSERT(pl->isDirty());
    TS_ASSERT(mf->isDirty());
}

