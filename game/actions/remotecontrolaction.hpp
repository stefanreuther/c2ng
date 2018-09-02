/**
  *  \file game/actions/remotecontrolaction.hpp
  */
#ifndef C2NG_GAME_ACTIONS_REMOTECONTROLACTION_HPP
#define C2NG_GAME_ACTIONS_REMOTECONTROLACTION_HPP

#include "game/turn.hpp"

namespace game { namespace actions {

    class RemoteControlAction {
     public:
        // /** Remote control state summary. */
        enum State {
            Forbidden,                   ///< Own ship, RC disallowed. ex rc_Forbidden.
            Normal,                      ///< Own ship, normal. ex rc_Normal.
            RemoteControlled,            ///< Enemy ship, we control it. ex rc_RemoteControlled.
            Applying,                    ///< Enemy ship, control requested. ex rc_Applying.
            Dropping,                    ///< Enemy ship, control being given back. ex rc_Dropping.
            Other,                       ///< Enemy ship, normal (we don't control). ex rc_Other.
            OtherForbidden,              ///< Enemy ship, RC disallowed. ex rc_OtherForbidden.
            OurRemoteControlled          ///< Own ship under foreign control. ex rc_OurRemoteControlled.
        };

        // /** Remote control verbs. Note that the order is important here, each
        //     verb can be negated by '^1'. In addition, this is used as index
        //     into a number of tables. */
        enum Verb {
            Allow   = 0,    // ex rcv_Allow.
            Forbid  = 1,    // ex rcv_Forbid.
            Drop    = 2,    // ex rcv_Drop.
            Control = 3     // ex rcv_Control.
        };
        
        RemoteControlAction(Turn& turn, Id_t shipId, int playerId);

        State getOldState() const;

        State getNewState() const;

        bool setState(Verb verb);

        void toggleState();

     private:
        Turn& m_turn;
        Id_t m_shipId;
        int m_playerId;
    };

} }

#endif
