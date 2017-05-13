/**
  *  \file server/interface/talkforum.cpp
  */

#include <memory>
#include "server/interface/talkforum.hpp"
#include "server/types.hpp"

int32_t
server::interface::TalkForum::getIntegerValue(int32_t fid, String_t keyName)
{
    std::auto_ptr<afl::data::Value> p(getValue(fid, keyName));
    return toInteger(p.get());
}

String_t
server::interface::TalkForum::getStringValue(int32_t fid, String_t keyName)
{
    std::auto_ptr<afl::data::Value> p(getValue(fid, keyName));
    return toString(p.get());
}
