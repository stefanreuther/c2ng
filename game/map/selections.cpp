/**
  *  \file game/map/selections.cpp
  *  \brief Class game::map::Selections
  */

#include "game/map/selections.hpp"
#include "afl/base/countof.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/historyshiptype.hpp"
#include "game/map/objectvector.hpp"
#include "game/map/universe.hpp"

// Constructor.
game::map::Selections::Selections()
    : sig_selectionChange(),
      m_currentLayer(0)
{ }

// Destructor.
game::map::Selections::~Selections()
{ }

// Clear all selections.
void
game::map::Selections::clear()
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
game::map::Selections::copyFrom(Universe& u, size_t layer)
{
    if (SelectionVector* p = get(Planet, layer)) {
        AnyPlanetType ty(u);
        p->copyFrom(ty);
    }
    if (SelectionVector* p = get(Ship, layer)) {
        HistoryShipType ty(u);
        p->copyFrom(ty);
    }
}

// Copy to universe.
void
game::map::Selections::copyTo(Universe& u, size_t layer) const
{
    if (const SelectionVector* p = get(Planet, layer)) {
        AnyPlanetType ty(u);
        p->copyTo(ty);
    }
    if (const SelectionVector* p = get(Ship, layer)) {
        HistoryShipType ty(u);
        p->copyTo(ty);
    }
}

// Limit to existing objects.
void
game::map::Selections::limitToExistingObjects(Universe& u, size_t layer)
{
    // ex GSelection::limitToExistingObjects
    if (SelectionVector* p = get(Planet, layer)) {
        AnyPlanetType ty(u);
        p->limitToExistingObjects(ty);
    }
    if (SelectionVector* p = get(Ship, layer)) {
        HistoryShipType ty(u);
        p->limitToExistingObjects(ty);
    }
}

// Execute compiled expression.
void
game::map::Selections::executeCompiledExpression(const String_t& compiledExpression, size_t targetLayer, Universe& u)
{
    // ex GMultiSelection::executeSelectionExpression
    // Save current state
    copyFrom(u, m_currentLayer);

    // Perform operation
    if (SelectionVector* p = get(Planet, targetLayer)) {
        p->executeCompiledExpression(compiledExpression, m_currentLayer, m_planets, u.planets().size(), true);
    }
    if (SelectionVector* p = get(Ship, targetLayer)) {
        p->executeCompiledExpression(compiledExpression, m_currentLayer, m_ships, u.ships().size(), false);
    }

    // Postprocess
    limitToExistingObjects(u, targetLayer);
    if (targetLayer == m_currentLayer) {
        copyTo(u, m_currentLayer);
    }
    sig_selectionChange.raise();
}

// Execute compiled expression on all layers.
void
game::map::Selections::executeCompiledExpressionAll(const String_t& compiledExpression, Universe& u)
{
    // Save current state
    copyFrom(u, m_currentLayer);

    // Perform operation on all layers
    for (size_t i = 0; i < NUM_LAYERS; ++i) {
        m_planets[i].executeCompiledExpression(compiledExpression, i, m_planets, u.planets().size(), true);
        m_ships  [i].executeCompiledExpression(compiledExpression, i, m_ships,   u.ships().size(),   false);
        limitToExistingObjects(u, i);
    }

    // Postprocess
    copyTo(u, m_currentLayer);
    sig_selectionChange.raise();
}

// Get current layer number.
size_t
game::map::Selections::getCurrentLayer() const
{
    // ex GMultiSelection::getCurrentSelectionLayer
    return m_currentLayer;
}

// Set current layer number.
void
game::map::Selections::setCurrentLayer(size_t newLayer, Universe& u)
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

// Get SelectionVector for one area.
game::map::SelectionVector*
game::map::Selections::get(Kind k, size_t layer)
{
    return get(k).at(layer);
}

const game::map::SelectionVector*
game::map::Selections::get(Kind k, size_t layer) const
{
    return const_cast<Selections&>(*this).get(k, layer);
}

// Get all SelectionVector's for one area.
afl::base::Memory<game::map::SelectionVector>
game::map::Selections::get(Kind k)
{
    switch (k) {
     case Ship:
        return afl::base::Memory<game::map::SelectionVector>(m_ships);
     case Planet:
        return afl::base::Memory<game::map::SelectionVector>(m_planets);
    }
    return afl::base::Nothing;
}

afl::base::Memory<const game::map::SelectionVector>
game::map::Selections::get(Kind k) const
{
    return const_cast<Selections&>(*this).get(k);
}

// Get number of layers.
size_t
game::map::Selections::getNumLayers() const
{
    return NUM_LAYERS;
}
