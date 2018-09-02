/**
  *  \file game/ref/sortbyhulltype.hpp
  */
#ifndef C2NG_GAME_REF_SORTBYHULLTYPE_HPP
#define C2NG_GAME_REF_SORTBYHULLTYPE_HPP

#include "game/ref/sortpredicate.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace ref {

    class SortByHullType : public SortPredicate {
     public:
        SortByHullType(const game::map::Universe& univ, const game::spec::ShipList& shipList, afl::string::Translator& tx);

        virtual int compare(const Reference& a, const Reference& b) const;
        virtual String_t getClass(const Reference& a) const;

        int getHullType(const Reference& a) const;

     private:
        const game::map::Universe& m_universe;
        const game::spec::ShipList& m_shipList;
        afl::string::Translator& m_translator;
    };        

} }

#endif
