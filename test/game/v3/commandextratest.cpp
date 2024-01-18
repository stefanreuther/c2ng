/**
  *  \file test/game/v3/commandextratest.cpp
  *  \brief Test for game::v3::CommandExtra
  */

#include "game/v3/commandextra.hpp"

#include "afl/test/testrunner.hpp"
#include "game/turn.hpp"
#include "game/v3/commandcontainer.hpp"

/** Test event propagation. */
AFL_TEST("game.v3.CommandExtra:events", a)
{
    using game::v3::Command;

    // Create
    game::Turn turn;
    game::v3::CommandExtra testee(turn);

    // Add ship, planet, minefield
    game::map::Ship* sh = turn.universe().ships().create(42);
    game::map::Planet* pl = turn.universe().planets().create(23);
    game::map::Minefield* mf = turn.universe().minefields().create(15);
    a.checkNonNull("01", sh);
    a.checkNonNull("02", pl);
    a.checkNonNull("03", mf);

    sh->markClean();
    pl->markClean();
    mf->markClean();

    // Create CommandContainer for one player
    game::v3::CommandContainer& cc = testee.create(3);
    a.check("11. isDirty", !sh->isDirty());
    a.check("12. isDirty", !pl->isDirty());
    a.check("13. isDirty", !mf->isDirty());

    // Create unrelated commands
    cc.addCommand(Command::RemoteControl, 23, "control");
    cc.addCommand(Command::GivePlanet,    42, "9");
    cc.addCommand(Command::AddDropAlly,   15, "add");
    cc.addCommand(Command::ShowPlanet,    15, "7");
    a.check("21. isDirty", !sh->isDirty());
    a.check("22. isDirty", !pl->isDirty());
    a.check("23. isDirty", !mf->isDirty());

    // Ship command
    cc.addCommand(Command::RemoteControl, 42, "allow");
    a.check("31. isDirty", sh->isDirty());
    a.check("32. isDirty", !pl->isDirty());
    a.check("33. isDirty", !mf->isDirty());
    sh->markClean();

    // Planet command
    cc.addCommand(Command::GivePlanet, 23, "11");
    a.check("41. isDirty", !sh->isDirty());
    a.check("42. isDirty", pl->isDirty());
    a.check("43. isDirty", !mf->isDirty());
    pl->markClean();

    // Minefield command
    cc.addCommand(Command::ShowMinefield, 15, "1");
    a.check("51. isDirty", !sh->isDirty());
    a.check("52. isDirty", !pl->isDirty());
    a.check("53. isDirty", mf->isDirty());
    mf->markClean();

    // Clear
    cc.clear();
    a.check("61. isDirty", sh->isDirty());
    a.check("62. isDirty", pl->isDirty());
    a.check("63. isDirty", mf->isDirty());
}

/** Test access to CommandExtra. */
AFL_TEST("game.v3.CommandExtra:access", a)
{
    using game::v3::CommandExtra;
    using game::v3::CommandContainer;

    game::Turn t;
    const game::Turn& ct = t;

    // Initially, no CommandExtra present
    a.checkNull("01", CommandExtra::get(t));
    a.checkNull("02", CommandExtra::get(ct));

    // Create one
    CommandExtra::create(t);

    // Now it's there
    CommandExtra* p = CommandExtra::get(t);
    a.checkNonNull("11", p);
    a.checkEqual("12", CommandExtra::get(ct), CommandExtra::get(t));

    // Same thing for command containers
    a.checkNull("21", CommandExtra::get(t, 4));
    a.checkNull("22", CommandExtra::get(ct, 4));

    p->create(4);
    CommandContainer* cc = CommandExtra::get(t, 4);
    a.checkNonNull("31", cc);
    a.checkEqual("32", CommandExtra::get(ct, 4), cc);
}
