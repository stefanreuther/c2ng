/**
  *  \file ui/res/resid.cpp
  *  \brief Resource Identifiers in PCC2ng
  */

#include "ui/res/resid.hpp"
#include "afl/string/format.hpp"
#include "util/stringparser.hpp"

const char ui::res::SHIP[] = "ship";
const char ui::res::PLANET[] = "planet";
const char ui::res::BASE[] = "base";
const char ui::res::RSHIP[] = "rship", ui::res::LSHIP[] = "lship";
const char ui::res::VCR_FIGHTER[] = "vcr.ftr";

// Make resource Id from prefix and one integer.
String_t
ui::res::makeResourceId(const char* prefix, int a)
{
    return afl::string::Format("%s.%d", prefix, a);
}

// Make resource Id from prefix and two integers.
String_t
ui::res::makeResourceId(const char* prefix, int a, int b)
{
    return afl::string::Format("%s.%d.%d", prefix, a, b);
}

// Generalize resource Id.
bool
ui::res::generalizeResourceId(String_t& s)
{
    // ex RedId::generalize, sort-of
    String_t::size_type pos = s.rfind('.');
    if (pos != String_t::npos) {
        s.erase(pos);
        return true;
    } else {
        return false;
    }
}

// Match resource Id to prefix and one integer.
bool
ui::res::matchResourceId(const String_t& resId, const char* prefix, int& a)
{
    util::StringParser p(resId);
    return p.parseString(prefix)
        && p.parseString(".")
        && p.parseInt(a)
        && p.parseEnd();
}

// Match resource Id to prefix and two integers.
bool
ui::res::matchResourceId(const String_t& resId, const char* prefix, int& a, int& b)
{
    util::StringParser p(resId);
    return p.parseString(prefix)
        && p.parseString(".")
        && p.parseInt(a)
        && p.parseString(".")
        && p.parseInt(b)
        && p.parseEnd();
}
