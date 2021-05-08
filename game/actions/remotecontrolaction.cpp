/**
  *  \file game/actions/remotecontrolaction.cpp
  *  \brief Class game::actions::RemoteControlAction
  */

#include "game/actions/remotecontrolaction.hpp"
#include "afl/string/char.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"

using game::v3::Command;
using game::v3::CommandContainer;
using game::v3::CommandExtra;

namespace {

    /* Remote-control verbs. Indexed by Verb. */
    static const char*const RC_VERBS[] = {
        "allow",
        "forbid",
        "drop",
        "control",
    };

    /** Mapping of State to Verb that was used to enter this state.
        In particular, this also means that sending the opposite verb leaves the state. */
    static const uint8_t RC_VERB_INDEX[] = {
        game::actions::RemoteControlAction::Forbid,   // Forbidden
        game::actions::RemoteControlAction::Allow,    // Normal
        game::actions::RemoteControlAction::Control,  // RemoteControlled
        game::actions::RemoteControlAction::Control,  // Applying
        game::actions::RemoteControlAction::Drop,     // Dropping
        game::actions::RemoteControlAction::Drop,     // Other
        game::actions::RemoteControlAction::Drop,     // OtherForbidden
        game::actions::RemoteControlAction::Allow,    // OurRemoteControlled
    };
}


// Constructor.
game::actions::RemoteControlAction::RemoteControlAction(Turn& turn, Id_t shipId, int playerId)
    : m_turn(turn),
      m_shipId(shipId),
      m_playerId(playerId)
{ }

// Get old remote-control state (beginning of turn).
game::actions::RemoteControlAction::State
game::actions::RemoteControlAction::getOldState() const
{
    // ex getOldRemoteControlState, phost.pas::GetRCFlag
    const game::map::Ship*const pShip = m_turn.universe().ships().get(m_shipId);
    int shipOwner;
    if (pShip == 0 || !pShip->getOwner(shipOwner)) {
        // Ship not known: report normal state
        return Other;
    } else {
        // Normal case
        int rc = pShip->getRemoteControlFlag();

        // Special case. Is it used?
        if (rc == 0) {
            rc = shipOwner;
        }

        if (rc < 0) {
            // Remote control forbidden
            if (shipOwner == m_playerId) {
                return Forbidden;
            } else {
                return OtherForbidden;
            }
        } else if (rc == m_playerId) {
            // Remote control flag says it's ours
            if (shipOwner == m_playerId) {
                return Normal;
            } else {
                return OurRemoteControlled;
            }
        } else {
            // Remote control flag says it's someone else's
            if (shipOwner == m_playerId) {
                return RemoteControlled;
            } else {
                return Other;
            }
        }
    }
}

// Get new remote-control state (end of turn, after processing of commands).
game::actions::RemoteControlAction::State
game::actions::RemoteControlAction::getNewState() const
{
    // ex getNewRemoteControlState, phost.pas::GetNewRCFlag
    CommandContainer* cc = CommandExtra::get(m_turn, m_playerId);
    if (cc == 0) {
        return getOldState();
    }

    const Command* cmd = cc->getCommand(Command::RemoteControl, m_shipId);
    if (cmd == 0) {
        return getOldState();
    }

    // Check command
    const String_t& arg = cmd->getArg();
    if (!arg.empty()) {
        switch (afl::string::charToLower(arg[0])) {
         case 'c': return Applying;
         case 'a': return Normal;
         case 'd': return Dropping;
         case 'f': return Forbidden;
        }
    }

    // We end up here if the command is invalid
    return getOldState();
}

// Set remote control state.
bool
game::actions::RemoteControlAction::setState(Verb verb)
{
    // ex setRemoteControl, phost.pas::BringShipIntoRCState
    State oldState = getOldState();
    State newState = getNewState();

    bool result;

    CommandContainer* cc = CommandExtra::get(m_turn, m_playerId);
    if (cc == 0) {
        // Command cannot be given
        result = false;
    } else if (RC_VERB_INDEX[newState] == verb) {
        // I am already in the right state
        result = false;
    } else if (RC_VERB_INDEX[oldState] == verb) {
        // I was in the right state at the beginning of the turn
        // This will signal change via CommandContainer::sig_commandChange -> CommandExtra
        result = cc->removeCommand(Command::RemoteControl, m_shipId);
    } else if ((RC_VERB_INDEX[oldState] ^ 1) == verb) {
        // I can reach the desired state by issuing a command
        result = (cc->addCommand(Command::RemoteControl, m_shipId, RC_VERBS[verb]) != 0);
    } else {
        // The state cannot be reached, e.g. "drop" for a ship I own.
        result = false;
    }

    return result;
}

// Toggle remote control state.
bool
game::actions::RemoteControlAction::toggleState()
{
    // ex doRemoteControl (part)
    if (CommandContainer* cc = CommandExtra::get(m_turn, m_playerId)) {
        int newVerb = RC_VERB_INDEX[getNewState()] ^ 1;
        int oldVerb = RC_VERB_INDEX[getOldState()];

        if (newVerb == oldVerb) {
            cc->removeCommand(Command::RemoteControl, m_shipId);
        } else {
            cc->addCommand(Command::RemoteControl, m_shipId, RC_VERBS[newVerb]);
        }
        return true;
    } else {
        return false;
    }
}
