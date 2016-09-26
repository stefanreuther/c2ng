/**
  *  \file game/point.hpp
  */
#ifndef C2NG_GAME_POINT_HPP
#define C2NG_GAME_POINT_HPP

#include "afl/string/string.hpp"

namespace game { namespace map {

    class Point {
     public:
        Point();
        Point(int x, int y);

        int getX() const;
        int getY() const;
        void setX(int x);
        void setY(int y);
        void addX(int dx);
        void addY(int dy);

        // --> in Configuration
        // bool    isOnMap() const;
        // GPoint  getCanonicalLocation() const;
        // GPoint  getSimpleCanonicalLocation() const;
        // GPoint  getSimpleNearestAlias(GPoint a) const;

        bool operator==(Point rhs) const;
        bool operator!=(Point rhs) const;
        Point operator+(Point other) const;
        Point operator-(Point other) const;
        Point& operator+=(Point other);
        Point& operator-=(Point other);

        bool parseCoordinates(const String_t& s);

        // --> in Configuration
        // bool parseSectorNumber(const string_t& s);
        // bool parseSectorNumber(int n);
        // int  getSectorNumber() const;

        long getSquaredRawDistance(Point other) const;
        bool isCloserThan(Point other, long distance) const;

     private:
        int m_x;
        int m_y;
    };

} }


#endif
