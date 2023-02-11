/**
  *  \file game/map/planetdata.hpp
  *  \brief Structure game::map::PlanetData
  */
#ifndef C2NG_GAME_MAP_PLANETDATA_HPP
#define C2NG_GAME_MAP_PLANETDATA_HPP

#include "game/types.hpp"

namespace game { namespace map {

    /** Planet data.
        This structure provides a lower-level access to the starbase data.
        It does not match binary storage layout, but also does not interpret the data;
        that is done by the Planet class. */
    struct PlanetData {
        IntegerProperty_t  owner;                  ///< Owner.
        StringProperty_t   friendlyCode;           ///< Friendly code.
        IntegerProperty_t  numMines;               ///< Number of mines.
        IntegerProperty_t  numFactories;           ///< Number of factories.
        IntegerProperty_t  numDefensePosts;        ///< Number of defense posts.
        LongProperty_t     minedNeutronium;        ///< Mined Neutronium.
        LongProperty_t     minedTritanium;         ///< Mined Tritanium.
        LongProperty_t     minedDuranium;          ///< Mined Duranium.
        LongProperty_t     minedMolybdenum;        ///< Mined Molybdenum.
        LongProperty_t     colonistClans;          ///< Colonist clans.
        LongProperty_t     supplies;               ///< Supplies.
        LongProperty_t     money;                  ///< Money.
        LongProperty_t     groundNeutronium;       ///< Ground Neutronium amount.
        LongProperty_t     groundTritanium;        ///< Ground Tritanium amount.
        LongProperty_t     groundDuranium;         ///< Ground Duranium amount.
        LongProperty_t     groundMolybdenum;       ///< Ground Molybdenum amount.
        IntegerProperty_t  densityNeutronium;      ///< Ground Neutronium density.
        IntegerProperty_t  densityTritanium;       ///< Ground Tritanium density.
        IntegerProperty_t  densityDuranium;        ///< Ground Duranium density.
        IntegerProperty_t  densityMolybdenum;      ///< Ground Molybdenum density.
        IntegerProperty_t  colonistTax;            ///< Colonist tax rate.
        IntegerProperty_t  nativeTax;              ///< Native tax rate.
        NegativeProperty_t colonistHappiness;      ///< Colonist happiness.
        NegativeProperty_t nativeHappiness;        ///< Native happiness.
        IntegerProperty_t  nativeGovernment;       ///< Native government.
        LongProperty_t     nativeClans;            ///< Native clans.
        IntegerProperty_t  nativeRace;             ///< Native race.
        IntegerProperty_t  temperature;            ///< Temperature.
        IntegerProperty_t  baseFlag;               ///< Starbase build/presence flag.

        /** Constructor.
            @param id ignored. This parameter exists to allow using an ObjectVector<PlanetData>. */
        PlanetData(int id = 0)
            { (void) id; }
    };

} }

#endif
