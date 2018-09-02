/**
  *  \file game/ref/sortbynewlocation.hpp
  */
#ifndef C2NG_GAME_REF_SORTBYNEWLOCATION_HPP
#define C2NG_GAME_REF_SORTBYNEWLOCATION_HPP

#include "game/game.hpp"
#include "game/map/movementpredictor.hpp"
#include "game/map/universe.hpp"
#include "game/ref/sortpredicate.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace ref {

    class SortByNewLocation : public SortPredicate {
     public:
        SortByNewLocation(const game::map::Universe& univ,
                          const Game& game,
                          const game::spec::ShipList& shipList,
                          const Root& root,
                          afl::string::Translator& tx);

        virtual int compare(const Reference& a, const Reference& b) const;
        virtual String_t getClass(const Reference& a) const;

        bool getLocation(const Reference& a, game::map::Point& out) const;

     private:
        const game::map::Universe& m_universe;
        afl::string::Translator& m_translator;
        game::map::MovementPredictor m_predictor;
    };

} }

#endif
