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
        Packer(Turn& turn, afl::charset::Charset& cs);

        void addUfo(const structures::Ufo& ufo);
        void addPlanet(const structures::Planet& planet);
        void addShip(const structures::Ship& ship);
        void addShipTrack(int id, int turn, const structures::ShipTrackEntry& entry);

        void packUfo(structures::Ufo& out, const game::map::Ufo& in);
        void packPlanet(structures::Planet& out, const game::map::Planet& in);
        void packShip(structures::Ship& out, const game::map::Ship& in);


     private:
        Turn& m_turn;
        afl::charset::Charset& m_charset;
    };

} }

#endif
