/**
  *  \file server/interface/sessionrouter.cpp
  */

#include "server/interface/sessionrouter.hpp"

String_t
server::interface::SessionRouter::formatAction(Action action)
{
    switch (action) {
     case Close:   return "CLOSE";
     case Restart: return "RESTART";
     case Save:    return "SAVE";
     case SaveNN:  return "SAVENN";
    }
    return String_t();
}

bool
server::interface::SessionRouter::parseAction(const String_t& str, Action& result)
{
    using afl::string::strCaseCompare;
    if (strCaseCompare(str, "CLOSE") == 0) {
        result = Close;
        return true;
    } else if (strCaseCompare(str, "RESTART") == 0) {
        result = Restart;
        return true;
    } else if (strCaseCompare(str, "SAVE") == 0) {
        result = Save;
        return true;
    } else if (strCaseCompare(str, "SAVENN") == 0) {
        result = SaveNN;
        return true;
    } else {
        return false;
    }
}
