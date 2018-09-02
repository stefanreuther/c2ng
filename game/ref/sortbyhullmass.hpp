/**
  *  \file game/ref/sortbyhullmass.hpp
  */
#ifndef C2NG_GAME_REF_SORTBYHULLMASS_HPP
#define C2NG_GAME_REF_SORTBYHULLMASS_HPP

#include "game/ref/sortpredicate.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace ref {

    class SortByHullMass : public SortPredicate {
     public:
        SortByHullMass(const game::map::Universe& univ, const game::spec::ShipList& shipList);

        virtual int compare(const Reference& a, const Reference& b) const;
        virtual String_t getClass(const Reference& a) const;

        int getHullMass(const Reference& a) const;
        int getHullType(const Reference& a) const;

     private:
        const game::map::Universe& m_universe;
        const game::spec::ShipList& m_shipList;
    };        

} }

#endif
