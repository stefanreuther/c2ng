/**
  *  \file util/skincolor.cpp
  *  \brief Enum util::SkinColor
  */

#include "util/skincolor.hpp"

const size_t util::SkinColor::NUM_COLORS;

bool
util::SkinColor::parse(const String_t& name, Color& result)
{
    struct ColorName {
        const char* name;
        Color color;
    };
    static const ColorName styles[] = {
        { "static",            Static },
        { "green",             Green },
        { "yellow",            Yellow },
        { "red",               Red },
        { "white",             White },
        { "contrast-color",    Contrast },
        { "input-color",       Input },
        { "blue",              Blue },
        { "dim",               Faded },
        { "heading-color",     Heading },
        { "selection-color",   Selection },
        { "inverse-color",     InvStatic },
        { "background-color",  Background },
        { "link-color",        Link },
        { "link-shade-color",  LinkShade },
        { "link-focus-color",  LinkFocus },
    };
    for (size_t i = 0; i < sizeof(styles)/sizeof(styles[0]); ++i) {
        if (name == styles[i].name) {
            result = styles[i].color;
            return true;
        }
    }
    return false;
}
