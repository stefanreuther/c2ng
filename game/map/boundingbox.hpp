/**
  *  \file game/map/boundingbox.hpp
  *  \brief Class game::map::BoundingBox
  */
#ifndef C2NG_GAME_MAP_BOUNDINGBOX_HPP
#define C2NG_GAME_MAP_BOUNDINGBOX_HPP

#include "game/map/point.hpp"

namespace game { namespace map {

    class Configuration;
    class Drawing;
    class Universe;
    class ObjectType;

    /** Map bounding box.
        A bounding box is represented as a half-open interval,
        i.e. the numerical minimum coordinate (south-west) is included,
        the numerical maximum coordinate (north-east) is not.

        An empty bounding box (no object added) is represented by getMinimumCoordinates() == getMaximumCoordinates(). */
    class BoundingBox {
     public:
        /** Default constructor.
            Makes an empty bounding box. */
        BoundingBox();

        /** Add a universe.
            Adds all objects from the universe.
            \param univ Universe */
        void addUniverse(const Universe& univ, const Configuration& mapConfig);

        /** Add a point object.
            \param pt Point */
        void addPoint(Point pt);

        /** Add a circular object.
            \param pt Center
            \param radius Radius */
        void addCircle(Point pt, int radius);

        /** Add a drawing.
            \param d Drawing */
        void addDrawing(const Drawing& d);

        /** Get minimum coordinates (inclusive).
            \return coordinates */
        Point getMinimumCoordinates() const;

        /** Get maximum coordinates (exclusive).
            \return coordinates */
        Point getMaximumCoordinates() const;

     private:
        Point m_min;
        Point m_max;

        void addType(const ObjectType& ty);
    };

} }

#endif
