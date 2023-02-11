/**
  *  \file game/map/basedata.hpp
  *  \brief Structure game::map::BaseData
  */
#ifndef C2NG_GAME_MAP_BASEDATA_HPP
#define C2NG_GAME_MAP_BASEDATA_HPP

#include "game/map/basestorage.hpp"
#include "game/shipbuildorder.hpp"
#include "game/types.hpp"

namespace game { namespace map {

    /** Starbase data.
        This structure provides a lower-level access to the starbase data.
        It does not match binary storage layout, but also does not interpret the data;
        that is done by the Planet class. */
    struct BaseData {
        IntegerProperty_t    numBaseDefensePosts;              ///< Number of defense posts.
        IntegerProperty_t    damage;                           ///< Damage level.
        IntegerProperty_t    techLevels[NUM_TECH_AREAS];       ///< Tech levels. Indexed by enum TechLevel.

        /* Component/torpedo storage.
           The loader must set empty values to 0; only those values will be accessible.
           The user will not grow these arrays. */
        BaseStorage          engineStorage;                    ///< Engines in storage. Indexed by engine type.
        BaseStorage          hullStorage;                      ///< Hulls in storage. Indexed by HullAssignmentList index, not hull type.
        BaseStorage          beamStorage;                      ///< Beams in storage. Indexed by beam type.
        BaseStorage          launcherStorage;                  ///< Torpedo launchers in storage. Indexed by torpedo type.
        BaseStorage          torpedoStorage;                   ///< Torpedoes in storage. Indexed by torpedo type.

        IntegerProperty_t    numFighters;                      ///< Number of fighters.
        IntegerProperty_t    shipyardId;                       ///< Ship being worked on in shipyard.
        IntegerProperty_t    shipyardAction;                   ///< Shipyard action.
        IntegerProperty_t    mission;                          ///< Starbase mission.

        ShipBuildOrder       shipBuildOrder;                   ///< Ship build order.

        /** Constructor.
            @param id ignored. This parameter exists to allow using an ObjectVector<BaseData>. */
        BaseData(int id = 0)
            { (void) id; }
    };

    /** Get BaseStorage instance for a component type.
        @param bd    BaseData
        @param area  Component type
        @return BaseStorage instance for that component (hull, engine, beam, launcher storage), null on error */
    BaseStorage* getBaseStorage(BaseData& bd, TechLevel area);

    /** Get BaseStorage instance for a component type (const version).
        @param bd    BaseData
        @param area  Component type
        @return BaseStorage instance for that component (hull, engine, beam, launcher storage), null on error */
    const BaseStorage* getBaseStorage(const BaseData& bd, TechLevel area);

} }

#endif
