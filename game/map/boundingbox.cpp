/**
  *  \file game/map/boundingbox.cpp
  *  \brief Class game::map::BoundingBox
  */

#include <algorithm>
#include "game/map/boundingbox.hpp"
#include "game/map/configuration.hpp"
#include "game/map/circularobject.hpp"
#include "game/map/drawing.hpp"
#include "game/map/drawingcontainer.hpp"
#include "game/map/objecttype.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"

game::map::BoundingBox::BoundingBox()
    : m_min(), m_max()
{ }

void
game::map::BoundingBox::addUniverse(const Universe& univ, const Configuration& mapConfig)
{
    // ex GChartBBox::initFromUniverse (part), chart.pas:CalcBBox (part)
    // Add known/configured size of universe
    addPoint(mapConfig.getMinimumCoordinates());
    addPoint(mapConfig.getMaximumCoordinates());

    // Add regular units
    addType(univ.allPlanets());
    addType(univ.allShips());
    addType(univ.ionStormType());
    addType(univ.minefields());
    addType(univ.explosions());

    // Add drawings
    for (DrawingContainer::Iterator_t it = univ.drawings().begin(), end = univ.drawings().end(); it != end; ++it) {
        if (Drawing* p = *it) {
            addDrawing(*p);
        }
    }

    // Add Ufos
    // These are special because the connectors may pass a wrap seam.
    // FIXME: check whether we need this: if both Ufos draw the connector, we may not need it
    UfoType& ufos = const_cast<Universe&>(univ).ufos();
    for (Id_t i = ufos.findNextIndex(0); i != 0; i = ufos.findNextIndex(i)) {
        const Ufo* ufo = ufos.getObjectByIndex(i);
        Point pt;
        int radius;
        if (ufo != 0 && ufo->getPosition().get(pt) && ufo->getRadius().get(radius)) {
            // Valid Ufo: add it
            addCircle(pt, radius);

            // If it has another end, add the connector
            const Ufo* otherEnd = ufo->getOtherEnd();
            Point otherPos;
            int otherRadius;
            if (otherEnd != 0 && otherEnd->getPosition().get(otherPos) && otherEnd->getRadius().get(otherRadius)) {
                addCircle(mapConfig.getSimpleNearestAlias(otherPos, pt), otherRadius);
            }
        }
    }
}

void
game::map::BoundingBox::addPoint(Point pt)
{
    // ex GChartBBox::addPoint, chart.pas:AddBBoxPoint
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
    // ex GChartBBox::addCircle, chart.pas:AddBBoxCircle
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
game::map::BoundingBox::getMinimumCoordinates() const
{
    // ex GChartBBox::getMinimumXY
    return m_min;
}

game::map::Point
game::map::BoundingBox::getMaximumCoordinates() const
{
    // ex GChartBBox::getMaximumXY
    return m_max;
}

void
game::map::BoundingBox::addType(const ObjectType& ty)
{
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        if (const Object* obj = const_cast<ObjectType&>(ty).getObjectByIndex(i)) {
            Point pos;
            if (obj->getPosition().get(pos)) {
                if (const CircularObject* circ = dynamic_cast<const CircularObject*>(obj)) {
                    int r;
                    if (circ->getRadius().get(r)) {
                        addCircle(pos, r);
                    }
                } else {
                    addPoint(pos);
                }
            }
        }
    }
}
