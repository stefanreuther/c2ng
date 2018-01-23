/**
  *  \file game/map/basedata.cpp
  */

#include "game/map/basedata.hpp"

game::map::BaseStorage*
game::map::getBaseStorage(BaseData& bd, TechLevel area)
{
    switch (area) {
     case HullTech:
        return &bd.hullStorage;
     case EngineTech:
        return &bd.engineStorage;
     case BeamTech:
        return &bd.beamStorage;
     case TorpedoTech:
        return &bd.launcherStorage;
    }
    return 0;
}

const game::map::BaseStorage*
game::map::getBaseStorage(const BaseData& bd, TechLevel area)
{
    return getBaseStorage(const_cast<BaseData&>(bd), area);
}
