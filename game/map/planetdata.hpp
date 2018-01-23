/**
  *  \file game/map/planetdata.hpp
  */
#ifndef C2NG_GAME_MAP_PLANETDATA_HPP
#define C2NG_GAME_MAP_PLANETDATA_HPP

#include "game/types.hpp"

namespace game { namespace map {

    struct PlanetData {
        IntegerProperty_t  owner;
        StringProperty_t   friendlyCode;
        IntegerProperty_t  numMines;
        IntegerProperty_t  numFactories;
        IntegerProperty_t  numDefensePosts;
        LongProperty_t     minedNeutronium;
        LongProperty_t     minedTritanium;
        LongProperty_t     minedDuranium;
        LongProperty_t     minedMolybdenum;
        LongProperty_t     colonistClans;
        LongProperty_t     supplies;
        LongProperty_t     money;
        LongProperty_t     groundNeutronium;
        LongProperty_t     groundTritanium;
        LongProperty_t     groundDuranium;
        LongProperty_t     groundMolybdenum;
        IntegerProperty_t  densityNeutronium;
        IntegerProperty_t  densityTritanium;
        IntegerProperty_t  densityDuranium;
        IntegerProperty_t  densityMolybdenum;
        IntegerProperty_t  colonistTax;
        IntegerProperty_t  nativeTax;
        NegativeProperty_t colonistHappiness;
        NegativeProperty_t nativeHappiness;
        IntegerProperty_t  nativeGovernment;
        LongProperty_t     nativeClans;
        IntegerProperty_t  nativeRace;
        IntegerProperty_t  temperature;
        IntegerProperty_t  baseFlag;

        PlanetData(int = 0)
            { }
    };

} }

#endif
