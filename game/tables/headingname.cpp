/**
  *  \file game/tables/headingname.cpp
  */

#include "game/tables/headingname.hpp"

game::tables::HeadingName::HeadingName()
{ }

// /** Get name for a heading (direction).
//     \param heading Heading in degrees
//     \return statically-allocated, translated string */
String_t
game::tables::HeadingName::get(int heading) const
{
    // ex game/tables.h:getHeadingName
    // FIXME: those cannot easily be translated because they're so short.
    // One way to make them translatable is to put them into a big string
    // containing all headings, and have this function return a string.
    // \change This only deals with positive values!
    static const char names[][4] = {
        "N", "NNE", "NE", "ENE",
        "E", "ESE", "SE", "SSE",
        "S", "SSW", "SW", "WSW",
        "W", "WNW", "NW", "NNW"
    };

    /* There are 16 headings in 360 degrees, each covering 22.5 degrees,
       and aligned to 11.25 degrees. We can remove the fractional digits
       easily by multiplying by four. */
    return names[((4*heading + 45) / 90) & 15];
}

bool
game::tables::HeadingName::getFirstKey(int& a) const
{
    a = 0;
    return true;
}

bool
game::tables::HeadingName::getNextKey(int& a) const
{
    // Headings cover 22.5 degrees each.
    // We can be imperfect and advance in steps of 22, which still hits each direction once.
    if (a < 15*22) {
        a += 22;
        return true;
    } else {
        return false;
    }
}
