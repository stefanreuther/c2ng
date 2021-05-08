/**
  *  \file game/vcr/flak/position.cpp
  *  \brief Class game::vcr::flak::Position
  */

#include <cmath>
#include <cstdlib>
#include "game/vcr/flak/position.hpp"

double
game::vcr::flak::Position::distanceTo(const Position& other) const
{
    // ex FlakPos::distanceTo, flak.pas:DistanceFleetToFleet, flak.pas:DistanceFleetToXY, flak.pas:DistanceFleetToShip
    // FIXME: PCC1 has optimisations for same y's here
    // These must be double, we're computing stuff like 200000^2 here
    const double dist_x = x - other.x;
    const double dist_y = y - other.y;
    return std::sqrt(dist_x*dist_x + dist_y*dist_y);
}

bool
game::vcr::flak::Position::isDistanceLERadius(const Position& other, int32_t radius) const
{
    // ex FlakPos::isDistanceLERadius, flak.pas:IsDistanceLERadius
    const int32_t dist_x = std::abs(x - other.x);
    const int32_t dist_y = std::abs(y - other.y);

    return dist_x <= radius
        && dist_y <= radius
        && dist_x*dist_x + dist_y*dist_y <= radius*radius;
}
