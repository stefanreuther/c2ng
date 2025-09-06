/**
  *  \file server/talk/linkparser.hpp
  *  \brief Interface server::talk::LinkParser
  */
#ifndef C2NG_SERVER_TALK_LINKPARSER_HPP
#define C2NG_SERVER_TALK_LINKPARSER_HPP

#include <utility>
#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"

namespace server { namespace talk {

    /** Link parser interface. */
    class LinkParser {
     public:
        /** Result.
            - first: object Id
            - second: object name */
        typedef std::pair<int32_t, String_t> Result_t;

        /** Virtual destructor. */
        virtual ~LinkParser()
            { }

        /** Parse game link.
            @param text Text ("42")
            @return game Id/name on success; nothing on error */
        virtual afl::base::Optional<Result_t> parseGameLink(String_t text) const = 0;

        /** Parse forum link.
            @param text Text ("42")
            @return forum Id/name on success; nothing on error */
        virtual afl::base::Optional<Result_t> parseForumLink(String_t text) const = 0;

        /** Parse topic (thread) link.
            @param text Text ("42")
            @return topic Id/subject on success; nothing on error */
        virtual afl::base::Optional<Result_t> parseTopicLink(String_t text) const = 0;

        /** Parse message (post) link.
            @param text Text ("42")
            @return message Id/subject on success; nothing on error */
        virtual afl::base::Optional<Result_t> parseMessageLink(String_t text) const = 0;

        /** Parse user link.
            @param text User login name ("fred")
            @return user Id ("1001") on success; nothing on error */
        virtual afl::base::Optional<String_t> parseUserLink(String_t text) const = 0;
    };

} }

#endif
