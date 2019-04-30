/**
  *  \file game/interface/vmfile.hpp
  */
#ifndef C2NG_GAME_INTERFACE_VMFILE_HPP
#define C2NG_GAME_INTERFACE_VMFILE_HPP

#include "game/session.hpp"

namespace game { namespace interface {

    void loadVM(Session& session, int playerNr);
    void saveVM(Session& session, int playerNr);

} }

#endif
