/**
  *  \file client/si/values.cpp
  */

#include "client/si/values.hpp"
#include "afl/string/string.hpp"

String_t
client::si::formatFrameType(ui::widgets::FrameGroup::Type type)
{
    using ui::widgets::FrameGroup;
    switch (type) {
     case FrameGroup::NoFrame:      return "none";
     case FrameGroup::RedFrame:     return "red";
     case FrameGroup::YellowFrame:  return "yellow";
     case FrameGroup::GreenFrame:   return "green";
     case FrameGroup::RaisedFrame:  return "raised";
     case FrameGroup::LoweredFrame: return "lowered";
    }
    return String_t();
}
    
bool
client::si::parseFrameType(ui::widgets::FrameGroup::Type& type, String_t value)
{
    using afl::string::strCaseCompare;
    using ui::widgets::FrameGroup;
    if (strCaseCompare(value, "no") == 0 || strCaseCompare(value, "none") == 0) {
        type = FrameGroup::NoFrame;
        return true;
    } else if (strCaseCompare(value, "red") == 0) {
        type = FrameGroup::RedFrame;
        return true;
    } else if (strCaseCompare(value, "yellow") == 0) {
        type = FrameGroup::YellowFrame;
        return true;
    } else if (strCaseCompare(value, "green") == 0) {
        type = FrameGroup::GreenFrame;
        return true;
    } else if (strCaseCompare(value, "raised") == 0) {
        type = FrameGroup::RaisedFrame;
        return true;
    } else if (strCaseCompare(value, "lowered") == 0) {
        type = FrameGroup::LoweredFrame;
        return true;
    } else {
        return false;
    }
}
