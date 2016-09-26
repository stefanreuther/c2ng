/**
  *  \file game/vcr/classic/algorithm.cpp
  *  \brief Interface game::vcr::classic::Algorithm
  */

#include "game/vcr/classic/algorithm.hpp"

// Constructor.
game::vcr::classic::Algorithm::Algorithm(Visualizer& vis)
    : m_pVisualizer(&vis)
{ }

// Destructor.
game::vcr::classic::Algorithm::~Algorithm()
{ }

// Set visualizer.
void
game::vcr::classic::Algorithm::setVisualizer(Visualizer& vis)
{
    m_pVisualizer = &vis;
}

// Get visualizer.
game::vcr::classic::Visualizer&
game::vcr::classic::Algorithm::visualizer()
{
    return *m_pVisualizer;
}

// Play battle.
void
game::vcr::classic::Algorithm::playBattle(const Object& left, const Object& right, uint16_t seed)
{
    // ex VcrPlayer::playVcr
    // FIXME: what about initBattle() returning false? who does doneBattle()?
    initBattle(left, right, seed);
    playFastForward();
    while (playCycle()) {
        // empty
    }
}
