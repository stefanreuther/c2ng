/**
  *  \file client/vcr/configuration.cpp
  *  \brief Class client::vcr::Configuration
  */

#include "client/vcr/configuration.hpp"

#include <algorithm>
#include "afl/base/countof.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/string/format.hpp"

using game::config::UserConfiguration;

const int client::vcr::Configuration::FASTEST_SPEED;
const int client::vcr::Configuration::SLOWEST_SPEED;

namespace {
    const int DEFAULT_SPEED = 2;

    struct Item {
        int8_t interval;
        int8_t ticks;
    };

    // Definition of the speed values
    const Item DEFS[] = {
        { 15, 1 },              // 66 Hz
        { 20, 1 },              // 50 Hz
        { 20, 2 },              // 25 Hz
        { 20, 3 },              // 16 Hz
        { 20, 4 },              // 12 Hz
        { 20, 5 },              // 10 Hz
        { 30, 5 },              // 6 Hz
        { 30, 8 },              // 4 Hz
        { 30, 11 },             // 3 Hz
        { 30, 15 },             // 2 Hz
        { 30, 20 },             // 1 Hz
    };
}

client::vcr::Configuration::Configuration()
    // Defaults chosen to match UserConfiguration
    : m_speed(DEFAULT_SPEED), m_rendererMode(0), m_effectsMode(0), m_flakRendererMode(0), m_flakGrid(true)
{
    static_assert(countof(DEFS) == SLOWEST_SPEED - FASTEST_SPEED + 1, "countof DEFS");
}

void
client::vcr::Configuration::load(game::proxy::WaitIndicator& link, game::proxy::ConfigurationProxy& proxy)
{
    // Always in correct range
    setSpeed(proxy.getOption(link, UserConfiguration::Vcr_Speed));

    // Verified on use
    m_rendererMode     = proxy.getOption(link, UserConfiguration::Vcr_Renderer);
    m_effectsMode      = proxy.getOption(link, UserConfiguration::Vcr_Effects);
    m_flakRendererMode = proxy.getOption(link, UserConfiguration::Flak_Renderer);

    m_flakGrid = proxy.getOption(link, UserConfiguration::Flak_Grid) != 0;
}

void
client::vcr::Configuration::save(game::proxy::ConfigurationProxy& proxy)
{
    proxy.setOption(UserConfiguration::Vcr_Speed,     m_speed);
    proxy.setOption(UserConfiguration::Vcr_Renderer,  m_rendererMode);
    proxy.setOption(UserConfiguration::Vcr_Effects,   m_effectsMode);
    proxy.setOption(UserConfiguration::Flak_Renderer, m_flakRendererMode);
    proxy.setOption(UserConfiguration::Flak_Grid,     m_flakGrid ? 1 : 0);
}

void
client::vcr::Configuration::changeSpeed(int delta)
{
    setSpeed(m_speed + delta);
}

void
client::vcr::Configuration::setSpeed(int value)
{
    m_speed = std::max(FASTEST_SPEED, std::min(SLOWEST_SPEED, value));
}

void
client::vcr::Configuration::setRendererMode(game::config::UserConfiguration::RendererMode m)
{
    m_rendererMode = m;
}

void
client::vcr::Configuration::cycleRendererMode()
{
    ++m_rendererMode;
    m_rendererMode = getRendererMode();
}

void
client::vcr::Configuration::setEffectsMode(game::config::UserConfiguration::EffectsMode m)
{
    m_effectsMode = m;
}

void
client::vcr::Configuration::cycleEffectsMode()
{
    ++m_effectsMode;
    m_effectsMode = getEffectsMode();
}

void
client::vcr::Configuration::setFlakRendererMode(game::config::UserConfiguration::FlakRendererMode m)
{
    m_flakRendererMode = m;
}

void
client::vcr::Configuration::cycleFlakRendererMode()
{
    ++m_flakRendererMode;
    m_flakRendererMode = getFlakRendererMode();
}

void
client::vcr::Configuration::toggleFlakRendererMode(game::config::UserConfiguration::FlakRendererMode a,
                                                   game::config::UserConfiguration::FlakRendererMode b)
{
    if (m_flakRendererMode == a) {
        setFlakRendererMode(b);
    } else {
        setFlakRendererMode(a);
    }
}

void
client::vcr::Configuration::setFlakGrid(bool flag)
{
    m_flakGrid = flag;
}

void
client::vcr::Configuration::toggleFlakGrid()
{
    m_flakGrid = !m_flakGrid;
}

int
client::vcr::Configuration::getTickInterval() const
{
    return DEFS[m_speed-FASTEST_SPEED].interval;
}

int
client::vcr::Configuration::getNumTicksPerBattleCycle() const
{
    return DEFS[m_speed-FASTEST_SPEED].ticks;
}

int
client::vcr::Configuration::getSpeed() const
{
    return m_speed;
}

game::config::UserConfiguration::RendererMode
client::vcr::Configuration::getRendererMode() const
{
    switch (m_rendererMode) {
     case UserConfiguration::StandardRenderer:
     case UserConfiguration::TraditionalRenderer:
     case UserConfiguration::InterleavedRenderer:
        return UserConfiguration::RendererMode(m_rendererMode);
     default:
        return UserConfiguration::StandardRenderer;
    }
}

game::config::UserConfiguration::EffectsMode
client::vcr::Configuration::getEffectsMode() const
{
    switch (m_effectsMode) {
     case UserConfiguration::StandardEffects:
     case UserConfiguration::SimpleEffects:
        return UserConfiguration::EffectsMode(m_effectsMode);
     default:
        return UserConfiguration::StandardEffects;
    }
}

game::config::UserConfiguration::FlakRendererMode
client::vcr::Configuration::getFlakRendererMode() const
{
    switch (m_flakRendererMode) {
     case UserConfiguration::ThreeDMode:
     case UserConfiguration::FlatMode:
        return UserConfiguration::FlakRendererMode(m_flakRendererMode);
     default:
        return UserConfiguration::ThreeDMode;
    }
}

bool
client::vcr::Configuration::hasFlakGrid() const
{
    return m_flakGrid;
}

String_t
client::vcr::Configuration::getSpeedName(int speed, afl::string::Translator& tx)
{
    if (speed <= FASTEST_SPEED) {
        return tx("fastest");
    } else if (speed >= SLOWEST_SPEED) {
        return tx("slowest");
    } else {
        return afl::string::Format("%d", SLOWEST_SPEED - speed);
    }
}
