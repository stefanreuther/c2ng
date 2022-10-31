/**
  *  \file game/map/basedata.hpp
  */
#ifndef C2NG_GAME_MAP_BASEDATA_HPP
#define C2NG_GAME_MAP_BASEDATA_HPP

#include <vector>
#include "game/types.hpp"
#include "game/map/basestorage.hpp"
#include "game/shipbuildorder.hpp"

namespace game { namespace map {

    struct BaseData {
        IntegerProperty_t    numBaseDefensePosts;
        IntegerProperty_t    damage;
        IntegerProperty_t    techLevels[4];

        // Loader must set empty values to 0. User will not grow these arrays.
        BaseStorage          engineStorage;
        BaseStorage          hullStorage;
        BaseStorage          beamStorage;
        BaseStorage          launcherStorage;
        BaseStorage          torpedoStorage;
        IntegerProperty_t    numFighters;
        IntegerProperty_t    shipyardId;
        IntegerProperty_t    shipyardAction;
        IntegerProperty_t    mission;

        ShipBuildOrder       shipBuildOrder;

        BaseData(int = 0)
            { }
    };

    BaseStorage* getBaseStorage(BaseData& bd, TechLevel area);
    const BaseStorage* getBaseStorage(const BaseData& bd, TechLevel area);

} }

#endif
