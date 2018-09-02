/**
  *  \file game/ref/sortbybattleorder.hpp
  */
#ifndef C2NG_GAME_REF_SORTBYBATTLEORDER_HPP
#define C2NG_GAME_REF_SORTBYBATTLEORDER_HPP

#include "game/ref/sortpredicate.hpp"
#include "game/hostversion.hpp"
#include "game/battleorderrule.hpp"

namespace game { namespace ref {

    class SortByBattleOrder : public SortPredicate {
     public:
        SortByBattleOrder(const game::map::Universe& univ, HostVersion host, afl::string::Translator& tx);

        virtual int compare(const Reference& a, const Reference& b) const;
        virtual String_t getClass(const Reference& a) const;

        int getBattleOrderValue(const Reference& a) const;

     private:
        const game::map::Universe& m_universe;
        BattleOrderRule m_rule;
        afl::string::Translator& m_translator;
    };

} }

#endif
