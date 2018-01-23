/**
  *  \file game/shipbuildorder.hpp
  *  \brief Class game::ShipBuildOrder
  */
#ifndef C2NG_GAME_SHIPBUILDORDER_HPP
#define C2NG_GAME_SHIPBUILDORDER_HPP

namespace game {

    /** Data container for a ship build order. */
    class ShipBuildOrder {
     public:
        /** Default constructor.
            Makes an empty (all-zero) order. */
        ShipBuildOrder();

        /** Get hull index.
            The interpretation is up to the context and can be a truehull index or hull number.
            Zero means no build order.
            \return hull index */
        int getHullIndex() const;

        /** Set hull index.
            The interpretation is up to the context and can be a truehull index or hull number.
            Zero means no build order.
            \param n hull index */
        void setHullIndex(int n);

        /** Get engine type.
            \return engine type */
        int getEngineType() const;

        /** Set engine type.
            \param n engine type */
        void setEngineType(int n);

        /** Get beam type.
            Can be zero if count is also zero.
            \return beam type */
        int getBeamType() const;

        /** Set beam type.
            Can be zero if count is also zero.
            \param n beam type */
        void setBeamType(int n);

        /** Get number of beams.
            \return number of beams */
        int getNumBeams() const;

        /** Set number of beams.
            \param n number of beams */
        void setNumBeams(int n);

        /** Get type of torpedo launchers.
            Can be zero if count is also zero.
            \return type */
        int getLauncherType() const;

        /** Set type of torpedo launchers.
            Can be zero if count is also zero.
            \param n type */
        void setLauncherType(int n);

        /** Get number of torpedo launchers.
            \return number */
        int getNumLaunchers() const;

        /** Set number of torpedo launchers.
            \param n number */
        void setNumLaunchers(int n);


        /** Canonicalize build order.
            If a weapon count is zero, its type does not matter and is thus set to zero,
            to make the representation unique. */
        void canonicalize();

        bool operator==(const ShipBuildOrder& other) const;
        bool operator!=(const ShipBuildOrder& other) const;

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
