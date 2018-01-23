/**
  *  \file game/map/reverter.hpp
  */
#ifndef C2NG_GAME_MAP_REVERTER_HPP
#define C2NG_GAME_MAP_REVERTER_HPP

#include "afl/base/deletable.hpp"
#include "game/types.hpp"
#include "game/map/planet.hpp"
#include "game/map/point.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/base/optional.hpp"

namespace game { namespace map {

    class Reverter : public afl::base::Deletable {
     public:
        enum Mode {
            Missions,
            Cargo
        };
        typedef afl::bits::SmallSet<Mode> Modes_t;
        
        // - determine number of structures to sell
        virtual afl::base::Optional<int> getMinBuildings(int planetId, PlanetaryBuilding building) const = 0;

        // - determine number of supplies to buy
        virtual int getSuppliesAllowedToBuy(int planetId) const = 0;

        // - determine number of starship components to sell
        virtual afl::base::Optional<int> getMinTechLevel(int planetId, TechLevel techLevel) const = 0;

        virtual afl::base::Optional<int> getMinBaseStorage(int planetId, TechLevel area, int slot) const = 0;

        virtual int getNumTorpedoesAllowedToSell(int planetId, int slot) const = 0;
        virtual int getNumFightersAllowedToSell(int planetId) const = 0;

        // // - determine previous friendly code
        // virtual String_t getPreviousShipFriendlyCode(int shipId) const = 0;
        // virtual String_t getPreviousPlanetFriendlyCode(int planetId) const = 0;

        // // - determine previous mission
        // virtual void getPreviousShipMission(int shipId, int& m, int& i, int& t) const = 0;

        // // - prepare/execute location reset
        // virtual void resetLocation(Point pt, Modes_t modes) const = 0;
    };

} }

#endif
