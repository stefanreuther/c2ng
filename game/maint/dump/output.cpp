/**
  *  \file game/maint/dump/output.cpp
  *  \brief Class game::maint::dump::Output
  */

#include "game/maint/dump/output.hpp"

void
game::maint::dump::Output::addField(const char* name, String_t value)
{
    // DumpOutputReceiver::addField(const char* name, string_t value)
    addField(String_t(name), value);
}
