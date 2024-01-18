/**
  *  \file game/spec/engine.cpp
  *  \brief Class game::spec::Engine
  */

#include <cassert>
#include "game/spec/engine.hpp"
#include "afl/base/countof.hpp"

const int game::spec::Engine::MAX_WARP;

// Constructor.
game::spec::Engine::Engine(int id)
    : Component(ComponentNameProvider::Engine, id),
      m_maxEfficientWarp(0)
{
    for (size_t i = 0; i < countof(m_fuelFactors); ++i) {
        m_fuelFactors[i] = 0;
    }
}

// Destructor.
game::spec::Engine::~Engine()
{ }

// Set fuel factor.
void
game::spec::Engine::setFuelFactor(int warp, int32_t fuelFactor)
{
    if (warp > 0 && warp <= MAX_WARP) {
        m_fuelFactors[warp-1] = fuelFactor;
    }
}

// Get maximum efficient warp.
int
game::spec::Engine::getMaxEfficientWarp() const
{
    if (m_maxEfficientWarp != 0) {
        return m_maxEfficientWarp;
    } else {
        int warp = MAX_WARP;
        while (warp > 1 && m_fuelFactors[warp-1] > 120L * warp*warp) {
            --warp;
        }
        return warp;
    }
}

// Set maximum efficient warp.
void
game::spec::Engine::setMaxEfficientWarp(int warp)
{
    m_maxEfficientWarp = warp;
}
