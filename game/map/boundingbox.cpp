/**
  *  \file game/map/boundingbox.cpp
  */

#include <algorithm>
#include "game/map/boundingbox.hpp"
#include "game/map/universe.hpp"
#include "game/map/drawing.hpp"
#include "game/map/planet.hpp"
#include "game/map/objecttype.hpp"
#include "game/map/circularobject.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/drawingcontainer.hpp"

game::map::BoundingBox::BoundingBox()
    : m_min(), m_max()
{ }

void
game::map::BoundingBox::addUniverse(const Universe& univ)
{
    // ex GChartBBox::initFromUniverse (part)
    // Add known/configured size of universe
    addPoint(univ.config().getMinimumCoordinates());
    addPoint(univ.config().getMaximumCoordinates());

    // Add regular units
    {
        AnyPlanetType t(const_cast<Universe&>(univ));
        addType(t);
    }
    {
        AnyShipType t(const_cast<Universe&>(univ));
        addType(t);
    }
    addType(const_cast<Universe&>(univ).ionStormType());
    addType(const_cast<Universe&>(univ).minefields());

    // Add drawings
    for (DrawingContainer::Iterator_t it = univ.drawings().begin(), end = univ.drawings().end(); it != end; ++it) {
        if (Drawing* p = *it) {
            addDrawing(*p);
        }
    }

    // Add Ufos
    UfoType& ufos = const_cast<Universe&>(univ).ufos();
    for (Id_t i = ufos.findNextIndex(0); i != 0; i = ufos.findNextIndex(i)) {
        Ufo* ufo = ufos.getObjectByIndex(i);
        Point pt;
        int radius;
        if (ufo != 0 && ufo->getPosition(pt) && ufo->getRadius(radius)) {
            // Valid Ufo: add it
            addCircle(pt, radius);

            // If it has another end, add the connector
            Ufo* otherEnd = ufo->getOtherEnd();
            Point otherPos;
            int otherRadius;
            if (otherEnd != 0 && otherEnd->getPosition(otherPos) && otherEnd->getRadius(otherRadius)) {
                addCircle(univ.config().getSimpleNearestAlias(otherPos, pt), otherRadius);
            }
        }
    }

    // FIXME
    // initMinZoom();
    // initMaxZoom();
}

void
game::map::BoundingBox::addPoint(Point pt)
{
    // ex GChartBBox::addPoint
    // @change We use half-open intervals, hence different logic
    if (m_min == m_max) {
        // Empty -> set anew
        m_min = pt;
        m_max = pt + Point(1, 1);
    } else {
        // Nonempty
        m_min.setX(std::min(m_min.getX(), pt.getX()));
        m_min.setY(std::min(m_min.getY(), pt.getY()));
        m_max.setX(std::max(m_max.getX(), pt.getX() + 1));
        m_max.setY(std::max(m_max.getY(), pt.getY() + 1));
    }
}

void
game::map::BoundingBox::addCircle(Point pt, int radius)
{
    // ex GChartBBox::addCircle
    addPoint(pt + Point(radius, radius));
    addPoint(pt - Point(radius, radius));
}

void
game::map::BoundingBox::addDrawing(const Drawing& d)
{
    // ex GChartBBox::addDrawing
    switch (d.getType()) {
     case Drawing::LineDrawing:
     case Drawing::RectangleDrawing:
        // Line/rectangle: include endpoints
        addPoint(d.getPos());
        addPoint(d.getPos2());
        break;

     case Drawing::CircleDrawing:
        // Circle
        addCircle(d.getPos(), d.getCircleRadius());
        break;

     case Drawing::MarkerDrawing:
        // Marker.
        // We assume that all markers have a radius of 10 or less.
        // This does not consider the optional text of the marker, though.
        addCircle(d.getPos(), 10);
        break;
    }
}

game::map::Point
game::map::BoundingBox::getMinimumCoordinates()
{
    // ex GChartBBox::getMinimumXY
    return m_min;
}

game::map::Point
game::map::BoundingBox::getMaximumCoordinates()
{
    // ex GChartBBox::getMaximumXY
    return m_max;
}

void
game::map::BoundingBox::addType(const ObjectType& ty)
{
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        if (const MapObject* obj = dynamic_cast<const MapObject*>(const_cast<ObjectType&>(ty).getObjectByIndex(i))) {
            Point pos;
            if (obj->getPosition(pos)) {
                if (const CircularObject* circ = dynamic_cast<const CircularObject*>(obj)) {
                    int r;
                    if (circ->getRadius(r)) {
                        addCircle(pos, r);
                    }
                } else {
                    addPoint(pos);
                }
            }
        }
    }
}

// FIXME: port? initMinZoom, initMaxZoom
// // enum {
// //     MAX_ZOOM = 10
// // };
// /** Compute minimum zoom level. The minimum zoom is a soft limit;
//     below the limit, the scrolling map is replaced by a fixed one. */
// void
// GChartBBox::initMinZoom()
// {
//     /* Minimum zoom. At zoom smaller 1:N, switch to overview chart */
//     min_zoom = 2;
//     // FIXME: dummy
//     // CheckMinZoom(SCREEN_WIDTH DIV 2, ChartBX1-ChartConf.CenterX);
//     // CheckMinZoom(SCREEN_WIDTH DIV 2, ChartConf.CenterX-ChartBX0);
//     // CheckMinZoom((SCREEN_HEIGHT-80) DIV 2, ChartBY1-ChartConf.CenterY);
//     // CheckMinZoom((SCREEN_HEIGHT-80) DIV 2, ChartConf.CenterY-ChartBY0);
//     if (min_zoom > MAX_ZOOM)
//         min_zoom = MAX_ZOOM;
// }

// /** Compute maximum zoom level. The maximum zoom level is a hard
//     limit; under no conditions the hard limit may be exceeded to avoid
//     integer overflow. */
// void
// GChartBBox::initMaxZoom()
// {
//     max_zoom = INT_MAX / std::max(max.x, max.y);
//     if (max_zoom > MAX_ZOOM)
//         max_zoom = MAX_ZOOM;
// }

// /** Get minimum allowed zoom level. This is a soft limit, and decides at what
//     point we switch from a scrollable starchart to a fixed one. */
// inline int
// GChartBBox::getMinZoom() const
// {
//     return min_zoom;
// }

// /** Get maximum allowed zoom level. This is the maximum zoom level at which
//     there is no overflow in coordinates. */
// inline int
// GChartBBox::getMaxZoom() const
// {
//     return max_zoom;
// }
