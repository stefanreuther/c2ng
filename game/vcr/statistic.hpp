/**
  *  \file game/vcr/statistic.hpp
  *  \brief Class game::vcr::Statistic
  */
#ifndef C2NG_GAME_VCR_STATISTIC_HPP
#define C2NG_GAME_VCR_STATISTIC_HPP

namespace game { namespace vcr {

    class Object;

    /** Battle statistic.
        This records statistics that cannot be obtained by observing before and after status of a fight:
        - minimum fighters aboard
        - number of torpedo hits inflicted on the enemy

        It can gather data for a single fight of the unit, or a running total for multiple fightes in a battle. */
    class Statistic {
     public:
        /** Default constructor. */
        Statistic();

        /** Initialize from VCR participant.

            Use numFights=0 to initialize for tracking a running total;
            use merge() with statistics produced by fights to update.

            Use numFights=1 to initialize for tracking a single fight;
            use handleFightersAboard() and handleTorpedoHit() to track the fight's status.

            \param obj Participant
            \param numFights Number of fights */
        void init(const Object& obj, int numFights);

        /** Record: Handle number of fighters aboard.
            Algorithm calls this when the number of fighters aboard this ship changes.
            If this sets a new minimum, record that.
            \param n Number of fighters */
        void handleFightersAboard(int n);

        /** Record: Handle torpedo hit.
            Algorithm calls this when this ship fires a torpedo that hits. */
        void handleTorpedoHit();

        /** Inquiry: Get number of torpedo hits.
            \return number of calls to handleTorpedoHit() */
        int getNumTorpedoHits() const;

        /** Inquiry: Get minimum fighters aboard.
            \return minimum number passed to handleFightersAboard() */
        int getMinFightersAboard() const;

        /** Inquiry: Get number of fights.
            \return num */
        int getNumFights() const;

        /** Merge with another statistic.
            Updates this object to represent the fight of both objects.
            \param other other statistic */
        void merge(const Statistic& other);

     private:
        int m_minFightersAboard;
        int m_numTorpedoHits;
        int m_numFights;
    };

} }

// Record: Handle number of fighters aboard.
inline void
game::vcr::Statistic::handleFightersAboard(int n)
{
    // ex VcrPlayer::setStatFighters
    if (n < m_minFightersAboard) {
        m_minFightersAboard = n;
    }
}

// Record: Handle torpedo hit.
inline void
game::vcr::Statistic::handleTorpedoHit()
{
    // ex VcrPlayer::setStatTorpHit
    ++m_numTorpedoHits;
}

// Inquiry: Get number of torpedo hits.
inline int
game::vcr::Statistic::getNumTorpedoHits() const
{
    return m_numTorpedoHits;
}

// Inquiry: Get minimum fighters aboard.
inline int
game::vcr::Statistic::getMinFightersAboard() const
{
    return m_minFightersAboard;
}

// Inquiry: Get number of fights.
inline int
game::vcr::Statistic::getNumFights() const
{
    return m_numFights;
}

#endif
