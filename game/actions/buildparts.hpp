/**
  *  \file game/actions/buildparts.hpp
  *  \brief Class game::actions::BuildParts
  */
#ifndef C2NG_GAME_ACTIONS_BUILDPARTS_HPP
#define C2NG_GAME_ACTIONS_BUILDPARTS_HPP

#include <vector>
#include "game/actions/basebuildaction.hpp"
#include "afl/base/signalconnection.hpp"

namespace game { namespace actions {

    /** Build starship parts and store in starbase storage.
        Parts are indexed using
        - the tech level that defines the area (hulls/engines/beams/torpedo launchers)
        - the storage slot. This is the truehull (HullAssignmentList) slot for hulls,
          the actual component index for engines/beams/launchers. */
    class BuildParts : public BaseBuildAction {
     public:
        /** Constructor.
            \param planet    Planet to work on. Must have a played starbase.
            \param container Container to bill the builds on. Usually a PlanetStorage for the same planet.
            \param shipList  Ship list. Needed to access component costs and hull slots.
            \param root      Game root. Needed to access host configuration and registration key.
            \param tx        Translator. Needed for error messages during construction. */
        BuildParts(game::map::Planet& planet,
                   CargoContainer& container,
                   game::spec::ShipList& shipList,
                   Root& root,
                   afl::string::Translator& tx);

        /** Destructor. */
        ~BuildParts();

        /** Set undo information.
            This enables this transaction to undo former builds.
            This uses the universe's reverter, if any.
            Changes on the universe will automatically be propagated.
            \param univ Universe. Must live longer than the BuildParts action. */
        void setUndoInformation(game::map::Universe& univ);

        /** Get minimum number of parts that must remain.
            This limit honors the current build order.
            \param area Area containing part
            \param slot Slot of part
            \return minimum permitted amount */
        int getMinParts(TechLevel area, int slot) const;

        /** Get number of existing parts.
            This is the number of parts on the planet before this action.
            \param area Area containing part
            \param slot Slot of part
            \return amount */
        int getNumExistingParts(TechLevel area, int slot) const;

        /** Get current target number of parts.
            This is the number of parts adjusted by sales/purchases.
            \param area Area containing part
            \param slot Slot of part
            \return amount */
        int getNumParts(TechLevel area, int slot) const;

        /** Add parts.
            Note that this does not check how much we can pay for, only whether we can hold/sell that much.
            \param area Area containing part
            \param slot Slot of part
            \param amount Amount to add
            \param partial true: allow partial add/remove; false: execute change completely or not at all
            \return Number added/removed. With partial=false, either 0 or amount. */
        int add(TechLevel area, int slot, int amount, bool partial);

        // BaseBuildAction:
        virtual void perform(BaseBuildExecutor& exec);

     private:
        /*
         *  Data storage.
         *  We store only elements that were actually modified by the user.
         *  This saves us having to iterate through all possible components.
         */
        struct Element {
            TechLevel area;
            int slot;
            int target;
            Element(TechLevel area, int slot, int target)
                : area(area), slot(slot), target(target)
                { }
        };
        std::vector<Element> m_elements;

        const Element* find(TechLevel area, int slot) const;
        Element& findCreate(TechLevel area, int slot);

        /*
         *  Universe links for undo
         */
        game::map::Universe* m_pUniverse;
        afl::base::SignalConnection conn_universeChange;

        int computeMinParts(TechLevel area, int slot) const;
        void updateUndoInformation();

    };

} }

#endif
