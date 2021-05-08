/**
  *  \file game/map/drawingcontainer.cpp
  *  \brief Class game::map::DrawingContainer
  */

#include "game/map/drawingcontainer.hpp"
#include "game/map/configuration.hpp"

class game::map::DrawingContainer::Worker {
 public:
    virtual bool accept(const Drawing& d) = 0;
    virtual void handle(Drawing& d, List_t& list, Iterator_t& it) = 0;
};


// Create blank container.
game::map::DrawingContainer::DrawingContainer()
    : m_drawings()
{
    // ex GDrawingContainer::GDrawingContainer
}

game::map::DrawingContainer::~DrawingContainer()
{ }

// Add new drawing.
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

// Find nearest visible drawing.
game::map::DrawingContainer::Iterator_t
game::map::DrawingContainer::findNearestVisibleDrawing(Point pt, const Configuration& config, double maxDistance) const
{
    // ex GDrawingContainer::findNearestVisibleDrawing
    Iterator_t found = end();
    double minDistance = maxDistance;
    for (Iterator_t i = begin(), e = end(); i != e; ++i) {
        if (const Drawing* p = *i) {
            if (p->isVisible()) {
                double dist = p->getDistanceToWrap(pt, config);
                if (dist < minDistance) {
                    minDistance = dist;
                    found = i;
                }
            }
        }
    }
    return found;
}

// Find marker at a given position.
game::map::DrawingContainer::Iterator_t
game::map::DrawingContainer::findMarkerAt(Point pt, const Configuration& config) const
{
    // FIXME: it makes sense to locate the LAST marker
    Iterator_t i = begin();
    while (i != end()) {
        if (const Drawing* p = *i) {
            if (p->isVisible() && p->getType() == Drawing::MarkerDrawing && p->getPos() == pt) {
                break;
            }
        }
        ++i;
    }
    return i;
}

// Erase drawing.
void
game::map::DrawingContainer::erase(Iterator_t it)
{
    // ex GDrawingContainer::erase, accessor.pas:DeletePainting (sort-of)
    m_drawings.erase(it);
    sig_change.raise();
}

// Erase all expired drawings.
void
game::map::DrawingContainer::eraseExpiredDrawings(int turnNumber)
{
    // ex GDrawingContainer::eraseExpiredDrawings
    bool did = false;
    for (Iterator_t i = begin(), e = end(); i != e; ++i) {
        if (Drawing* p = *i) {
            if (p->getExpire() >= 0 && p->getExpire() < turnNumber) {
                m_drawings.erase(i);
                did = true;
            }
        }
    }
    if (did) {
        sig_change.raise();
    }
}

void
game::map::DrawingContainer::eraseAdjacentLines(Point pos, const Configuration& config)
{
    // ex deleteAdjacent
    class EraseWorker : public Worker {
     public:
        virtual bool accept(const Drawing&)
            { return true; }
        virtual void handle(Drawing&, List_t& list, Iterator_t& it)
            { list.erase(it); }
    };
    EraseWorker w;
    processAdjacent(pos, w, config);
}

void
game::map::DrawingContainer::setAdjacentLinesColor(Point pos, uint8_t color, const Configuration& config)
{
    // ex doColorizeAdjacent
    class ColorWorker : public Worker {
     public:
        ColorWorker(uint8_t color)
            : m_color(color)
            { }
        virtual bool accept(const Drawing& d)
            { return d.getColor() != m_color; }
        virtual void handle(Drawing& d, List_t&, Iterator_t&)
            { d.setColor(m_color); }
     private:
        uint8_t m_color;
    };
    ColorWorker w(color);
    processAdjacent(pos, w, config);
}

void
game::map::DrawingContainer::setAdjacentLinesTag(Point pos, util::Atom_t tag, const Configuration& config)
{
    // ex doTagAdjacent
    class TagWorker : public Worker {
     public:
        TagWorker(util::Atom_t tag)
            : m_tag(tag)
            { }
        virtual bool accept(const Drawing& d)
            { return d.getTag() != m_tag; }
        virtual void handle(Drawing& d, List_t&, Iterator_t&)
            { d.setTag(m_tag); }
     private:
        util::Atom_t m_tag;
    };
    TagWorker w(tag);
    processAdjacent(pos, w, config);
}

void
game::map::DrawingContainer::processAdjacent(Point pos, Worker& worker, const Configuration& config)
{
    bool did = false;
    Iterator_t i = begin();
    pos = config.getSimpleCanonicalLocation(pos);
    while (i != end()) {
        Drawing* d = *i;
        if (d != 0 && d->isVisible() && d->getType() == Drawing::LineDrawing && worker.accept(*d)) {
            if (config.getSimpleCanonicalLocation(d->getPos()) == pos) {
                // Starts at given position
                pos = config.getSimpleCanonicalLocation(d->getPos2());
                worker.handle(*d, m_drawings, i);
                i = begin();
                did = true;
            } else if (config.getSimpleCanonicalLocation(d->getPos2()) == pos) {
                // Ends at given position
                pos = config.getSimpleCanonicalLocation(d->getPos());
                worker.handle(*d, m_drawings, i);
                i = begin();
                did = true;
            } else {
                // No match
                ++i;
            }
        } else {
            ++i;
        }
    }
    if (did) {
        sig_change.raise();
    }
}
