/**
  *  \file test/game/actions/remotecontrolactiontest.cpp
  *  \brief Test for game::actions::RemoteControlAction
  */

#include "game/actions/remotecontrolaction.hpp"

#include "afl/test/testrunner.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/test/simpleturn.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"

using game::actions::RemoteControlAction;
using game::map::Object;
using game::parser::MessageInformation;
using game::test::SimpleTurn;
using game::v3::Command;
using game::v3::CommandContainer;
using game::v3::CommandExtra;

namespace {
    const int SHIP_ID = 99;
    const int PLAYER = 2;
    const int OTHER_PLAYER = 7;
    const int THIRD_PLAYER = 11;
}

/** Test behaviour on empty universe.
    A: construct RemoteControlAction on empty universe.
    E: must be able to access state; state changes report error. */
AFL_TEST("game.actions.RemoteControlAction:empty", a)
{
    SimpleTurn t;
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);
    a.checkEqual("01. getOldState", testee.getOldState(), RemoteControlAction::Other);
    a.checkEqual("02. getNewState", testee.getNewState(), RemoteControlAction::Other);
    a.checkEqual("03. setState", testee.setState(RemoteControlAction::Allow), false);
    a.checkEqual("04. setState", testee.setState(RemoteControlAction::Control), false);
    a.checkEqual("05. toggleState", testee.toggleState(), false);
}

/** Test own ship.
    A: create own ship.
    E: ship must be reported as normal; Forbid command can be given. */
AFL_TEST("game.actions.RemoteControlAction:own-ship:forbid", a)
{
    // Environment
    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, PLAYER, Object::Playable);
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    a.checkEqual("01. getOldState", testee.getOldState(), RemoteControlAction::Normal);
    a.checkEqual("02. getNewState", testee.getNewState(), RemoteControlAction::Normal);

    // Allow not permitted because already allowed
    a.checkEqual("11. setState", testee.setState(RemoteControlAction::Allow), false);

    // Forbid succeeds
    a.checkEqual("21. setState", testee.setState(RemoteControlAction::Forbid), true);
    a.checkEqual("22. getNewState", testee.getNewState(), RemoteControlAction::Forbidden);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    a.checkNonNull("31. cmd", cmd);
    a.checkEqual("32. getArg", cmd->getArg(), "forbid");

    // Allow now succeeds
    a.checkEqual("41. setState", testee.setState(RemoteControlAction::Allow), true);
    cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    a.checkNull("42. cmd", cmd);
}

/** Test own ship, drop command.
    A: create own ship. Give a drop command.
    E: command refused. */
AFL_TEST("game.actions.RemoteControlAction:own-ship:drop", a)
{
    // Environment
    SimpleTurn t;
    /*CommandContainer& cc =*/ CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, PLAYER, Object::Playable);
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    a.checkEqual("01. getOldState", testee.getOldState(), RemoteControlAction::Normal);
    a.checkEqual("02. getNewState", testee.getNewState(), RemoteControlAction::Normal);

    // Allow not permitted because already allowed
    a.checkEqual("11. setState", testee.setState(RemoteControlAction::Drop), false);
}

/** Test own ship, forbidden remote control.
    A: create own ship that has remote control forbidden.
    E: ship must be reported as Forbidden; Allow command can be given. */
AFL_TEST("game.actions.RemoteControlAction:own-ship:allow", a)
{
    // Environment
    MessageInformation info(MessageInformation::Ship, SHIP_ID, 0);
    info.addValue(game::parser::mi_ShipRemoteFlag, -1);

    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, PLAYER, Object::Playable).addMessageInformation(info, game::PlayerSet_t(PLAYER));
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    a.checkEqual("01. getOldState", testee.getOldState(), RemoteControlAction::Forbidden);
    a.checkEqual("02. getNewState", testee.getNewState(), RemoteControlAction::Forbidden);

    // Toggle succeeds
    a.checkEqual("11. toggleState", testee.toggleState(), true);
    a.checkEqual("12. getNewState", testee.getNewState(), RemoteControlAction::Normal);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    a.checkNonNull("21. cmd", cmd);
    a.checkEqual("22. getArg", cmd->getArg(), "allow");
}

/** Test own ship controlled by other player.
    A: create ship controlled by other player, owned by us.
    E: ship must be reported as OurRemoteControlled; Forbid command can be given. */
AFL_TEST("game.actions.RemoteControlAction:own-ship:controlled", a)
{
    // Environment
    MessageInformation info(MessageInformation::Ship, SHIP_ID, 0);
    info.addValue(game::parser::mi_ShipRemoteFlag, PLAYER);

    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, OTHER_PLAYER, Object::NotPlayable).addMessageInformation(info, game::PlayerSet_t(PLAYER));
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    a.checkEqual("01. getOldState", testee.getOldState(), RemoteControlAction::OurRemoteControlled);
    a.checkEqual("02. getNewState", testee.getNewState(), RemoteControlAction::OurRemoteControlled);

    // Toggle succeeds
    a.checkEqual("11. toggleState", testee.toggleState(), true);
    a.checkEqual("12. getNewState", testee.getNewState(), RemoteControlAction::Forbidden);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    a.checkNonNull("21. cmd", cmd);
    a.checkEqual("22. getArg", cmd->getArg(), "forbid");
}

/** Test foreign ship.
    A: create foreign ship.
    E: ship must be reported as Forbidden; Allow command can be given. */
AFL_TEST("game.actions.RemoteControlAction:foreign-ship:apply", a)
{
    // Environment
    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, OTHER_PLAYER, Object::NotPlayable);
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    a.checkEqual("01. getOldState", testee.getOldState(), RemoteControlAction::Other);
    a.checkEqual("02. getNewState", testee.getNewState(), RemoteControlAction::Other);

    // Toggle succeeds
    a.checkEqual("11. toggleState", testee.toggleState(), true);
    a.checkEqual("12. getNewState", testee.getNewState(), RemoteControlAction::Applying);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    a.checkNonNull("21. cmd", cmd);
    a.checkEqual("22. getArg", cmd->getArg(), "control");
}

/** Test foreign ship, forbidden remote control.
    A: create own ship that has remote control forbidden.
    E: ship must be reported as OtherForbidden; Request command can be given. */
AFL_TEST("game.actions.RemoteControlAction:foreign-ship:apply-forbidden", a)
{
    // Environment
    MessageInformation info(MessageInformation::Ship, SHIP_ID, 0);
    info.addValue(game::parser::mi_ShipRemoteFlag, -1);

    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, OTHER_PLAYER, Object::NotPlayable).addMessageInformation(info, game::PlayerSet_t(PLAYER));
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    a.checkEqual("01. getOldState", testee.getOldState(), RemoteControlAction::OtherForbidden);
    a.checkEqual("02. getNewState", testee.getNewState(), RemoteControlAction::OtherForbidden);

    // Toggle succeeds
    a.checkEqual("11. toggleState", testee.toggleState(), true);
    a.checkEqual("12. getNewState", testee.getNewState(), RemoteControlAction::Applying);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    a.checkNonNull("21. cmd", cmd);
    a.checkEqual("22. getArg", cmd->getArg(), "control");

    // Toggle succeeds again
    a.checkEqual("31. toggleState", testee.toggleState(), true);
    a.checkNull("32. getCommand", cc.getCommand(Command::RemoteControl, SHIP_ID));
}

/** Test foreign ship, controlled by third party.
    A: create own ship that is controlled by a third player.
    E: ship must be reported as Other; Request command can be given. */
AFL_TEST("game.actions.RemoteControlAction:foreign-ship:apply-third-party", a)
{
    // Environment
    MessageInformation info(MessageInformation::Ship, SHIP_ID, 0);
    info.addValue(game::parser::mi_ShipRemoteFlag, THIRD_PLAYER);

    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, OTHER_PLAYER, Object::NotPlayable).addMessageInformation(info, game::PlayerSet_t(PLAYER));
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    a.checkEqual("01. getOldState", testee.getOldState(), RemoteControlAction::Other);
    a.checkEqual("02. getNewState", testee.getNewState(), RemoteControlAction::Other);

    // Toggle succeeds
    a.checkEqual("11. toggleState", testee.toggleState(), true);
    a.checkEqual("12. getNewState", testee.getNewState(), RemoteControlAction::Applying);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    a.checkNonNull("21. cmd", cmd);
    a.checkEqual("22. getArg", cmd->getArg(), "control");

    // Toggle succeeds again
    a.checkEqual("31. toggleState", testee.toggleState(), true);
    a.checkNull("32. getCommand", cc.getCommand(Command::RemoteControl, SHIP_ID));
}

/** Test foreign ship, controlled by us.
    A: create ship owned by us that is actually owned by someone else (i.e. we control it).
    E: ship must be reported as RemoteControlled; Drop command can be given. */
AFL_TEST("game.actions.RemoteControlAction:foreign-ship:drop", a)
{
    // Environment
    MessageInformation info(MessageInformation::Ship, SHIP_ID, 0);
    info.addValue(game::parser::mi_ShipRemoteFlag, OTHER_PLAYER);

    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, PLAYER, Object::Playable).addMessageInformation(info, game::PlayerSet_t(PLAYER));
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    a.checkEqual("01. getOldState", testee.getOldState(), RemoteControlAction::RemoteControlled);
    a.checkEqual("02. getNewState", testee.getNewState(), RemoteControlAction::RemoteControlled);

    // Toggle succeeds
    a.checkEqual("11. toggleState", testee.toggleState(), true);
    a.checkEqual("12. getNewState", testee.getNewState(), RemoteControlAction::Dropping);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    a.checkNonNull("21. cmd", cmd);
    a.checkEqual("22. getArg", cmd->getArg(), "drop");

    // Toggle succeeds again
    a.checkEqual("31. toggleState", testee.toggleState(), true);
    a.checkNull("32. getCommand", cc.getCommand(Command::RemoteControl, SHIP_ID));
}

/** Test parseVerb(). */
AFL_TEST("game.actions.RemoteControlAction:parseVerb", a)
{
    RemoteControlAction::Verb v;

    // Normal
    a.check("01. allow", RemoteControlAction::parseVerb("allow", v));
    a.checkEqual("02. allow", v, RemoteControlAction::Allow);

    a.check("11. forbid", RemoteControlAction::parseVerb("forbid", v));
    a.checkEqual("12. forbid", v, RemoteControlAction::Forbid);

    a.check("21. drop", RemoteControlAction::parseVerb("drop", v));
    a.checkEqual("22. drop", v, RemoteControlAction::Drop);

    a.check("31. control", RemoteControlAction::parseVerb("control", v));
    a.checkEqual("32. control", v, RemoteControlAction::Control);

    // Shortened
    a.check("41. a", RemoteControlAction::parseVerb("a", v));
    a.checkEqual("42. a", v, RemoteControlAction::Allow);

    // Errors
    a.check("51. error", !RemoteControlAction::parseVerb("drops", v));
    a.check("52. error", !RemoteControlAction::parseVerb("request", v));
    a.check("53. error", !RemoteControlAction::parseVerb("", v));
}
