/**
  *  \file game/spec/shiplist.hpp
  *  \brief Class game::spec::ShipList
  */
#ifndef C2NG_GAME_SPEC_SHIPLIST_HPP
#define C2NG_GAME_SPEC_SHIPLIST_HPP

#include "afl/base/refcounted.hpp"
#include "afl/base/signal.hpp"
#include "game/reference.hpp"
#include "game/spec/basichullfunctionlist.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/componentvector.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/friendlycodelist.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/hullassignmentlist.hpp"
#include "game/spec/hullfunctionassignmentlist.hpp"
#include "game/spec/missionlist.hpp"
#include "game/spec/modifiedhullfunctionlist.hpp"
#include "game/spec/standardcomponentnameprovider.hpp"
#include "game/spec/torpedolauncher.hpp"

namespace game { namespace spec {

    /** Ship list.
        Aggregates all ship list information in a single object.

        - beams
        - engines
        - torpedo launchers
        - hulls
          - with hull function definitions ("cloak")
          - with modified hull function definitions ("cloak at level 2")
          - with hull function assigned as racial abilities and assigned to hulls
        - component namer
        - friendly codes */
    class ShipList : public afl::base::RefCounted {
     public:
        /** Constructor. */
        ShipList();

        /** Destructor. */
        ~ShipList();

        /** Access beams.
            \return ComponentVector of beams. Index is actual Id (1-based). */
        ComponentVector<Beam>& beams();
        const ComponentVector<Beam>& beams() const;

        /** Access engines.
            \return ComponentVector of engines. Index is actual Id (1-based). */
        ComponentVector<Engine>& engines();
        const ComponentVector<Engine>& engines() const;

        /** Access torpedo launchers.
            \return ComponentVector of torpedo launchers. Index is actual Id (1-based). */
        ComponentVector<TorpedoLauncher>& launchers();
        const ComponentVector<TorpedoLauncher>& launchers() const;

        /** Access hulls.
            \return ComponentVector of hulls. Index is actual Id (1-based). */
        ComponentVector<Hull>& hulls();
        const ComponentVector<Hull>& hulls() const;

        /** Access basic hull function definitions.
            This defines the basic hull functions and is constant per host version (PCC2: loaded from definition file).
            \return BasicHullFunctionList */
        BasicHullFunctionList& basicHullFunctions();
        const BasicHullFunctionList& basicHullFunctions() const;

        /** Access modified hull function definitions.
            A modified hull function is a basic hull function with a level restriction.
            This mapping is defined by the ship list; if no level restrictions exist, this is a 1:1 mapping.
            \return BasicHullFunctionList */
        ModifiedHullFunctionList& modifiedHullFunctions();
        const ModifiedHullFunctionList& modifiedHullFunctions() const;

        /** Access racial abilities.
            A racial ability is a ship ability that a race has on all their ships.
            This object contains the ready-made assignments, in the form of a
            (modified hull function Id, players added) mapping;
            configuration access is no longer needed.
            \return HullFunctionAssignmentList */
        HullFunctionAssignmentList& racialAbilities();
        const HullFunctionAssignmentList& racialAbilities() const;

        /** Access hull assignments.
            Stores the list of hulls each player is allowed to build.
            \return HullAssignmentList */
        HullAssignmentList& hullAssignments();
        const HullAssignmentList& hullAssignments() const;

        /** Access component namer.
            The component namer provides formatting rules for component (hull, engine, beam, torpedo) names.
            \return StandardComponentNameProvider */
        StandardComponentNameProvider& componentNamer();
        const StandardComponentNameProvider& componentNamer() const;

        /** Access friendly codes.
            \return FriendlyCodeList */
        FriendlyCodeList& friendlyCodes();
        const FriendlyCodeList& friendlyCodes() const;

        /** Access ship missions.
            \return MissionList */
        MissionList& missions();
        const MissionList& missions() const;

        /** Get a component, given a reference.
            \param ref Reference
            \return Component; null if reference does not point at a valid component */
        const Component* getComponent(Reference ref) const;

        /** Find racial abilities.
            We define a racial ability to be an ability which the given races have one every ship.
            We'll hide these during normal operation, to avoid cluttering up display real-estate with stuff everyone knows.

            This function will identify the abilities, remove them from the individual hulls if possible,
            and add them to racialAbilities() member.

            \param config Host configuration */
        void findRacialAbilities(const game::config::HostConfiguration& config);

        
        /** Enumerate all hull functions related to a hull.
            \param result [out] result will be appended here
            \param hullNr [in] Hull number
            \param config [in] Host configuration
            \param playerLimit [in] List  only functions accessible to these players
            \param levelLimit [in] List only functions accessible at these levels
            \param includeNewShip [in] Include functions assigned to newly-built ships
            \param includeRacialAbilities [in] Include racial abilities. */
        void enumerateHullFunctions(HullFunctionList& result,
                                    int hullNr,
                                    const game::config::HostConfiguration& config,
                                    PlayerSet_t playerLimit,
                                    ExperienceLevelSet_t levelLimit,
                                    bool includeNewShip,
                                    bool includeRacialAbilities) const;

        /** Get player mask for special function.
            \param basicFunctionId [in] basic function, hf_XXX (FIXME)
            \param hullNr [in] Hull number
            \param config [in] Host configuration
            \param levelLimit [in] List only functions accessible at these levels

            Change in c2ng: this always returns hull-specific abilities.
            The ability to return ship-specific abilities for new ships was removed.

            \return set of all players that can use this function. */
        PlayerSet_t getPlayersThatCan(int basicFunctionId,
                                      int hullNr,
                                      const game::config::HostConfiguration& config,
                                      ExperienceLevelSet_t levelLimit) const;

        /** Change notification. */
        afl::base::Signal<void()> sig_change;

     private:
        ComponentVector<Beam> m_beams;
        ComponentVector<Engine> m_engines;
        ComponentVector<TorpedoLauncher> m_launchers;
        ComponentVector<Hull> m_hulls;
        BasicHullFunctionList m_basicHullFunctions;
        ModifiedHullFunctionList m_modifiedHullFunctions;
        HullFunctionAssignmentList m_racialAbilities;
        HullAssignmentList m_hullAssignments;
        StandardComponentNameProvider m_componentNamer;
        FriendlyCodeList m_friendlyCodes;
        MissionList m_missions;
    };

} }

#endif
