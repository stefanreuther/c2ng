/**
  *  \file game/interface/ufomethod.cpp
  */

#include "game/interface/ufomethod.hpp"
#include "game/interface/objectcommand.hpp"

void
game::interface::callUfoMethod(game::map::Ufo& ufo, UfoMethod ium, interpreter::Arguments& args)
{
    switch (ium) {
     case iumMark:
        IFObjMark(ufo, args);
        break;
     case iumUnmark:
        IFObjUnmark(ufo, args);
        break;
    }
}
