/**
  *  \file game/actions/techupgrade.hpp
  *  \brief Class game::actions::TechUpgrade
  */
#ifndef C2NG_GAME_ACTIONS_TECHUPGRADE_HPP
#define C2NG_GAME_ACTIONS_TECHUPGRADE_HPP

#include "game/actions/basebuildaction.hpp"
#include "game/types.hpp"

namespace game { namespace actions {

    /** Tech level upgrade.

        This action allows a starbase to buy or sell tech levels.
        It will hard-limit requests to permitted tech levels (according to rules/key),
        but allow configuring a tech upgrade that exceeds the resources of the planet. */
    class TechUpgrade : public BaseBuildAction {
     public:
        /** Constructor.
            \param planet    Planet to work on. Must have a played starbase.
            \param container Container to bill the builds on. Usually a PlanetStorage for the same planet.
            \param shipList  Ship list. Needed to access component costs and hull slots.
            \param root      Game root. Needed to access host configuration and registration key. */
        TechUpgrade(game::map::Planet& planet,
                    CargoContainer& container,
                    game::spec::ShipList& shipList,
                    Root& root);

        /** Set undo information.
            This enables this transaction to undo former builds.
            This uses the universe's reverter, if any.
            Changes on the universe will automatically be propagated.
            \param univ Universe. Must live longer than the TechUpgrade action. */
        void setUndoInformation(game::map::Universe& univ);

        /** Get current minimum tech level.
            This is affected by other component builds happening at this place.

            Note: This function's return value may lag behind if the environment has changed,
            but Universe::notifyListeners() has not yet been called.
            If this causes an out-of-range update to be configured, perform() will fix this up.

            \param area Area to check
            \return level */
        int getMinTechLevel(TechLevel area) const;

        /** Get maximum tech level.
            This is affected by the registration key.
            \param area Area to check
            \return level */
        int getMaxTechLevel(TechLevel area) const;

        /** Get current target tech level.
            \param area Area to check
            \return level */
        int getTechLevel(TechLevel area) const;

        /** Set new target tech level.
            \param area Area
            \param level New level
            \retval true Success
            \retval false New level not accepted (out of range). */
        bool setTechLevel(TechLevel area, int level);

        virtual void perform(BaseBuildExecutor& exec);

     private:
        int m_minTechLevels[NUM_TECH_AREAS];
        int m_newTechLevels[NUM_TECH_AREAS];

        afl::base::SignalConnection conn_undoChange;
        game::map::Universe* m_pUniverse;

        void onUndoChange();
        void updateUndoInformation();
    };

} }

#endif
