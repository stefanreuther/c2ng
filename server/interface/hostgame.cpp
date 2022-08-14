/**
  *  \file server/interface/hostgame.cpp
  *  \brief Interface server::interface::HostGame
  */

#include "server/interface/hostgame.hpp"

/***************************** HostGame::Info ****************************/

server::interface::HostGame::Info::Info()
    : gameId(0),
      state(Preparing),
      type(PrivateGame),
      name(),
      description(),
      difficulty(0),
      currentSchedule(),
      slotStates(),
      turnStates(),
      joinable(),
      userPlays(),
      scores(),
      scoreName(),
      scoreDescription(),
      hostName(),
      hostDescription(),
      hostKind(),
      shipListName(),
      shipListDescription(),
      shipListKind(),
      masterName(),
      masterDescription(),
      turnNumber(0),
      lastHostTime(),
      nextHostTime(),
      forumId(),
      userRank(),
      otherRank()
{ }

server::interface::HostGame::Info::~Info()
{ }

/*********************** HostGame::VictoryCondition **********************/

server::interface::HostGame::VictoryCondition::VictoryCondition()
    : endCondition(),
      endTurn(),
      endProbability(),
      endScore(),
      endScoreName(),
      endScoreDescription(),
      referee(),
      refereeDescription()
{ }

server::interface::HostGame::VictoryCondition::~VictoryCondition()
{ }

/******************************** HostGame *******************************/

String_t
server::interface::HostGame::formatState(State state)
{
    switch (state) {
     case Preparing: return "preparing";
     case Joining:   return "joining";
     case Running:   return "running";
     case Finished:  return "finished";
     case Deleted:   return "deleted";
    }
    return String_t();
}

bool
server::interface::HostGame::parseState(const String_t& str, State& result)
{
    if (str == "preparing") {
        result = Preparing;
        return true;
    } else if (str == "joining") {
        result = Joining;
        return true;
    } else if (str == "running") {
        result = Running;
        return true;
    } else if (str == "finished") {
        result = Finished;
        return true;
    } else if (str == "deleted") {
        result = Deleted;
        return true;
    } else {
        return false;
    }
}

String_t
server::interface::HostGame::formatType(Type type)
{
    switch (type) {
     case PrivateGame:  return "private";
     case UnlistedGame: return "unlisted";
     case PublicGame:   return "public";
     case TestGame:     return "test";
    }
    return String_t();
}

bool
server::interface::HostGame::parseType(const String_t& str, Type& result)
{
    if (str == "private") {
        result = PrivateGame;
        return true;
    } else if (str == "unlisted") {
        result = UnlistedGame;
        return true;
    } else if (str == "public") {
        result = PublicGame;
        return true;
    } else if (str == "test") {
        result = TestGame;
        return true;
    } else {
        return false;
    }
}

String_t
server::interface::HostGame::formatSlotState(SlotState state)
{
    switch (state) {
     case OpenSlot: return "open";
     case SelfSlot: return "self";
     case OccupiedSlot: return "occupied";
     case DeadSlot: return "dead";
    }
    return String_t();
}

bool
server::interface::HostGame::parseSlotState(const String_t& str, SlotState& result)
{
    if (str == "open") {
        result = OpenSlot;
        return true;
    } else if (str == "self") {
        result = SelfSlot;
        return true;
    } else if (str == "occupied") {
        result = OccupiedSlot;
        return true;
    } else if (str == "dead") {
        result = DeadSlot;
        return true;
    } else {
        return false;
    }
}
