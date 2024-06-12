/**
  *  \file game/vcr/classic/algorithm.cpp
  *  \brief Interface game::vcr::classic::Algorithm
  */

#include "game/vcr/classic/algorithm.hpp"

const int32_t game::vcr::classic::Algorithm::MAX_COORDINATE;

// ex VCRMF_MAX_FTRS. We don't need VCRMF_MAX_BEAMS, VCRMF_MAX_TORPS, VCRMF_MAX_BAYS
// because those values are known from the battle specs and we no longer use fixed-sized arrays.
const int game::vcr::classic::Algorithm::MAX_FIGHTER_TRACKS;


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
    initBattle(left, right, seed);
    playFastForward();
    while (playCycle()) {
        // empty
    }
}
