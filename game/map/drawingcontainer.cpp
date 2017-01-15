/**
  *  \file game/map/drawingcontainer.cpp
  */

#include "game/map/drawingcontainer.hpp"

// /** Create blank container. */
game::map::DrawingContainer::DrawingContainer()
    : m_drawings()
{
    // ex GDrawingContainer::GDrawingContainer
}

game::map::DrawingContainer::~DrawingContainer()
{ }

// /** Add new drawing.
//     \param drawing Object, will be copied */
game::map::DrawingContainer::Iterator_t
game::map::DrawingContainer::addNew(Drawing* drawing)
{
    // ex GDrawingContainer::add
    if (drawing != 0) {
        Iterator_t it = m_drawings.insertNew(m_drawings.end(), drawing);
        sig_change.raise();
        return it;
    } else {
        return Iterator_t();
    }
}

// /** Find nearest visible drawing.
//     \param pt Location to start looking from
//     \return iterator pointing to drawing, end() if no drawing can be considered near */
game::map::DrawingContainer::Iterator_t
game::map::DrawingContainer::findNearestVisibleDrawing(Point pt, const Configuration& config)
{
    // ex GDrawingContainer::findNearestVisibleDrawing
    Iterator_t found = end();
    double minDistance = 21; /* minimum distance is less than 21 ly */
    for (Iterator_t i = begin(), e = end(); i != e; ++i) {
        if (const Drawing* p = *i) {
            double dist = p->getDistanceToWrap(pt, config);
            if (dist < minDistance) {
                minDistance = dist;
                found = i;
            }
        }
    }
    return found;
}

// /** Erase a drawing by iterator. */
void
game::map::DrawingContainer::erase(Iterator_t it)
{
    // ex GDrawingContainer::erase
    // sig_deleted.raise(it);
    m_drawings.erase(it);
    sig_change.raise();
}

// /** Erase all expired drawings.
//     \param turn Current turn number
//     \todo We can actually do this during loading, when we know the current turn number
//     there. Currently, we don't know it there. */
void
game::map::DrawingContainer::eraseExpiredDrawings(int turn)
{
    // ex GDrawingContainer::eraseExpiredDrawings
    bool did = false;
    for (Iterator_t i = begin(), e = end(); i != e; ++i) {
        if (Drawing* p = *i) {
            if (p->getExpire() >= 0 && p->getExpire() < turn) {
                m_drawings.erase(i);
                did = true;
            }
        }
    }
    if (did) {
        sig_change.raise();
    }
}
