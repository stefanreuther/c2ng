/**
  *  \file game/v3/registry.hpp
  *  \brief Winplan Game Registry
  */
#ifndef C2NG_GAME_V3_REGISTRY_HPP
#define C2NG_GAME_V3_REGISTRY_HPP

#include "afl/io/directory.hpp"
#include "game/timestamp.hpp"

namespace game { namespace v3 {

    /** Update Winplan game registry file (snooker.dat).
        Winplan keeps a registry of the 40 most recently seen timestamps, and refuses to open games not in that list.
        This function adds a timestamp to the list if required.

        PCC currently never reads these values.

        \param gameDirectory Game directory. Winplan data is expected in its parent.
        \param time Timestamp to add */
    void updateGameRegistry(afl::io::Directory& gameDirectory, const Timestamp& time);

} }

#endif
