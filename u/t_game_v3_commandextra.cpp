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
    cc.addCommand(Command::RemoteControl, 23, "control");
    cc.addCommand(Command::GivePlanet,    42, "9");
    cc.addCommand(Command::AddDropAlly,   15, "add");
    cc.addCommand(Command::ShowPlanet,    15, "7");
    TS_ASSERT(!sh->isDirty());
    TS_ASSERT(!pl->isDirty());
    TS_ASSERT(!mf->isDirty());

    // Ship command
    cc.addCommand(Command::RemoteControl, 42, "allow");
    TS_ASSERT(sh->isDirty());
    TS_ASSERT(!pl->isDirty());
    TS_ASSERT(!mf->isDirty());
    sh->markClean();

    // Planet command
    cc.addCommand(Command::GivePlanet, 23, "11");
    TS_ASSERT(!sh->isDirty());
    TS_ASSERT(pl->isDirty());
    TS_ASSERT(!mf->isDirty());
    pl->markClean();

    // Minefield command
    cc.addCommand(Command::ShowMinefield, 15, "1");
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

/** Test access to CommandExtra. */
void
TestGameV3CommandExtra::testGet()
{
    using game::v3::CommandExtra;
    using game::v3::CommandContainer;

    game::Turn t;
    const game::Turn& ct = t;

    // Initially, no CommandExtra present
    TS_ASSERT(CommandExtra::get(t) == 0);
    TS_ASSERT(CommandExtra::get(ct) == 0);

    // Create one
    CommandExtra::create(t);

    // Now it's there
    CommandExtra* p = CommandExtra::get(t);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(CommandExtra::get(ct), CommandExtra::get(t));

    // Same thing for command containers
    TS_ASSERT(CommandExtra::get(t, 4) == 0);
    TS_ASSERT(CommandExtra::get(ct, 4) == 0);

    p->create(4);
    CommandContainer* cc = CommandExtra::get(t, 4);
    TS_ASSERT(cc != 0);
    TS_ASSERT_EQUALS(CommandExtra::get(ct, 4), cc);
}

