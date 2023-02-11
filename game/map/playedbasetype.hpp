/**
  *  \file game/map/playedbasetype.hpp
  *  \brief Class game::map::PlayedBaseType
  */
#ifndef C2NG_GAME_MAP_PLAYEDBASETYPE_HPP
#define C2NG_GAME_MAP_PLAYEDBASETYPE_HPP

#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"
#include "game/map/planet.hpp"

namespace game { namespace map {

    /** Played starbases type.
        Contains all playable starbases. */
    class PlayedBaseType : public ObjectVectorType<Planet> {
     public:
        /** Constructor.
            @param vec Planet vector (must live longer than PlayedBaseType) */
        explicit PlayedBaseType(ObjectVector<Planet>& vec);

        // ObjectVectorType:
        virtual bool isValid(const Planet& p) const;
    };

} }

#endif
