/**
  *  \file game/element.cpp
  */

#include "game/element.hpp"

game::Element::Type
game::Element::fromTorpedoType(int torpedoType)
{
    return Type(FirstTorpedo + (torpedoType - 1));
}

bool
game::Element::isTorpedoType(Type t, int& torpedoType)
{
    if (t >= FirstTorpedo) {
        torpedoType = (t - FirstTorpedo) + 1;
        return true;
    } else {
        return false;
    }
}

game::Element::Type&
game::operator++(Element::Type& t)
{
    t = static_cast<Element::Type>(t + 1);
    return t;
}

game::Element::Type&
game::operator--(Element::Type& t)
{
    t = static_cast<Element::Type>(t - 1);
    return t;
}

game::Element::Type
game::operator++(Element::Type& t, int)
{
    Element::Type result = t;
    ++t;
    return result;
}

game::Element::Type
game::operator--(Element::Type& t, int)
{
    Element::Type result = t;
    --t;
    return result;
}
