/**
  *  \file game/spec/hull.hpp
  */
#ifndef C2NG_GAME_SPEC_HULL_HPP
#define C2NG_GAME_SPEC_HULL_HPP

#include "game/spec/component.hpp"
#include "game/spec/hullfunctionassignmentlist.hpp"

namespace game { namespace spec {

    class Hull : public Component {
     public:
        explicit Hull(int id);
        ~Hull();

        int getExternalPictureNumber() const;
        void setExternalPictureNumber(int nr);

        int getInternalPictureNumber() const;
        void setInternalPictureNumber(int nr);

        int getMaxFuel() const;
        void setMaxFuel(int maxFuel);

        int getMaxCrew() const;
        void setMaxCrew(int maxCrew);

        int getNumEngines() const;
        void setNumEngines(int numEngines);

        int getMaxCargo() const;
        void setMaxCargo(int maxCargo);

        int getNumBays() const;
        void setNumBays(int numBays);

        int getMaxLaunchers() const;
        void setMaxLaunchers(int maxTorpedoLaunchers);

        int getMaxBeams() const;
        void setMaxBeams(int maxBeams);

        void clearHullFunctions();
        void changeHullFunction(ModifiedHullFunctionList::Function_t function, PlayerSet_t add, PlayerSet_t remove, bool assignToHull);

        HullFunctionAssignmentList& getHullFunctions(bool assignedToHull);
        const HullFunctionAssignmentList& getHullFunctions(bool assignedToHull) const;

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
