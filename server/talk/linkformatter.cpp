/**
  *  \file server/talk/linkformatter.cpp
  */

#include "server/talk/linkformatter.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"

namespace {
    const char GAME_BASE_URL[] = "host/game.cgi/";
    const char USER_BASE_URL[] = "userinfo.cgi/";
    const char FORUM_BASE_URL[] = "talk/forum.cgi/";
    const char THREAD_BASE_URL[] = "talk/thread.cgi/";

    // /** Simplify a topic name for inclusion in an URL. */
    String_t simplifyTopic(String_t s)
    {
        // ex planetscentral/talk/url.h:simplifyTopic
        String_t result;
        bool needsp = false;
        for (size_t i = 0; i < s.size(); ++i) {
            char ch = s[i];
            if (afl::string::charIsAlphanumeric(ch)) {
                if (needsp) {
                    result += '-';
                }
                result += ch;
                needsp = false;
            } else {
                needsp = (result.size() != 0);
            }
        }
        return result;
    }
}

String_t
server::talk::LinkFormatter::makeGameUrl(int32_t gameId, String_t gameName)
{
    // ex planetscentral/talk/url.h:makeGameUrl
    return GAME_BASE_URL + simplifyTopic(afl::string::Format("%d-%s", gameId, gameName));
}

String_t
server::talk::LinkFormatter::makeForumUrl(int32_t forumId, String_t forumName)
{
    // ex planetscentral/talk/url.h:makeForumUrl
    return FORUM_BASE_URL + simplifyTopic(afl::string::Format("%d-%s", forumId, forumName));

}

String_t
server::talk::LinkFormatter::makePostUrl(int32_t topicId, String_t subject, int32_t messageId)
{
    // ex planetscentral/talk/url.h:makePostUrl
    return THREAD_BASE_URL + String_t(afl::string::Format("%s#p%d", simplifyTopic(afl::string::Format("%d-%s", topicId, subject)), messageId));
}

String_t
server::talk::LinkFormatter::makeTopicUrl(int32_t topicId, String_t subject)
{
    // ex planetscentral/talk/url.h:makeTopicUrl
    return THREAD_BASE_URL + simplifyTopic(afl::string::Format("%d-%s", topicId, subject));
}

String_t
server::talk::LinkFormatter::makeUserUrl(String_t userId)
{
    return USER_BASE_URL + userId;
}
