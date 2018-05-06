/**
  *  \file game/map/planeteffectors.hpp
  */
#ifndef C2NG_GAME_MAP_PLANETEFFECTORS_HPP
#define C2NG_GAME_MAP_PLANETEFFECTORS_HPP

#include "afl/base/types.hpp"

namespace game { namespace map {

    class PlanetEffectors {
     public:
        enum Kind {
            Hiss,
            RebelGroundAttack,
            Pillage,
            Meteor,
            HeatsTo50,
            CoolsTo50,
            HeatsTo100
        };
        static const size_t NUM_EFFECTS = HeatsTo100+1;

        PlanetEffectors();

        void clear();
        void add(Kind eff, int count);
        void set(Kind eff, int count);
        int get(Kind eff) const;
        int getNumTerraformers() const;

     private:
        int m_effectors[NUM_EFFECTS];
    };

} }

#endif
