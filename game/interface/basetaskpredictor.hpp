/**
  *  \file game/interface/basetaskpredictor.hpp
  *  \brief Class game::interface::BaseTaskPredictor
  */
#ifndef C2NG_GAME_INTERFACE_BASETASKPREDICTOR_HPP
#define C2NG_GAME_INTERFACE_BASETASKPREDICTOR_HPP

#include "game/config/hostconfiguration.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/taskpredictor.hpp"

namespace game { namespace interface {

    /** Starbase task predictor.
        Predicts starbase auto-tasks.
        Its main job is to keep track of starbase storage for build orders;
        it also tracks missions and friendly codes. */
    class BaseTaskPredictor : public interpreter::TaskPredictor {
     public:
        /** Constructor.
            \param p         Planet (will be copied)
            \param univ      Universe (for resolving recycle orders)
            \param shipList  Ship list (for resolving hull references)
            \param config    Configuration (for resolving hull references) */
        BaseTaskPredictor(const game::map::Planet& p,
                          const game::map::Universe& univ,
                          const game::spec::ShipList& shipList,
                          const game::config::HostConfiguration& config);

        /** Advance turn.
            If a build order is active, it will be performed. */
        void advanceTurn();

        /** Access planet.
            \return current state of planet */
        game::map::Planet& planet();

        // TaskPredictor:
        virtual bool predictInstruction(const String_t& name, interpreter::Arguments& args);

     private:
        game::map::Planet m_planet;
        const game::map::Universe& m_universe;
        const game::spec::ShipList& m_shipList;
        const game::config::HostConfiguration& m_config;

        void postBuildOrder(ShipBuildOrder order);
    };

} }

#endif
