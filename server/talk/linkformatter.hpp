/**
  *  \file server/talk/linkformatter.hpp
  *  \brief Class server::talk::LinkFormatter
  */
#ifndef C2NG_SERVER_TALK_LINKFORMATTER_HPP
#define C2NG_SERVER_TALK_LINKFORMATTER_HPP

#include "afl/string/string.hpp"
#include "afl/base/types.hpp"

namespace server { namespace talk {

    /** Generating links for HTML rendering.

        For now, this is a concrete class.
        It might be turned into an interface later. */
    class LinkFormatter {
     public:
        /** Make link for game.
            @param gameId Game Id
            @param gameName Game name
            @return link, not including site-root */
        String_t makeGameUrl(int32_t gameId, String_t gameName);

        /** Make link for forum.
            @param forumId Forum Id
            @param forumName Forum name
            @return link, not including site-root */
        String_t makeForumUrl(int32_t forumId, String_t forumName);

        /** Make link for post.
            @param topicId Topic (thread) Id
            @param subject Topic subject
            @param messageId Message (post) Id
            @return link, not including site-root */
        String_t makePostUrl(int32_t topicId, String_t subject, int32_t messageId);

        /** Make link for topic (thread).
            @param topicId Topic (thread) Id
            @param subject Topic subject
            @return link, not including site-root */
        String_t makeTopicUrl(int32_t topicId, String_t subject);

        /** Make link for user profile.
            @param userId User (User::getLoginName())
            @return link, not including site-root */
        String_t makeUserUrl(String_t userId);
    };

} }

#endif
