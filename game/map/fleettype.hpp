/**
  *  \file game/map/fleettype.hpp
  */
#ifndef C2NG_GAME_MAP_FLEETTYPE_HPP
#define C2NG_GAME_MAP_FLEETTYPE_HPP

#include "game/map/objectvectortype.hpp"
#include "game/map/ship.hpp"

namespace game { namespace map {

    class Universe;

    /** Any ships type.
        Contains all ships. */
    class FleetType : public ObjectVectorType<Ship> {
     public:
        FleetType(Universe& univ);

        virtual bool isValid(const Ship& p) const;

        /** Handle fleet change.
            If a change caused the current fleet to get invalid, this finds a new one.
            \param hintfid Hint for a possible fleet Id.
            May or may not be valid. */
        void handleFleetChange(Id_t hint);
    };

} }

#endif
