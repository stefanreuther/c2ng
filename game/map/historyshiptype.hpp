/**
  *  \file game/map/historyshiptype.hpp
  *  \brief Class game::map::HistoryShipType
  */
#ifndef C2NG_GAME_MAP_HISTORYSHIPTYPE_HPP
#define C2NG_GAME_MAP_HISTORYSHIPTYPE_HPP

#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"
#include "game/map/ship.hpp"

namespace game { namespace map {

    /** History starships type.
        Contains all starships that have history (even if they are not visible now). */
    class HistoryShipType : public ObjectVectorType<Ship> {
     public:
        HistoryShipType(ObjectVector<Ship>& vec);

        virtual bool isValid(const Ship& p) const;
    };

} }

#endif
