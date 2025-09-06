/**
  *  \file server/talk/nulllinkparser.hpp
  *  \brief Class server::talk::NullLinkParser
  */
#ifndef C2NG_SERVER_TALK_NULLLINKPARSER_HPP
#define C2NG_SERVER_TALK_NULLLINKPARSER_HPP

#include "server/talk/linkparser.hpp"

namespace server { namespace talk {

    /** Null link parser interface.
        This interface reports all links to be valid.
        It can be used to parse a message without verifying links and generating warnings. */
    class NullLinkParser : public LinkParser {
     public:
        virtual afl::base::Optional<Result_t> parseGameLink(String_t text) const;
        virtual afl::base::Optional<Result_t> parseForumLink(String_t text) const;
        virtual afl::base::Optional<Result_t> parseTopicLink(String_t text) const;
        virtual afl::base::Optional<Result_t> parseMessageLink(String_t text) const;
        virtual afl::base::Optional<String_t> parseUserLink(String_t text) const;
    };

} }

#endif
