/**
  *  \file game/db/packer.hpp
  *  \brief Class game::db::Packer
  */
#ifndef C2NG_GAME_DB_PACKER_HPP
#define C2NG_GAME_DB_PACKER_HPP

#include "game/db/structures.hpp"
#include "game/map/ufo.hpp"
#include "game/turn.hpp"

namespace game { namespace db {

    /** Packers for starchart database. */
    class Packer {
     public:
        /** Constructor.
            \param cs Game character set */
        explicit Packer(afl::charset::Charset& cs);

        /** Decode an Ufo and add it to a turn.
            \param turn Turn
            \param ufo  Database Ufo record */
        void addUfo(Turn& turn, const structures::Ufo& ufo);

        /** Decode a planet and add it to a turn.
            \param turn   Turn
            \param planet Database planet record */
        void addPlanet(Turn& turn, const structures::Planet& planet);

        /** Decode a ship and add it to a turn.
            \param turn Turn
            \param ship Database ship record */
        void addShip(Turn& turn, const structures::Ship& ship);

        /** Decode a ship-track record and add it to a turn.
            \param turn   Turn
            \param id     Ship Id
            \param turnNr Turn number
            \param entry  Database ship track record */
        void addShipTrack(Turn& turn, int id, int turnNr, const structures::ShipTrackEntry& entry);


        /** Pack an Ufo into a database record.
            \param [out] out  Database record
            \param [in]  in   Ufo to pack */
        void packUfo(structures::Ufo& out, const game::map::Ufo& in);

        /** Pack a planet into a database record.
            \param [out] out  Database record
            \param [in]  in   Planet to pack */
        void packPlanet(structures::Planet& out, const game::map::Planet& in);

        /** Pack a ship into a database record.
            \param [out] out  Database record
            \param [in]  in   Ship to pack */
        void packShip(structures::Ship& out, const game::map::Ship& in);

     private:
        afl::charset::Charset& m_charset;
    };

} }

#endif
