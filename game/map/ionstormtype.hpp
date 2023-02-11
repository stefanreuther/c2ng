/**
  *  \file game/map/ionstormtype.hpp
  *  \brief Class game::map::IonStormType
  */
#ifndef C2NG_GAME_MAP_IONSTORMTYPE_HPP
#define C2NG_GAME_MAP_IONSTORMTYPE_HPP

#include "game/map/ionstorm.hpp"
#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"

namespace game { namespace map {

    /** Ion Storm type.
        Contains all the active/visible ion storms. */
    class IonStormType : public ObjectVectorType<IonStorm> {
     public:
        /** Constructor.
            @param vec Ion Storm vector */
        explicit IonStormType(ObjectVector<IonStorm>& vec);

        // ObjectVectorType:
        virtual bool isValid(const IonStorm& s) const;
    };

} }

#endif
