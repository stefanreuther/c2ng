/**
  *  \file game/map/fleettype.hpp
  *  \brief Class game::map::FleetType
  */
#ifndef C2NG_GAME_MAP_FLEETTYPE_HPP
#define C2NG_GAME_MAP_FLEETTYPE_HPP

#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"
#include "game/map/ship.hpp"

namespace game { namespace map {

    class Universe;

    /** Fleet type.
        Contains all fleet leaders. */
    class FleetType : public ObjectVectorType<Ship> {
     public:
        /** Constructor.
            \param vec Ships */
        explicit FleetType(ObjectVector<Ship>& vec);

        // ObjectVectorType:
        virtual bool isValid(const Ship& s) const;

        /** Handle fleet change.
            If a change caused the current fleet to get invalid, this finds a new one.
            \param hint Hint for a possible fleet Id. May or may not be valid. */
        void handleFleetChange(Id_t hint);
    };

} }

#endif
