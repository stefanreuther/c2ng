/**
  *  \file game/interface/vmfile.hpp
  *  \brief VM File I/O
  */
#ifndef C2NG_GAME_INTERFACE_VMFILE_HPP
#define C2NG_GAME_INTERFACE_VMFILE_HPP

#include "game/session.hpp"

namespace game { namespace interface {

    /** Load script VM file.
        Loads the given player's VM file from the session root's game directory.

        @param [in,out] session  Session
        @param [in]     playerNr Player number */
    void loadVM(Session& session, int playerNr);

    /** Save script VM file.
        Saves the current VM state into the given player's VM file in the session root's game directory.

        @param [in,out] session  Session
        @param [in]     playerNr Player number */
    void saveVM(Session& session, int playerNr);

} }

#endif
