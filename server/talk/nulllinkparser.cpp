/**
  *  \file server/talk/nulllinkparser.cpp
  *  \brief Class server::talk::NullLinkParser
  */

#include "server/talk/nulllinkparser.hpp"

afl::base::Optional<server::talk::LinkParser::Result_t>
server::talk::NullLinkParser::parseGameLink(String_t /*text*/) const
{
    return std::make_pair(1, String_t());
}

afl::base::Optional<server::talk::LinkParser::Result_t>
server::talk::NullLinkParser::parseForumLink(String_t /*text*/) const
{
    return std::make_pair(1, String_t());
}

afl::base::Optional<server::talk::LinkParser::Result_t>
server::talk::NullLinkParser::parseTopicLink(String_t /*text*/) const
{
    return std::make_pair(1, String_t());
}

afl::base::Optional<server::talk::LinkParser::Result_t>
server::talk::NullLinkParser::parseMessageLink(String_t /*text*/) const
{
    return std::make_pair(1, String_t());
}

afl::base::Optional<String_t>
server::talk::NullLinkParser::parseUserLink(String_t /*text*/) const
{
    return String_t();
}
