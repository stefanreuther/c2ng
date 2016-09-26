/**
  *  \file game/map/historyshiptype.hpp
  */
#ifndef C2NG_GAME_MAP_HISTORYSHIPTYPE_HPP
#define C2NG_GAME_MAP_HISTORYSHIPTYPE_HPP

#include "game/map/objectvectortype.hpp"
#include "game/map/ship.hpp"

namespace game { namespace map {

    class Ship;
    class Universe;

    /** History starships type.
        Contains all starships that have history (even if they are not visible now). */
    class HistoryShipType : public ObjectVectorType<Ship> {
     public:
        HistoryShipType(Universe& univ);

        virtual bool isValid(const Ship& p) const;
    };

} }

#endif
