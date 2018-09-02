/**
  *  \file server/interface/hostfile.cpp
  *  \brief Interface server::interface::HostFile
  */

#include "server/interface/hostfile.hpp"

namespace {
    template<typename T>
    void merge(afl::base::Optional<T>& out, const afl::base::Optional<T>& in)
    {
        if (!out.isValid()) {
            out = in;
        }
    }
}

// Format Label to string.
String_t
server::interface::HostFile::formatLabel(Label label)
{
    String_t result;
    switch (label) {
     case NameLabel:    result = "name";    break;
     case TurnLabel:    result = "turn";    break;
     case SlotLabel:    result = "slot";    break;
     case GameLabel:    result = "game";    break;
     case ToolLabel:    result = "tool";    break;
     case NoLabel:      result = "none";    break;
     case HistoryLabel: result = "history"; break;
    }
    return result;
}

// Parse string into Label.
bool
server::interface::HostFile::parseLabel(const String_t& str, Label& result)
{
    if (str == "name") {
        result = NameLabel;
        return true;
    } else if (str == "turn") {
        result = TurnLabel;
        return true;
    } else if (str == "slot") {
        result = SlotLabel;
        return true;
    } else if (str == "game") {
        result = GameLabel;
        return true;
    } else if (str == "tool") {
        result = ToolLabel;
        return true;
    } else if (str == "none") {
        result = NoLabel;
        return true;
    } else if (str == "history") {
        result = HistoryLabel;
        return true;
    } else {
        return false;
    }
}

// Merge information.
void
server::interface::HostFile::mergeInfo(Info& i, const Info& parent)
{
    merge(i.turnNumber, parent.turnNumber);
    merge(i.slotId, parent.slotId);
    merge(i.slotName, parent.slotName);
    merge(i.gameId, parent.gameId);
    merge(i.gameName, parent.gameName);
    merge(i.toolName, parent.toolName);
}
