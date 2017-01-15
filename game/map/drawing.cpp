/**
  *  \file game/map/drawing.cpp
  */

#include <cmath>
#include <algorithm>
#include "game/map/drawing.hpp"
#include "game/map/configuration.hpp"

namespace {
    /** Check whether value approximately lies in range.
        \param testee [in] Value to test
        \param a,b    [in] Range
        \return true iff \c testee can be considered close to or between \c a and \c b */
    bool isInRangeApprox(int testee, int a, int b)
    {
        if (a > b) {
            std::swap(a, b);
        }

        return testee >= a - 20
            && testee <= b + 20;
    }
}

// /** Constructor. Make a new drawing.
//     \param pos Position or center point.
//     \param type Type of drawing.

//     All other fields are set to defaults and should be set using the
//     setXXX functions. */
game::map::Drawing::Drawing(Point pos, Type type)
    : m_pos(pos),
      m_type(type),
      m_color(1),
      m_x2(0),
      m_y2(0),
      m_tag(0),
      m_expire(-1),
      m_comment()
{
    // ex GDrawing::GDrawing
}

// /** Set position.
//     \param pos Position or center point. */
void
game::map::Drawing::setPos(Point pos)
{
    // ex GDrawing::setPos
    m_pos = pos;
}

// /** Set other position. Valid only for RectangleDrawing and LineDrawing.
//     \param pos Position of bottom/right corner. */
void
game::map::Drawing::setPos2(Point pos)
{
    // ex GDrawing::setPos2
    // ASSERT(type == RectangleDrawing || type == LineDrawing);
    m_x2 = pos.getX();
    m_y2 = pos.getY();
}

// /** Set circle radius. Valid only for CircleDrawing.
//     \param r New radius */
void
game::map::Drawing::setCircleRadius(int r)
{
    // ex GDrawing::setCircleRadius
    // ASSERT(type == CircleDrawing);
    m_x2 = r;
}

// /** Set marker kind (shape). Valid only for MarkerDrawing.
//     \param k New kind (see getUserMarker()) */
void
game::map::Drawing::setMarkerKind(int k)
{
    // ex GDrawing::setMarkerKind
    // ASSERT(type == MarkerDrawing);
    m_x2 = k;
}

// /** Set marker tag. This is a general-purpose numeric value, conventionally
//     an atom.
//     \param tag New tag */
void
game::map::Drawing::setTag(Atom_t tag)
{
    // ex GDrawing::setTag
    m_tag = tag;
}

// /** Set color. By convention, markers allow colors 0-9 and 15, as well
//     as 97-116. 0 means invisible.
//     \param color Color index (COLOR_xxx, see gfx/palette.h) */
// FIXME: we use a different color interpretation, namely: 1-30 user colors
void
game::map::Drawing::setColor(uint8_t color)
{
    // ex GDrawing::setColor
    m_color = color;
}

// /** Set comment. The comment is limited to 255 characters for storage.
//     \param comment New comment. */
void
game::map::Drawing::setComment(String_t comment)
{
    // ex GDrawing::setComment
    m_comment = comment;
}

// /** Set time of expiry. Specifies a turn number. When a turn after that
//     is seen, the marker is deleted (not loaded).
//     \param expire Turn of expiry, -1 for never */
void
game::map::Drawing::setExpire(int expire)
{
    // ex GDrawing::setExpire
    m_expire = expire;
}

// /** Compute distance of this drawing to a given point in the game
//     universe. This is used to select a drawing for editing. This is an
//     almost straightforward port of the same code in PCC 1.x. It does,
//     however, <em>not</em> special-case locked-on markers or check
//     visibility / editability; that must be done by the caller. */
double
game::map::Drawing::getDistanceTo(Point pt, const Configuration& config) const
{
    // ex GDrawing::getDistanceTo
    switch (m_type) {
     case LineDrawing:
        if (isInRangeApprox(pt.getX(), m_pos.getX(), m_x2) && isInRangeApprox(pt.getY(), m_pos.getY(), m_y2)) {
            /* position is approximately within bounding rectangle of line */
            long d0 = config.getSquaredDistance(m_pos, Point(m_x2, m_y2));
            if (d0 == 0) {
                /* degenerate case: line has length 0 */
                return std::sqrt(double(config.getSquaredDistance(m_pos, pt)));
            } else {
                /* regular case: compute distance between point and line */
                int32_t det = int32_t(pt.getY() - m_pos.getY()) * int32_t(m_x2 - m_pos.getX())
                            - int32_t(pt.getX() - m_pos.getX()) * int32_t(m_y2 - m_pos.getY());
                return std::abs(det / std::sqrt(double(d0)));
            }
        } else {
            /* position is outside bounding rectangle, don't bother looking further */
            /* FIXME: only the Line case still has this hardwired restriction */
            return 1e6;
        }

     case RectangleDrawing:
        // /* for rectangles, simply the distance to either edge */
        // /* FIXME: what about wrap? */
        if (isInRangeApprox(pt.getX(), m_pos.getX(), m_x2) && isInRangeApprox(pt.getY(), m_pos.getY(), m_y2)) {
            return std::min(std::min(std::abs(pt.getX() - m_pos.getX()),
                                     std::abs(pt.getX() - m_x2)),
                            std::min(std::abs(pt.getY() - m_pos.getY()),
                                     std::abs(pt.getY() - m_y2)));
        } else {
            return 1e6;
        }

     case CircleDrawing:
        return std::abs(std::sqrt(double(config.getSquaredDistance(m_pos, pt))) - m_x2);

     case MarkerDrawing:
        return std::sqrt(double(config.getSquaredDistance(m_pos, pt)));

     default:
        return 1e6;
    }
}

// /** Compute distance of this drawing to a given point in the game
//     universe. Same as getDistanceTo, but honors wrapping. */
double
game::map::Drawing::getDistanceToWrap(Point pt, const Configuration& config) const
{
    // ex GDrawing::getDistanceToWrap
    double dist = getDistanceTo(pt, config);
    for (int i = 1, n = config.getNumRectangularImages(); i < n; ++i) {
        Point tmp;
        if (config.getPointAlias(pt, tmp, i, false)) {
            double ndist = getDistanceTo(tmp, config);
            if (ndist < dist) {
                dist = ndist;
            }
        }
    }
    return dist;
}
