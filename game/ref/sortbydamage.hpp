/**
  *  \file game/ref/sortbydamage.hpp
  */
#ifndef C2NG_GAME_REF_SORTBYDAMAGE_HPP
#define C2NG_GAME_REF_SORTBYDAMAGE_HPP

#include "game/ref/sortpredicate.hpp"
#include "game/map/universe.hpp"
#include "afl/string/translator.hpp"
#include "game/interpreterinterface.hpp"

namespace game { namespace ref {

    class SortByDamage : public SortPredicate {
     public:
        SortByDamage(const game::map::Universe& univ);

        virtual int compare(const Reference& a, const Reference& b) const;
        virtual String_t getClass(const Reference& a) const;

        int getDamage(const Reference& a) const;

     private:
        const game::map::Universe& m_universe;
    };

} }

#endif
