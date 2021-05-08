/**
  *  \file game/map/planetpredictor.hpp
  *  \brief Class game::map::PlanetPredictor
  */
#ifndef C2NG_GAME_MAP_PLANETPREDICTOR_HPP
#define C2NG_GAME_MAP_PLANETPREDICTOR_HPP

#include "game/map/planet.hpp"

namespace game { namespace map {

    class PlanetEffectors;

    /** Planet predictor.
        Stores a copy of a planet and computes turn predictions for it.
        The copy is updated in-place. */
    class PlanetPredictor {
     public:
        /** Constructor.
            \param planet Planet to work on (will be copied) */
        explicit PlanetPredictor(const Planet& planet);

        /** Compute one turn.
            \param eff           PlanetEffectors (ships hissing, terraforming, etc.)
            \param planetScores  Planet score definitions (for experience levels)
            \param config        Host configuration
            \param host          Host version */
        void computeTurn(const PlanetEffectors& eff,
                         const UnitScoreDefinitionList& planetScores,
                         const game::config::HostConfiguration& config,
                         const HostVersion& host);

        /** Access current status.
            \return current status */
        Planet& planet();
     private:
        Planet m_planet;
    };

} }

#endif
