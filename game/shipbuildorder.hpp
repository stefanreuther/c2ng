/**
  *  \file game/shipbuildorder.hpp
  *  \brief Class game::ShipBuildOrder
  */
#ifndef C2NG_GAME_SHIPBUILDORDER_HPP
#define C2NG_GAME_SHIPBUILDORDER_HPP

namespace game {

    class ShipBuildOrder {
     public:
        ShipBuildOrder();

        int getHullIndex() const;
        void setHullIndex(int n);
        int getEngineType() const;
        void setEngineType(int n);
        int getBeamType() const;
        void setBeamType(int n);
        int getNumBeams() const;
        void setNumBeams(int n);
        int getLauncherType() const;
        void setLauncherType(int n);
        int getNumLaunchers() const;
        void setNumLaunchers(int n);

     private:
        int m_hullIndex;
        int m_engineType;
        int m_beamType;
        int m_numBeams;
        int m_launcherType;
        int m_numLaunchers;
    };

}

#endif
