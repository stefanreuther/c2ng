/**
  *  \file server/talk/linkformatter.hpp
  */
#ifndef C2NG_SERVER_TALK_LINKFORMATTER_HPP
#define C2NG_SERVER_TALK_LINKFORMATTER_HPP

#include "afl/string/string.hpp"
#include "afl/base/types.hpp"

namespace server { namespace talk {

    // FIXME: turn this into an abstract base class with virtual functions
    class LinkFormatter {
     public:
        String_t makeGameUrl(int32_t gameId, String_t gameName);
        String_t makeForumUrl(int32_t forumId, String_t forumName);
        String_t makePostUrl(int32_t topicId, String_t subject, int32_t messageId);
        String_t makeTopicUrl(int32_t topicId, String_t subject);
        String_t makeUserUrl(String_t userId);
    };

} }

#endif
