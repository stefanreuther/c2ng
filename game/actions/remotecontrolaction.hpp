/**
  *  \file game/actions/remotecontrolaction.hpp
  *  \brief Class game::actions::RemoteControlAction
  */
#ifndef C2NG_GAME_ACTIONS_REMOTECONTROLACTION_HPP
#define C2NG_GAME_ACTIONS_REMOTECONTROLACTION_HPP

#include "game/turn.hpp"

namespace game { namespace actions {

    /** PHost Remote Control action.
        Contains functions to query and modify remote-control state.
        If is intended as a very short-lived object.

        Note that this class does not check whether remote-control is actually allowed in the game
        (check HostConfiguration::CPEnableRemote). */
    class RemoteControlAction {
     public:
        /** Remote control state summary. */
        enum State {
            Forbidden,                   ///< Own ship, RC disallowed. ex rc_Forbidden.
            Normal,                      ///< Own ship, normal. ex rc_Normal.
            RemoteControlled,            ///< Foreign ship, we control it. ex rc_RemoteControlled.
            Applying,                    ///< Foreign ship, control requested. ex rc_Applying.
            Dropping,                    ///< Foreign ship, control being given back. ex rc_Dropping.
            Other,                       ///< Foreign ship, normal (we don't control). ex rc_Other.
            OtherForbidden,              ///< Foreign ship, RC disallowed. ex rc_OtherForbidden.
            OurRemoteControlled          ///< Own ship under foreign control. ex rc_OurRemoteControlled.
        };

        /** Remote control verbs.
            Note that the order is important here, each verb can be negated by '^1'.
            In addition, this is used as index into a number of tables. */
        enum Verb {
            Allow   = 0,    // ex rcv_Allow.
            Forbid  = 1,    // ex rcv_Forbid.
            Drop    = 2,    // ex rcv_Drop.
            Control = 3     // ex rcv_Control.
        };

        /** Constructor.
            \param turn     Turn
            \param shipId   Ship Id
            \param playerId Viewpoint player Id */
        RemoteControlAction(Turn& turn, Id_t shipId, int playerId);

        /** Get old remote-control state (beginning of turn).
            \return state */
        State getOldState() const;

        /** Get new remote-control state (end of turn, after processing of commands).
            \return state */
        State getNewState() const;

        /** Set remote control state.
            Tries to issue or remove a command corresponding to the given verb.
            For example, setState(Allow) will issue a "remote allow" command,
            or delete a contradictory other "remote" command.
            \param verb Verb
            \retval true A command has been given or removed to obtain the state
            \retval false Nothing changed because the state already was correct, or cannot be reached */
        bool setState(Verb verb);

        /** Toggle remote control state.
            Tries to issue or remove a command to reach the opposite of the current state
            (e.g. from "remote allow" to "remote forbid" and back).
            \return true on success */
        bool toggleState();

     private:
        Turn& m_turn;
        Id_t m_shipId;
        int m_playerId;
    };

} }

#endif
