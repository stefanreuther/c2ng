/**
  *  \file game/db/packer.hpp
  */
#ifndef C2NG_GAME_DB_PACKER_HPP
#define C2NG_GAME_DB_PACKER_HPP

#include "game/turn.hpp"
#include "game/db/structures.hpp"
#include "game/map/ufo.hpp"

namespace game { namespace db {

    class Packer {
     public:
        explicit Packer(afl::charset::Charset& cs);

        void addUfo(Turn& turn, const structures::Ufo& ufo);
        void addPlanet(Turn& turn, const structures::Planet& planet);
        void addShip(Turn& turn, const structures::Ship& ship);
        void addShipTrack(Turn& turn, int id, int turnNr, const structures::ShipTrackEntry& entry);

        void packUfo(structures::Ufo& out, const game::map::Ufo& in);
        void packPlanet(structures::Planet& out, const game::map::Planet& in);
        void packShip(structures::Ship& out, const game::map::Ship& in);


     private:
        afl::charset::Charset& m_charset;
    };

} }

#endif
