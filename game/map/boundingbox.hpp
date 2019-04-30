/**
  *  \file game/map/boundingbox.hpp
  */
#ifndef C2NG_GAME_MAP_BOUNDINGBOX_HPP
#define C2NG_GAME_MAP_BOUNDINGBOX_HPP

#include "game/map/point.hpp"

namespace game { namespace map {

    class Drawing;
    class Universe;
    class ObjectType;

    class BoundingBox {
     public:
        BoundingBox();

        void addUniverse(const Universe& univ);

        void addPoint(Point pt);
        void addCircle(Point pt, int radius);
        void addDrawing(const Drawing& d);

        Point getMinimumCoordinates();
        Point getMaximumCoordinates();

     private:
        Point m_min;
        Point m_max;

        void addType(const ObjectType& ty);
    };

} }

#endif
