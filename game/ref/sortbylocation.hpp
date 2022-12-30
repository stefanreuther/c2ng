/**
  *  \file game/ref/sortbylocation.hpp
  */
#ifndef C2NG_GAME_REF_SORTBYLOCATION_HPP
#define C2NG_GAME_REF_SORTBYLOCATION_HPP

#include "game/map/universe.hpp"
#include "afl/string/translator.hpp"
#include "game/ref/sortpredicate.hpp"
#include "game/map/point.hpp"

namespace game { namespace ref {

    class SortByLocation : public SortPredicate {
     public:
        SortByLocation(const game::map::Universe& univ, afl::string::Translator& tx);

        virtual int compare(const Reference& a, const Reference& b) const;
        virtual String_t getClass(const Reference& a) const;

        String_t getClassFor(afl::base::Optional<game::map::Point> pt) const;
        int comparePositions(afl::base::Optional<game::map::Point> a, afl::base::Optional<game::map::Point> b) const;

        afl::base::Optional<game::map::Point> getLocation(const Reference& a) const;

     private:
        const game::map::Universe& m_universe;
        afl::string::Translator& m_translator;
    };

} }

#endif
