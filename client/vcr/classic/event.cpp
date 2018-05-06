/**
  *  \file client/vcr/classic/event.cpp
  */

#include "client/vcr/classic/event.hpp"

const char*
client::vcr::classic::Event::toString(Type t)
{
    const char* result = "?";
    switch (t) {
     case UpdateTime:             result = "UpdateTime";             break;
     case UpdateDistance:         result = "UpdateDistance";         break;
     case MoveObject:             result = "MoveObject";             break;
     case StartFighter:           result = "StartFighter";           break;
     case RemoveFighter:          result = "RemoveFighter";          break;
     case UpdateNumFighters:      result = "UpdateNumFighters";      break;
     case MoveFighter:            result = "MoveFighter";            break;
     case UpdateFighter:          result = "UpdateFighter";          break;
     case ExplodeFighter:         result = "ExplodeFighter";         break;
     case FireBeamShipFighter:    result = "FireBeamShipFighter";    break;
     case FireBeamShipShip:       result = "FireBeamShipShip";       break;
     case FireBeamFighterShip:    result = "FireBeamFighterShip";    break;
     case FireBeamFighterFighter: result = "FireBeamFighterFighter"; break;
     case BlockBeam:              result = "BlockBeam";              break;
     case UnblockBeam:            result = "UnblockBeam";            break;
     case UpdateBeam:             result = "UpdateBeam";             break;
     case FireTorpedo:            result = "FireTorpedo";            break;
     case UpdateNumTorpedoes:     result = "UpdateNumTorpedoes";     break;
     case BlockLauncher:          result = "BlockLauncher";          break;
     case UnblockLauncher:        result = "UnblockLauncher";        break;
     case UpdateLauncher:         result = "UpdateLauncher";         break;
     case UpdateObject:           result = "UpdateObject";           break;
     case UpdateAmmo:             result = "UpdateAmmo";             break;
     case HitObject:              result = "HitObject";              break;
     case SetResult:              result = "SetResult";              break;
     case WaitTick:               result = "WaitTick";               break;
     case WaitAnimation:          result = "WaitAnimation";          break;
    }
    return result;
}
