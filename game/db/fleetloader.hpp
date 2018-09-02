/**
  *  \file game/db/fleetloader.hpp
  *  \brief Class game::db::FleetLoader
  */
#ifndef C2NG_GAME_DB_FLEETLOADER_HPP
#define C2NG_GAME_DB_FLEETLOADER_HPP

#include "afl/charset/charset.hpp"
#include "afl/io/directory.hpp"
#include "game/map/universe.hpp"

namespace game { namespace db {

    /** Fleet File I/O.
        Provides methods to load and save the "fleetX.cc" file that defines fleets. */
    class FleetLoader {
     public:
        /** Constructor.
            \param cs Game character set */
        explicit FleetLoader(afl::charset::Charset& cs);

        /** Load fleets.

            This will make sure that only valid fleets are built, no matter what the file contains.
            If ships from a fleet were destroyed, they are removed; if a fleet leader was destroyed, another ship (if any) is appointed leader.

            Therefore, before this call, ship source flags (Ship::getShipSource()) need to be available,
            that is, the ship data needs to have been loaded.

            \param dir Game directory
            \param univ Target universe
            \param playerNumber Player number */
        void load(afl::io::Directory& dir, game::map::Universe& univ, int playerNumber);

        /** Save fleets.
            If there are any fleets, will write the "fleetX.cc" file; otherwise, erases it.

            \param dir Game directory
            \param univ Universe
            \param playerNumber Player number */
        void save(afl::io::Directory& dir, const game::map::Universe& univ, int playerNumber);

     private:
        afl::charset::Charset& m_charset;
    };

} }

#endif
