/**
  *  \file game/ref/sortbyfleet.hpp
  */
#ifndef C2NG_GAME_REF_SORTBYFLEET_HPP
#define C2NG_GAME_REF_SORTBYFLEET_HPP

#include "game/ref/sortpredicate.hpp"
#include "game/map/universe.hpp"
#include "afl/string/translator.hpp"
#include "game/interpreterinterface.hpp"

namespace game { namespace ref {

    class SortByFleet : public SortPredicate {
     public:
        SortByFleet(const game::map::Universe& univ, afl::string::Translator& tx, InterpreterInterface& interface);

        virtual int compare(const Reference& a, const Reference& b) const;

        virtual String_t getClass(const Reference& a) const;

        int getFleetNumberKey(const Reference& a) const;

     private:
        const game::map::Universe& m_universe;
        afl::string::Translator& m_translator;
        InterpreterInterface& m_interface;
    };

} }

#endif
