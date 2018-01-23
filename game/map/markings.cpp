/**
  *  \file game/map/markings.cpp
  *  \brief Class game::map::Markings
  */

#include "game/map/markings.hpp"
#include "afl/base/countof.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/historyshiptype.hpp"
#include "game/map/objectvector.hpp"
#include "game/map/universe.hpp"

// Constructor.
game::map::Markings::Markings()
    : sig_selectionChange(),
      m_currentLayer(0)
{ }

// Destructor.
game::map::Markings::~Markings()
{ }

// Clear all markings.
void
game::map::Markings::clear()
{
    // ex game/sel.cc:clearSelections
    for (size_t i = 0; i < countof(m_ships); ++i) {
        m_ships[i].clear();
    }
    for (size_t i = 0; i < countof(m_planets); ++i) {
        m_planets[i].clear();
    }
    m_currentLayer = 0;
    sig_selectionChange.raise();
}

// Copy from a universe.
void
game::map::Markings::copyFrom(Universe& u, size_t layer)
{
    if (MarkingVector* p = get(Planet, layer)) {
        AnyPlanetType ty(u);
        p->copyFrom(ty);
    }
    if (MarkingVector* p = get(Ship, layer)) {
        HistoryShipType ty(u);
        p->copyFrom(ty);
    }
}

// Copy to universe.
void
game::map::Markings::copyTo(Universe& u, size_t layer)
{
    if (MarkingVector* p = get(Planet, layer)) {
        AnyPlanetType ty(u);
        p->copyTo(ty);
    }
    if (MarkingVector* p = get(Ship, layer)) {
        HistoryShipType ty(u);
        p->copyTo(ty);
    }
}

// Limit to existing objects.
void
game::map::Markings::limitToExistingObjects(Universe& u, size_t layer)
{
    // ex GSelection::limitToExistingObjects
    if (MarkingVector* p = get(Planet, layer)) {
        AnyPlanetType ty(u);
        p->limitToExistingObjects(ty);
    }
    if (MarkingVector* p = get(Ship, layer)) {
        HistoryShipType ty(u);
        p->limitToExistingObjects(ty);
    }
}

// Execute compiled expression.
void
game::map::Markings::executeCompiledExpression(const String_t& compiledExpression, size_t targetLayer, Universe& u)
{
    // ex GMultiSelection::executeSelectionExpression
    // Save current state
    copyFrom(u, m_currentLayer);

    // Perform operation
    if (MarkingVector* p = get(Planet, targetLayer)) {
        p->executeCompiledExpression(compiledExpression, m_planets, u.planets().size(), true);
    }
    if (MarkingVector* p = get(Ship, targetLayer)) {
        p->executeCompiledExpression(compiledExpression, m_ships, u.ships().size(), false);
    }

    // Postprocess
    limitToExistingObjects(u, targetLayer);
    if (targetLayer == m_currentLayer) {
        copyTo(u, m_currentLayer);
    }
    sig_selectionChange.raise();
}

// Get current layer number.
size_t
game::map::Markings::getCurrentLayer() const
{
    // ex GMultiSelection::getCurrentSelectionLayer
    return m_currentLayer;
}

// Set current layer number.
void
game::map::Markings::setCurrentLayer(size_t newLayer, Universe& u)
{
    // ex GMultiSelection::setCurrentSelectionLayer
    // FIXME: PCC2 would have a way to accept this call even when no turn loaded
    if (newLayer != m_currentLayer) {
        copyFrom(u, m_currentLayer);
        m_currentLayer = newLayer;
        copyTo(u, m_currentLayer);
        limitToExistingObjects(u, m_currentLayer);
        sig_selectionChange.raise();
    }
}

// Get MarkingVector for one area.
game::map::MarkingVector*
game::map::Markings::get(Kind k, size_t layer)
{
    return get(k).at(layer);
}

// Get all MarkingVector's for one area.
afl::base::Memory<game::map::MarkingVector>
game::map::Markings::get(Kind k)
{
    switch (k) {
     case Ship:
        return afl::base::Memory<game::map::MarkingVector>(m_ships);
     case Planet:
        return afl::base::Memory<game::map::MarkingVector>(m_planets);
    }
    return afl::base::Nothing;
}

// Get number of layers.
size_t
game::map::Markings::getNumLayers() const
{
    return NUM_LAYERS;
}
