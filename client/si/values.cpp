/**
  *  \file client/si/values.cpp
  */

#include "client/si/values.hpp"
#include "afl/string/string.hpp"

String_t
client::si::formatFrameType(ui::FrameType type)
{
    switch (type) {
     case ui::NoFrame:      return "none";
     case ui::RedFrame:     return "red";
     case ui::YellowFrame:  return "yellow";
     case ui::GreenFrame:   return "green";
     case ui::RaisedFrame:  return "raised";
     case ui::LoweredFrame: return "lowered";
    }
    return String_t();
}
    
bool
client::si::parseFrameType(ui::FrameType& type, String_t value)
{
    using afl::string::strCaseCompare;
    if (strCaseCompare(value, "no") == 0 || strCaseCompare(value, "none") == 0) {
        type = ui::NoFrame;
        return true;
    } else if (strCaseCompare(value, "red") == 0) {
        type = ui::RedFrame;
        return true;
    } else if (strCaseCompare(value, "yellow") == 0) {
        type = ui::YellowFrame;
        return true;
    } else if (strCaseCompare(value, "green") == 0) {
        type = ui::GreenFrame;
        return true;
    } else if (strCaseCompare(value, "raised") == 0) {
        type = ui::RaisedFrame;
        return true;
    } else if (strCaseCompare(value, "lowered") == 0) {
        type = ui::LoweredFrame;
        return true;
    } else {
        return false;
    }
}
