/**
  *  \file game/map/anyshiptype.hpp
  *  \brief Class game::map::AnyShipType
  */
#ifndef C2NG_GAME_MAP_ANYSHIPTYPE_HPP
#define C2NG_GAME_MAP_ANYSHIPTYPE_HPP

#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"
#include "game/map/ship.hpp"

namespace game { namespace map {

    /** Any ships type.
        Contains all current (visible) ships. */
    class AnyShipType : public ObjectVectorType<Ship> {
     public:
        /** Constructor.
            @param vec Ship vector */
        explicit AnyShipType(ObjectVector<Ship>& vec);

        // ObjectVectorType:
        virtual bool isValid(const Ship& p) const;
    };

} }

#endif
