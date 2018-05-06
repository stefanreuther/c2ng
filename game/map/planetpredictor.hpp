/**
  *  \file game/map/planetpredictor.hpp
  */
#ifndef C2NG_GAME_MAP_PLANETPREDICTOR_HPP
#define C2NG_GAME_MAP_PLANETPREDICTOR_HPP

#include "game/map/planet.hpp"

namespace game { namespace map {

    class PlanetEffectors;

    class PlanetPredictor {
     public:
        PlanetPredictor(const Planet& planet);

        void computeTurn(const PlanetEffectors& eff,
                         const game::config::HostConfiguration& config,
                         const HostVersion& host);

        Planet& planet();
     private:
        Planet m_planet;
    };

} }

#endif
