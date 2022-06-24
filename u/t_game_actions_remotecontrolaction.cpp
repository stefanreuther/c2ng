/**
  *  \file u/t_game_actions_remotecontrolaction.cpp
  *  \brief Test for game::actions::RemoteControlAction
  */

#include "game/actions/remotecontrolaction.hpp"

#include "t_game_actions.hpp"
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
void
TestGameActionsRemoteControlAction::testEmpty()
{
    SimpleTurn t;
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);
    TS_ASSERT_EQUALS(testee.getOldState(), RemoteControlAction::Other);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::Other);
    TS_ASSERT_EQUALS(testee.setState(RemoteControlAction::Allow), false);
    TS_ASSERT_EQUALS(testee.setState(RemoteControlAction::Control), false);
    TS_ASSERT_EQUALS(testee.toggleState(), false);
}

/** Test own ship.
    A: create own ship.
    E: ship must be reported as normal; Forbid command can be given. */
void
TestGameActionsRemoteControlAction::testOwn()
{
    // Environment
    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, PLAYER, Object::Playable);
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    TS_ASSERT_EQUALS(testee.getOldState(), RemoteControlAction::Normal);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::Normal);

    // Allow not permitted because already allowed
    TS_ASSERT_EQUALS(testee.setState(RemoteControlAction::Allow), false);

    // Forbid succeeds
    TS_ASSERT_EQUALS(testee.setState(RemoteControlAction::Forbid), true);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::Forbidden);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(cmd->getArg(), "forbid");

    // Allow now succeeds
    TS_ASSERT_EQUALS(testee.setState(RemoteControlAction::Allow), true);
    cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    TS_ASSERT(!cmd);
}

/** Test own ship, drop command.
    A: create own ship. Give a drop command.
    E: command refused. */
void
TestGameActionsRemoteControlAction::testOwnDrop()
{
    // Environment
    SimpleTurn t;
    /*CommandContainer& cc =*/ CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, PLAYER, Object::Playable);
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    TS_ASSERT_EQUALS(testee.getOldState(), RemoteControlAction::Normal);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::Normal);

    // Allow not permitted because already allowed
    TS_ASSERT_EQUALS(testee.setState(RemoteControlAction::Drop), false);
}

/** Test own ship, forbidden remote control.
    A: create own ship that has remote control forbidden.
    E: ship must be reported as Forbidden; Allow command can be given. */
void
TestGameActionsRemoteControlAction::testOwnDisabled()
{
    // Environment
    MessageInformation info(MessageInformation::Ship, SHIP_ID, 0);
    info.addValue(game::parser::mi_ShipRemoteFlag, -1);

    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, PLAYER, Object::Playable).addMessageInformation(info, game::PlayerSet_t(PLAYER));
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    TS_ASSERT_EQUALS(testee.getOldState(), RemoteControlAction::Forbidden);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::Forbidden);

    // Toggle succeeds
    TS_ASSERT_EQUALS(testee.toggleState(), true);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::Normal);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(cmd->getArg(), "allow");
}

/** Test own ship controlled by other player.
    A: create ship controlled by other player, owned by us.
    E: ship must be reported as OurRemoteControlled; Forbid command can be given. */
void
TestGameActionsRemoteControlAction::testOwnControlled()
{
    // Environment
    MessageInformation info(MessageInformation::Ship, SHIP_ID, 0);
    info.addValue(game::parser::mi_ShipRemoteFlag, PLAYER);

    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, OTHER_PLAYER, Object::NotPlayable).addMessageInformation(info, game::PlayerSet_t(PLAYER));
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    TS_ASSERT_EQUALS(testee.getOldState(), RemoteControlAction::OurRemoteControlled);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::OurRemoteControlled);

    // Toggle succeeds
    TS_ASSERT_EQUALS(testee.toggleState(), true);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::Forbidden);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(cmd->getArg(), "forbid");
}

/** Test foreign ship.
    A: create foreign ship.
    E: ship must be reported as Forbidden; Allow command can be given. */
void
TestGameActionsRemoteControlAction::testForeign()
{
    // Environment
    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, OTHER_PLAYER, Object::NotPlayable);
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    TS_ASSERT_EQUALS(testee.getOldState(), RemoteControlAction::Other);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::Other);

    // Toggle succeeds
    TS_ASSERT_EQUALS(testee.toggleState(), true);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::Applying);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(cmd->getArg(), "control");
}

/** Test foreign ship, forbidden remote control.
    A: create own ship that has remote control forbidden.
    E: ship must be reported as OtherForbidden; Request command can be given. */
void
TestGameActionsRemoteControlAction::testForeignDisabled()
{
    // Environment
    MessageInformation info(MessageInformation::Ship, SHIP_ID, 0);
    info.addValue(game::parser::mi_ShipRemoteFlag, -1);

    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, OTHER_PLAYER, Object::NotPlayable).addMessageInformation(info, game::PlayerSet_t(PLAYER));
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    TS_ASSERT_EQUALS(testee.getOldState(), RemoteControlAction::OtherForbidden);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::OtherForbidden);

    // Toggle succeeds
    TS_ASSERT_EQUALS(testee.toggleState(), true);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::Applying);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(cmd->getArg(), "control");

    // Toggle succeeds again
    TS_ASSERT_EQUALS(testee.toggleState(), true);
    TS_ASSERT(cc.getCommand(Command::RemoteControl, SHIP_ID) == 0);
}

/** Test foreign ship, controlled by third party.
    A: create own ship that is controlled by a third player.
    E: ship must be reported as Other; Request command can be given. */
void
TestGameActionsRemoteControlAction::testForeignThird()
{
    // Environment
    MessageInformation info(MessageInformation::Ship, SHIP_ID, 0);
    info.addValue(game::parser::mi_ShipRemoteFlag, THIRD_PLAYER);

    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, OTHER_PLAYER, Object::NotPlayable).addMessageInformation(info, game::PlayerSet_t(PLAYER));
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    TS_ASSERT_EQUALS(testee.getOldState(), RemoteControlAction::Other);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::Other);

    // Toggle succeeds
    TS_ASSERT_EQUALS(testee.toggleState(), true);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::Applying);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(cmd->getArg(), "control");

    // Toggle succeeds again
    TS_ASSERT_EQUALS(testee.toggleState(), true);
    TS_ASSERT(cc.getCommand(Command::RemoteControl, SHIP_ID) == 0);
}

/** Test foreign ship, controlled by us.
    A: create ship owned by us that is actually owned by someone else (i.e. we control it).
    E: ship must be reported as RemoteControlled; Drop command can be given. */
void
TestGameActionsRemoteControlAction::testForeignControlled()
{
    // Environment
    MessageInformation info(MessageInformation::Ship, SHIP_ID, 0);
    info.addValue(game::parser::mi_ShipRemoteFlag, OTHER_PLAYER);

    SimpleTurn t;
    CommandContainer& cc = CommandExtra::create(t.turn()).create(PLAYER);
    t.addShip(SHIP_ID, PLAYER, Object::Playable).addMessageInformation(info, game::PlayerSet_t(PLAYER));
    RemoteControlAction testee(t.turn(), SHIP_ID, PLAYER);

    // Initial status
    TS_ASSERT_EQUALS(testee.getOldState(), RemoteControlAction::RemoteControlled);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::RemoteControlled);

    // Toggle succeeds
    TS_ASSERT_EQUALS(testee.toggleState(), true);
    TS_ASSERT_EQUALS(testee.getNewState(), RemoteControlAction::Dropping);

    // Command created
    const Command* cmd = cc.getCommand(Command::RemoteControl, SHIP_ID);
    TS_ASSERT(cmd);
    TS_ASSERT_EQUALS(cmd->getArg(), "drop");

    // Toggle succeeds again
    TS_ASSERT_EQUALS(testee.toggleState(), true);
    TS_ASSERT(cc.getCommand(Command::RemoteControl, SHIP_ID) == 0);
}

/** Test parseVerb(). */
void
TestGameActionsRemoteControlAction::testParseVerb()
{
    RemoteControlAction::Verb v;

    // Normal
    TS_ASSERT(RemoteControlAction::parseVerb("allow", v));
    TS_ASSERT_EQUALS(v, RemoteControlAction::Allow);

    TS_ASSERT(RemoteControlAction::parseVerb("forbid", v));
    TS_ASSERT_EQUALS(v, RemoteControlAction::Forbid);

    TS_ASSERT(RemoteControlAction::parseVerb("drop", v));
    TS_ASSERT_EQUALS(v, RemoteControlAction::Drop);

    TS_ASSERT(RemoteControlAction::parseVerb("control", v));
    TS_ASSERT_EQUALS(v, RemoteControlAction::Control);

    // Shortened
    TS_ASSERT(RemoteControlAction::parseVerb("a", v));
    TS_ASSERT_EQUALS(v, RemoteControlAction::Allow);

    // Errors
    TS_ASSERT(!RemoteControlAction::parseVerb("drops", v));
    TS_ASSERT(!RemoteControlAction::parseVerb("request", v));
    TS_ASSERT(!RemoteControlAction::parseVerb("", v));
}

