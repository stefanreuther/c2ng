/**
  *  \file game/ref/sortbyowner.hpp
  */
#ifndef C2NG_GAME_REF_SORTBYOWNER_HPP
#define C2NG_GAME_REF_SORTBYOWNER_HPP

#include "game/ref/sortpredicate.hpp"
#include "game/map/universe.hpp"

namespace game { namespace ref {

    class SortByOwner : public SortPredicate {
     public:
        SortByOwner(const game::map::Universe& univ, const PlayerList& players);
        virtual int compare(const Reference& a, const Reference& b) const;
        virtual String_t getClass(const Reference& a) const;

        int getOwner(const Reference& a) const;

     private:
        const game::map::Universe& m_universe;
        const PlayerList& m_players;
    };

} }

#endif
