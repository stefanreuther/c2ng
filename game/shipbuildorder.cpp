/**
  *  \file game/shipbuildorder.cpp
  *  \brief Class game::ShipBuildOrder
  */

#include "game/shipbuildorder.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/torpedolauncher.hpp"
#include "afl/string/format.hpp"

using afl::string::Format;

game::ShipBuildOrder::ShipBuildOrder()
    : m_hullIndex(0),
      m_engineType(0),
      m_beamType(0),
      m_numBeams(0),
      m_launcherType(0),
      m_numLaunchers(0)
{ }

int
game::ShipBuildOrder::getHullIndex() const
{
    return m_hullIndex;
}

void
game::ShipBuildOrder::setHullIndex(int n)
{
    m_hullIndex = n;
}

int
game::ShipBuildOrder::getEngineType() const
{
    return m_engineType;
}

void
game::ShipBuildOrder::setEngineType(int n)
{
    m_engineType = n;
}

int
game::ShipBuildOrder::getBeamType() const
{
    return m_beamType;
}

void
game::ShipBuildOrder::setBeamType(int n)
{
    m_beamType = n;
}

int
game::ShipBuildOrder::getNumBeams() const
{
    return m_numBeams;
}

void
game::ShipBuildOrder::setNumBeams(int n)
{
    m_numBeams = n;
}

int
game::ShipBuildOrder::getLauncherType() const
{
    return m_launcherType;
}

void
game::ShipBuildOrder::setLauncherType(int n)
{
    m_launcherType = n;
}

int
game::ShipBuildOrder::getNumLaunchers() const
{
    return m_numLaunchers;
}

void
game::ShipBuildOrder::setNumLaunchers(int n)
{
    m_numLaunchers = n;
}

void
game::ShipBuildOrder::canonicalize()
{
    // ex game/actions/baseship.cc:canonicalizeBuildOrder
    if (m_numLaunchers == 0) {
        m_launcherType = 0;
    }
    if (m_numBeams == 0) {
        m_beamType = 0;
    }
}

void
game::ShipBuildOrder::describe(afl::data::StringList_t& result, const game::spec::ShipList& shipList, afl::string::Translator& tx) const
{
    // ex formatBuildOrder
    const game::spec::Hull* pHull = shipList.hulls().get(m_hullIndex);
    if (pHull != 0) {
        // Hull
        const game::spec::ComponentNameProvider& namer = shipList.componentNamer();
        result.push_back(pHull->getName(namer));

        // Components
        // This format strings turn into "Transwarp Drive" when there's one, "7 x Transwarp Drive" when there are many.
        String_t fmt = tx("%!d%!1{%0$d \xC3\x97 %}%1$s");
        if (const game::spec::Engine* pEngine = shipList.engines().get(m_engineType)) {
            result.push_back(Format(fmt, pHull->getNumEngines(), pEngine->getName(namer)));
        }

        if (m_numBeams != 0) {
            if (const game::spec::Beam* pBeam = shipList.beams().get(m_beamType)) {
                result.push_back(Format(fmt, m_numBeams, pBeam->getName(namer)));
            }
        }

        if (m_numLaunchers != 0) {
            if (const game::spec::TorpedoLauncher* pTL = shipList.launchers().get(m_launcherType)) {
                result.push_back(Format(fmt, m_numLaunchers, pTL->getName(namer)));
            }
        }

        if (pHull->getNumBays() != 0) {
            result.push_back(Format(tx("%d fighter bay%!1{s%}"), pHull->getNumBays()));
        }
    }
}

String_t
game::ShipBuildOrder::toScriptCommand(String_t verb, const game::spec::ShipList* pShipList) const
{
    // ex makeBuildOrderCommand
    if (m_hullIndex == 0) {
        return verb + " 0";
    } else {
        String_t result = Format("%s %d, %d, %d, %d, %d, %d")
            << verb
            << m_hullIndex
            << m_engineType
            << m_beamType << m_numBeams
            << m_launcherType << m_numLaunchers;
        if (pShipList != 0) {
            if (game::spec::Hull* h = pShipList->hulls().get(m_hullIndex)) {
                result += afl::string::Format("   %% %s", h->getShortName(pShipList->componentNamer()));
            }
        }
        return result;
    }
}

bool
game::ShipBuildOrder::operator==(const ShipBuildOrder& other) const
{
    return m_hullIndex == other.m_hullIndex
        && m_engineType == other.m_engineType
        && m_beamType == other.m_beamType
        && m_numBeams == other.m_numBeams
        && m_launcherType == other.m_launcherType
        && m_numLaunchers == other.m_numLaunchers;
}

bool
game::ShipBuildOrder::operator!=(const ShipBuildOrder& other) const
{
    return !operator==(other);
}
