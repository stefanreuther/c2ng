/**
  *  \file game/spec/hull.hpp
  *  \brief Class game::spec::Hull
  */
#ifndef C2NG_GAME_SPEC_HULL_HPP
#define C2NG_GAME_SPEC_HULL_HPP

#include "game/spec/component.hpp"
#include "game/spec/hullfunctionassignmentlist.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"

namespace game { namespace spec {

    /** A hull.
        This class only holds data which it does not interpret or limit.

        In addition to the standard specification values, Hull stores hull function assignments
        and a picture number for internal use. */
    class Hull : public Component {
     public:
        /** Constructor.
            \param id Hull Id */
        explicit Hull(int id);

        /** Destructor. */
        ~Hull();

        /** Get external picture number.
            This number is provided by specification files.
            \return external picture number */
        int getExternalPictureNumber() const;

        /** Set external picture number.
            This number is provided by specification files.
            \param nr external picture number */
        void setExternalPictureNumber(int nr);

        /** Get internal picture number.
            This number defaults to the external picture number, but can be modified.
            It is used to locate the picture to use.
            \return internal picture number */
        int getInternalPictureNumber() const;

        /** Set internal picture number.
            This number defaults to the external picture number, but can be modified.
            It is used to locate the picture to use.
            \param nr internal picture number */
        void setInternalPictureNumber(int nr);

        /** Get fuel capacity.
            \return fuel capacity */
        int getMaxFuel() const;

        /** Set fuel capacity.
            \param maxFuel fuel capacity */
        void setMaxFuel(int maxFuel);

        /** Get maximum crew.
            \return maximum crew */
        int getMaxCrew() const;

        /** Set maximum crew.
            \param maxCrew maximum crew */
        void setMaxCrew(int maxCrew);

        /** Get number of engines.
            \return number of engines */
        int getNumEngines() const;

        /** Set number of engines.
            \param numEngines number of engines */
        void setNumEngines(int numEngines);

        /** Get cargo capacity.
            \return cargo capacity */
        int getMaxCargo() const;

        /** Set cargo capacity.
            \param maxCargo cargo capacity */
        void setMaxCargo(int maxCargo);

        /** Get number of fighter bays.
            \return number of fighter bays */
        int getNumBays() const;

        /** Set number of fighter bays.
            \param numBays number of fighter bays */
        void setNumBays(int numBays);

        /** Get maximum number of torpedo launchers.
            \return maximum launchers */
        int getMaxLaunchers() const;

        /** Set maximum number of torpedo launchers.
            \param maxTorpedoLaunchers maximum launchers */
        void setMaxLaunchers(int maxTorpedoLaunchers);

        /** Get maximum number of beams.
            \return maximum beams */
        int getMaxBeams() const;

        /** Set maximum number of beams.
            \param maxBeams maximum beams */
        void setMaxBeams(int maxBeams);

        /** Clear hull function assignments. */
        void clearHullFunctions();

        /** Modify (add/remove) hull function assignments.
            \param function Function to modify
            \param add Players to add (which have the function)
            \param remove Players to remove (which do not have the function)
            \param assignToHull true: function assigned to hulls (AssignedToHull), false: to ship (AssignedToShip)
            \see HullFunctionAssignmentList::change */
        void changeHullFunction(ModifiedHullFunctionList::Function_t function, PlayerSet_t add, PlayerSet_t remove, bool assignToHull);

        /** Get hull function assignment list.
            \param assignedToHull true: access functions assigned to hulls; false: to ships */
        HullFunctionAssignmentList& getHullFunctions(bool assignedToHull);

        /** Get hull function assignment list.
            \param assignedToHull true: access functions assigned to hulls; false: to ships */
        const HullFunctionAssignmentList& getHullFunctions(bool assignedToHull) const;

        /** Get cloak fuel usage.
            This function does NOT honor possible hull functions.
            In particular, it will return a nonzero value also for advanced cloakers.
            \param forPlayer Player
            \param config Host configuration
            \return fuel usage for cloaking for one turn */
        int getCloakFuelUsage(int forPlayer, const game::config::HostConfiguration& config) const;

        /** Get per-turn fuel usage.
            \param forPlayer Player
            \param fight false: per turn; true: per fight
            \param config Host configuration
            \return fuel usage per turn
            \see HostVersion::isEugeneGame() */
        int getTurnFuelUsage(int forPlayer, bool fight, const game::config::HostConfiguration& config) const;

        /** Get mine hit damage.
            \param forPlayer Player
            \param web false: regular mines; true: web mines
            \param host Host version
            \param config Host configuration
            \return mine hit damage */
        int getMineHitDamage(int forPlayer, bool web, const HostVersion& host, const game::config::HostConfiguration& config) const;

        /** Get number of build points required to build this hull.
            \param forPlayer Player
            \param host Host version
            \param config Host configuration
            \return number of build points */
        int getPointsToBuild(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const;

        /** Get number of build points awarded for killing a ship of this type.
            \param forPlayer Player
            \param host Host version
            \param config Host configuration
            \return number of build points */
        int getPointsForKilling(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const;

        /** Get number of build points awarded for scrapping a ship of this type.
            \param forPlayer Player
            \param host Host version
            \param config Host configuration
            \return number of build points */
        int getPointsForScrapping(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const;

     private:
        int m_externalPictureNumber;
        int m_internalPictureNumber;
        int m_maxFuel;
        int m_maxCrew;
        int m_numEngines;
        int m_maxCargo;
        int m_numBays;
        int m_maxLaunchers;
        int m_maxBeams;

        HullFunctionAssignmentList m_hullFunctions;
        HullFunctionAssignmentList m_shipFunctions;
    };

} }

#endif
