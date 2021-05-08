/**
  *  \file game/map/ionstormtype.hpp
  */
#ifndef C2NG_GAME_MAP_IONSTORMTYPE_HPP
#define C2NG_GAME_MAP_IONSTORMTYPE_HPP

#include "game/map/objectvectortype.hpp"
#include "game/map/ionstorm.hpp"

namespace game { namespace map {

    class Universe;

    class IonStormType : public ObjectVectorType<IonStorm> {
     public:
        IonStormType(Universe& univ);

        virtual bool isValid(const IonStorm& s) const;
    };

} }

#endif
