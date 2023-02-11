/**
  *  \file game/map/drawing.cpp
  *  \brief Class game::map::Drawing
  */

#include <cmath>
#include <algorithm>
#include "game/map/drawing.hpp"
#include "game/map/configuration.hpp"
#include "util/math.hpp"

namespace {
    bool isInRange(int t, int a, int b)
    {
        if (a > b) {
            std::swap(a, b);
        }

        return t >= a && t <= b;
    }

    long scalarProduct(game::map::Point p1, game::map::Point p2)
    {
        return long(p1.getX())*long(p2.getX()) + long(p1.getY())*long(p2.getY());
    }
}

const int game::map::Drawing::NUM_USER_COLORS;
const int game::map::Drawing::NUM_USER_MARKERS;
const int game::map::Drawing::MAX_CIRCLE_RADIUS;


// Constructor.
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
    // @change Sensible defaults (previously in doDraw):
    switch (type) {
     case LineDrawing:
     case RectangleDrawing:
        setPos2(pos);
        break;

     case CircleDrawing:
        setCircleRadius(10);
        break;

     case MarkerDrawing:
        break;
    }
}

// Construct marker from template (canned marker).
game::map::Drawing::Drawing(Point pos, const game::config::MarkerOption::Data& tpl)
    : m_pos(pos),
      m_type(MarkerDrawing),
      m_color(tpl.color),
      m_x2(tpl.markerKind),
      m_y2(0),
      m_tag(0),
      m_expire(-1),
      m_comment()
{ }

// Set position.
void
game::map::Drawing::setPos(Point pos)
{
    // ex GDrawing::setPos
    m_pos = pos;
}

// Set other position.
void
game::map::Drawing::setPos2(Point pos)
{
    // ex GDrawing::setPos2
    m_x2 = pos.getX();
    m_y2 = pos.getY();
}

// Set radius.
void
game::map::Drawing::setCircleRadius(int r)
{
    // ex GDrawing::setCircleRadius
    m_x2 = r;
}

// Set marker kind (shape).
void
game::map::Drawing::setMarkerKind(int k)
{
    // ex GDrawing::setMarkerKind
    m_x2 = k;
}

// Set marker tag.
void
game::map::Drawing::setTag(Atom_t tag)
{
    // ex GDrawing::setTag
    m_tag = tag;
}

// Set color.
void
game::map::Drawing::setColor(uint8_t color)
{
    // ex GDrawing::setColor
    m_color = color;
}

// Set comment.
void
game::map::Drawing::setComment(String_t comment)
{
    // ex GDrawing::setComment
    m_comment = comment;
}

// Set time of expiry.
void
game::map::Drawing::setExpire(int expire)
{
    // ex GDrawing::setExpire
    m_expire = expire;
}

// Compare for equality.
bool
game::map::Drawing::equals(const Drawing& other) const
{
    return m_pos == other.m_pos
        && m_type == other.m_type
        && m_color == other.m_color
        && m_x2 == other.m_x2
        && m_y2 == other.m_y2
        && m_tag == other.m_tag
        && m_expire == other.m_expire
        && m_comment == other.m_comment;
}

// Compute distance of this drawing to a given point in the game universe.
double
game::map::Drawing::getDistanceTo(Point pt) const
{
    // ex GDrawing::getDistanceTo
    switch (m_type) {
     case LineDrawing: {
        const long d0 = m_pos.getSquaredRawDistance(Point(m_x2, m_y2));
        if (d0 == 0) {
            /* degenerate case: line has length 0 */
            return std::sqrt(double(m_pos.getSquaredRawDistance(pt)));
        } else {
            /* Distance to the line is length of the altitude of `pt`, in the triangle Pos/Pos2/pt.
               However, the altitude's foot actually needs to end up between Pos/Pos2.

               The foot point is Pos + ((pt-Pos).dir)/(dir.dir) * dir,
               with . marking scalar product, and dir = Pos2-Pos being the direction vector.

               Simplified, ((pt-Pos).dir)/(dir.dir) needs to be in [0,1] range.
               Given that (dir.dir) = |dir|^2 = d0 is always positive,
               we check ((pt-Pos).dir) for range [0, d0]. */
            Point dir = Point(m_x2, m_y2) - m_pos;
            long num = scalarProduct(pt - m_pos, dir);
            if (num >= 0 && num <= d0) {
                /* regular case: compute distance between point and line:
                   |(pt-Pos) x dir| / |dir|, where x denotes vector product.
                   Given two 2D vectors, |a x b| = xa*yb - xb*ya. */
                const int32_t det = int32_t(pt.getY() - m_pos.getY()) * int32_t(m_x2 - m_pos.getX())
                                  - int32_t(pt.getX() - m_pos.getX()) * int32_t(m_y2 - m_pos.getY());
                return std::abs(det / std::sqrt(double(d0)));
            } else {
                /* position is outside bounding rectangle, estimate using endpoints */
                return std::sqrt(double(std::min(pt.getSquaredRawDistance(m_pos),
                                                 pt.getSquaredRawDistance(Point(m_x2, m_y2)))));
            }
        }
     }

     case RectangleDrawing: {
        /* Rectangle. We have 9 cases:

                 1   2   3
                   +---+
                 4 | 5 | 6
                   +---+
                 7   8   9
        */
        const int distX = std::min(std::abs(pt.getX() - m_pos.getX()),
                                   std::abs(pt.getX() - m_x2));
        const int distY = std::min(std::abs(pt.getY() - m_pos.getY()),
                                   std::abs(pt.getY() - m_y2));
        const bool inRangeX = isInRange(pt.getX(), m_pos.getX(), m_x2);
        const bool inRangeY = isInRange(pt.getY(), m_pos.getY(), m_y2);

        if (inRangeX && inRangeY) {
            // Inside the rectangle (case 5). Distance is minimum to either edge.
            return std::min(distX, distY);
        } else if (inRangeX) {
            // Outside, case 2 or 8. Distance is minimum to horizontal edge.
            return distY;
        } else if (inRangeY) {
            // Outside, case 4 or 6. Distance is minimum to vertical edge.
            return distX;
        } else {
            // Outside the rectangle (cases 1, 3, 7, 9). Distance to corner, which is hypot-of-distance-to-edge.
            return util::getDistanceFromDX(distX, distY);
        }
     }

     case CircleDrawing:
        return std::abs(std::sqrt(double(m_pos.getSquaredRawDistance(pt))) - m_x2);

     case MarkerDrawing:
        return std::sqrt(double(m_pos.getSquaredRawDistance(pt)));

     default:
        return 1e6;
    }
}

// Compute distance of this drawing to a given point in the game universe, honoring wrap.
double
game::map::Drawing::getDistanceToWrap(Point pt, const Configuration& config) const
{
    // ex GDrawing::getDistanceToWrap
    double dist = getDistanceTo(pt);
    for (int i = 1, n = config.getNumRectangularImages(); i < n; ++i) {
        Point tmp;
        if (config.getPointAlias(pt, tmp, i, false)) {
            double ndist = getDistanceTo(tmp);
            if (ndist < dist) {
                dist = ndist;
            }
        }
    }
    return dist;
}
