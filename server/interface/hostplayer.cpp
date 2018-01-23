/**
  *  \file server/interface/hostplayer.cpp
  */

#include "server/interface/hostplayer.hpp"

server::interface::HostPlayer::Info::Info()
    : longName(),
      shortName(),
      adjectiveName(),
      userIds(),
      numEditable(0),
      joinable(false)
{ }

String_t
server::interface::HostPlayer::formatFileStatus(FileStatus st)
{
    switch (st) {
     case Stale:  return "stale";
     case Allow:  return "allow";
     case Turn:   return "trn";
     case Refuse: return "refuse";
    }
    return String_t();
}

bool
server::interface::HostPlayer::parseFileStatus(const String_t& str, FileStatus& st)
{
    if (str == "stale") {
        st = Stale;
        return true;
    } else if (str == "allow") {
        st = Allow;
        return true;
    } else if (str == "trn") {
        st = Turn;
        return true;
    } else if (str == "refuse") {
        st = Refuse;
        return true;
    } else {
        return false;
    }
}
