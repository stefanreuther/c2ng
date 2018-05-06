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

        /** Get beams. */
        ComponentVector<Beam>& beams();
        const ComponentVector<Beam>& beams() const;

        /** Get engines. */
        ComponentVector<Engine>& engines();
        const ComponentVector<Engine>& engines() const;

        /** Get torpedo launchers. */
        ComponentVector<TorpedoLauncher>& launchers();
        const ComponentVector<TorpedoLauncher>& launchers() const;

        /** Get hulls. */
        ComponentVector<Hull>& hulls();
        const ComponentVector<Hull>& hulls() const;

        /** Get basic hull function definitions. */
        BasicHullFunctionList& basicHullFunctions();
        const BasicHullFunctionList& basicHullFunctions() const;

        /** Get modified hull function definitions. */
        ModifiedHullFunctionList& modifiedHullFunctions();
        const ModifiedHullFunctionList& modifiedHullFunctions() const;

        /** Get racial abilities. */
        HullFunctionAssignmentList& racialAbilities();
        const HullFunctionAssignmentList& racialAbilities() const;

        /** Get hull function assignments. */
        HullAssignmentList& hullAssignments();
        const HullAssignmentList& hullAssignments() const;

        /** Get component namer. */
        StandardComponentNameProvider& componentNamer();
        const StandardComponentNameProvider& componentNamer() const;

        /** Get friendly codes. */
        FriendlyCodeList& friendlyCodes();
        const FriendlyCodeList& friendlyCodes() const;

        /** Get ship missions. */
        MissionList& missions();
        const MissionList& missions() const;

        /** Get a component, given a reference. */
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
