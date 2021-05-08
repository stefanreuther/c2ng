/**
  *  \file game/vcr/flak/nullvisualizer.hpp
  */
#ifndef C2NG_GAME_VCR_FLAK_NULLVISUALIZER_HPP
#define C2NG_GAME_VCR_FLAK_NULLVISUALIZER_HPP

#include "game/vcr/flak/visualizer.hpp"
#include "game/vcr/flak/battle.hpp"

namespace game { namespace vcr { namespace flak {

    class NullVisualizer : public Visualizer {
     public:
        NullVisualizer()
            : Visualizer()
            { }

        void createObject(const Algorithm::Object& obj, ship_t source);
        void destroyObject(const Algorithm::Object& obj, bool violent);
        void destroyShip(ship_t unit);
        void fireBeam(const Algorithm::Object& obj1, const Algorithm::Object& obj2);
        void fireBeam(const Algorithm::Object& obj1, ship_t unit2);
        void fireBeam(ship_t unit1, const Algorithm::Object& obj2, bool hits);
        void fireBeam(ship_t unit1, ship_t unit2, bool hits);
        void hitShip(ship_t unit, int expl, int kill, int death_flag);
    };

} } }

#endif
