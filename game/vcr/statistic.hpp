/**
  *  \file game/vcr/statistic.hpp
  */
#ifndef C2NG_GAME_VCR_STATISTIC_HPP
#define C2NG_GAME_VCR_STATISTIC_HPP

namespace game { namespace vcr {

    class Object;

    /** Battle statistic.
        This records statistics that cannot be obtained by observing before and after status of a fight.
        - minimum fighters aboard
        - number of torpedo hits inflicted on the enemy */
    class Statistic {
     public:
        /** Default constructor. */
        Statistic();

        /** Initialize from VCR object. */
        Statistic(const Object& obj);

        /** Handle number of fighters aboard.
            If this sets a new minimum, record that.
            \param n Number of fighters */
        void handleFightersAboard(int n);

        /** Handle torpedo hit. */
        void handleTorpedoHit();

     private:
        int m_minFightersAboard;
        int m_torpedoHits;

        // FIXME: original code had this member; might not be needed
        // int m_numFights;
    };

} }

#endif
