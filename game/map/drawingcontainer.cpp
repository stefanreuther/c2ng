/**
  *  \file game/map/drawingcontainer.cpp
  *  \brief Class game::map::DrawingContainer
  */

#include "game/map/drawingcontainer.hpp"
#include "game/map/configuration.hpp"

using game::map::Drawing;
using game::map::Point;
using game::MAX_NUMBER;
using game::parser::MessageInformation;

namespace {
    /* Parse MessageInformation into a Drawing object.
       Return drawing on success, or null. */
    Drawing* parseDrawing(const MessageInformation& info, util::AtomTable& atomTable)
    {
        // Check type
        Drawing::Type type;
        switch (info.getObjectType()) {
         case MessageInformation::MarkerDrawing:    type = Drawing::MarkerDrawing;    break;
         case MessageInformation::LineDrawing:      type = Drawing::LineDrawing;      break;
         case MessageInformation::RectangleDrawing: type = Drawing::RectangleDrawing; break;
         case MessageInformation::CircleDrawing:    type = Drawing::CircleDrawing;    break;
         default: return 0;
        }

        // Fetch X
        int32_t x, y;
        if (!info.getValue(game::parser::mi_X, x, 1, MAX_NUMBER) || !info.getValue(game::parser::mi_Y, y, 1, MAX_NUMBER)) {
            return 0;
        }

        // Create draft drawing
        std::auto_ptr<Drawing> d(new Drawing(Point(x, y), type));

        int32_t shape, radius, x2, y2;
        String_t comment;
        switch (type) {
         case Drawing::MarkerDrawing:
            // Requires shape
            if (!info.getValue(game::parser::mi_DrawingShape, shape, 0, Drawing::NUM_USER_MARKERS-1)) {
                return 0;
            }
            d->setMarkerKind(int(shape));

            // Optional comment
            if (info.getValue(game::parser::ms_DrawingComment, comment)) {
                d->setComment(comment);
            }
            break;

         case Drawing::CircleDrawing:
            // Requires radius
            if (!info.getValue(game::parser::mi_Radius, radius, 1, Drawing::MAX_CIRCLE_RADIUS)) {
                return 0;
            }
            d->setCircleRadius(radius);
            break;

         case Drawing::LineDrawing:
         case Drawing::RectangleDrawing:
            // Requires X2,Y2
            if (!info.getValue(game::parser::mi_EndX, x2, 1, MAX_NUMBER) || !info.getValue(game::parser::mi_EndY, y2, 1, MAX_NUMBER)) {
                return 0;
            }
            d->setPos2(Point(x2, y2));
            break;
        }

        // Common parameters:
        // - color
        int32_t color;
        if (info.getValue(game::parser::mi_Color, color, 0, Drawing::NUM_USER_COLORS)) {
            d->setColor(static_cast<uint8_t>(color));
        }

        // - tag
        String_t tag;
        if (info.getValue(game::parser::ms_DrawingTag, tag)) {
            d->setTag(atomTable.getAtomFromString(tag));
        }

        // - expire
        // If not given, defaults to 0, so markers created by message templates are temporary.
        int32_t expire;
        if (info.getValue(game::parser::mi_DrawingExpire, expire)) {
            d->setExpire(expire);
        } else {
            d->setExpire(0);
        }

        return d.release();
    }
}


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
    // ex GDrawingContainer::findNearestVisibleDrawing, chartusr.pas:FindNearestObject
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
game::map::DrawingContainer::findMarkerAt(Point pt) const
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

// Find a drawing.
game::map::DrawingContainer::Iterator_t
game::map::DrawingContainer::findDrawing(const Drawing& d) const
{
    Iterator_t i = begin();
    while (i != end()) {
        if (const Drawing* p = *i) {
            if (p->equals(d)) {
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
    // ex deleteAdjacent, chartusr.pas:DeleteAdjacent
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
    // ex doColorizeAdjacent, chartusr.pas:ColorizeAdjacent
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
    // ex doTagAdjacent, chartusr.pas:TagAdjacent
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
game::map::DrawingContainer::addMessageInformation(const game::parser::MessageInformation& info, util::AtomTable& atomTable)
{
    std::auto_ptr<Drawing> d(parseDrawing(info, atomTable));
    if (d.get() != 0) {
        if (findDrawing(*d) == end()) {
            addNew(d.release());
        }
    }
}

game::map::DrawingContainer::CheckResult
game::map::DrawingContainer::checkMessageInformation(const game::parser::MessageInformation& info, util::AtomTable& atomTable) const
{
    std::auto_ptr<Drawing> d(parseDrawing(info, atomTable));
    if (d.get() != 0) {
        if (findDrawing(*d) != end()) {
            return Found;
        } else {
            return NotFound;
        }
    } else {
        return Invalid;
    }
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
