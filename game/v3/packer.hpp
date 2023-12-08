/**
  *  \file game/v3/packer.hpp
  *  \brief Class game::v3::Packer
  */
#ifndef C2NG_GAME_V3_PACKER_HPP
#define C2NG_GAME_V3_PACKER_HPP

#include "game/v3/structures.hpp"
#include "game/map/planetdata.hpp"
#include "game/map/basedata.hpp"
#include "game/map/shipdata.hpp"

namespace game { namespace v3 {

    /** Conversion between on-disk and internal format.
        This class contains routines to convert between on-disk (game::v3::structures::Xx)
        and internal (game::map::XxData) formats.
        Those are required at multiple places in the v3 implementation. */
    class Packer {
     public:
        /** Constructor.
            \param cs Game character set */
        explicit Packer(afl::charset::Charset& cs);

        /** Unpack a ship.
            \param out [out] Target, internal
            \param in  [in] Source, on-disk
            \param remapExplore [in] Remap mission flag (true for SRace, false for classic) */
        void unpackShip(game::map::ShipData& out, const game::v3::structures::Ship& in, bool remapExplore);

        /** Unpack a planet.
            \param out [out] Target, internal
            \param in  [in] Source, on-disk */
        void unpackPlanet(game::map::PlanetData& out, const game::v3::structures::Planet& in);

        /** Unpack a starbase.
            \param out [out] Target, internal
            \param in  [in] Source, on-disk */
        void unpackBase(game::map::BaseData& out, const game::v3::structures::Base& in);

        /** Pack a ship.
            \param out [out] Target, on-disk
            \param id  [in] Ship Id
            \param in  [in] Source, internal
            \param remapExplore [in] Remap mission flag (true for SRace, false for classic) */
        void packShip(game::v3::structures::Ship& out, int id, const game::map::ShipData& in, bool remapExplore);

        /** Pack a planet.
            \param out [out] Target, on-disk
            \param id  [in] Planet Id
            \param in  [in] Source, internal */
        void packPlanet(game::v3::structures::Planet& out, int id, const game::map::PlanetData& in);

        /** Pack a starbase.
            \param out   [out] Target, on-disk
            \param id    [in] Base Id
            \param in    [in] Source, internal
            \param owner [in] Base owner */
        void packBase(game::v3::structures::Base& out, int id, const game::map::BaseData& in, int owner);

     private:
        afl::charset::Charset& m_charset;
    };

} }

#endif
