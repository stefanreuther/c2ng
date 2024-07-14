/**
  *  \file server/talk/root.cpp
  *  \brief Class server::talk::Root
  */

#include <memory>
#include "server/talk/root.hpp"
#include "server/talk/user.hpp"
#include "server/types.hpp"
#include "afl/net/redis/field.hpp"
#include "afl/sys/time.hpp"

namespace {
    /* Database root nodes.
       This is the only place containing DB root nodes.
       All other modules only use nodes derived from (and therefore below) those. */
    const char MESSAGE_ROOT[] =        "msg:";
    const char RFC_MSGID_ROOT[] =      "msgid:";
    const char THREAD_ROOT[] =         "thread:";
    const char FORUM_ROOT[] =          "forum:";
    const char GROUP_ROOT[] =          "group:";
    const char EMAIL_ROOT[] =          "email:";
    const char PM_ROOT[] =             "pm:";
    const char DEFAULT_FOLDER_ROOT[] = "default:folder:";
}

// Constructor.
server::talk::Root::Root(afl::net::CommandHandler& db, afl::net::CommandHandler& mail, const Configuration& config)
    : server::common::Root(db),
      m_log(),
      m_keywordTable(),
      m_recognizer(),
      m_linkFormatter(),
      m_db(db),
      m_mailQueue(mail),
      m_config(config)
{ }

// Destructor.
server::talk::Root::~Root()
{ }

// Access logger.
afl::sys::Log&
server::talk::Root::log()
{
    return m_log;
}

// Access keyword table.
util::syntax::KeywordTable&
server::talk::Root::keywordTable()
{
    return m_keywordTable;
}

// Access inline-markup recognizer.
server::talk::InlineRecognizer&
server::talk::Root::recognizer()
{
    return m_recognizer;
}

// Access link formatter.
server::talk::LinkFormatter&
server::talk::Root::linkFormatter()
{
    return m_linkFormatter;
}

// Access configuration.
const server::talk::Configuration&
server::talk::Root::config() const
{
    return m_config;
}

// Access mail queue service.
server::interface::MailQueue&
server::talk::Root::mailQueue()
{
    return m_mailQueue;
}

// Get current time.
server::Time_t
server::talk::Root::getTime()
{
    /* There is no need to configure a time scale as for c2host, as we're not going to configure this to run faster somehow;
       c2talk has no (important) dependencies on wall-clock times.
       But, as for c2host, let's store minutes only.
       This makes the values appearing on the network interface the same as for c2host, and will survive 2038 :-) */

    // ex planetscentral/talk/forum.h:getTime
    return packTime(afl::sys::Time::getCurrentTime());
}


afl::net::redis::Subtree
server::talk::Root::groupRoot()
{
    return afl::net::redis::Subtree(m_db, GROUP_ROOT);
}

afl::net::redis::Subtree
server::talk::Root::messageRoot()
{
    return afl::net::redis::Subtree(m_db, MESSAGE_ROOT);
}

afl::net::redis::IntegerKey
server::talk::Root::lastMessageId()
{
    // ex Message::allocateMessage (sort-of)
    // ex Message::getLastId
    return messageRoot().intKey("id");
}

afl::net::redis::Subtree
server::talk::Root::topicRoot()
{
    return afl::net::redis::Subtree(m_db, THREAD_ROOT);
}

afl::net::redis::IntegerKey
server::talk::Root::lastTopicId()
{
    // ex Topic::allocateTopic (sort-of)
    return topicRoot().intKey("id");
}

afl::net::redis::Subtree
server::talk::Root::forumRoot()
{
    return afl::net::redis::Subtree(m_db, FORUM_ROOT);
}

afl::net::redis::IntegerKey
server::talk::Root::lastForumId()
{
    // ex Forum::allocateForum (sort-of)
    return forumRoot().intKey("id");
}

afl::net::redis::IntegerSetKey
server::talk::Root::allForums()
{
    // ex Forum::exists (sort-of)
    return forumRoot().intSetKey("all");
}

afl::net::redis::HashKey
server::talk::Root::newsgroupMap()
{
    return forumRoot().hashKey("newsgroups");
}

afl::net::redis::HashKey
server::talk::Root::forumMap()
{
    return forumRoot().hashKey("byname");
}

afl::net::redis::Subtree
server::talk::Root::emailRoot()
{
    return afl::net::redis::Subtree(m_db, EMAIL_ROOT);
}

afl::net::redis::Subtree
server::talk::Root::defaultFolderRoot()
{
    return afl::net::redis::Subtree(m_db, DEFAULT_FOLDER_ROOT);
}

afl::net::redis::Subtree
server::talk::Root::pmRoot()
{
    return afl::net::redis::Subtree(m_db, PM_ROOT);
}

afl::net::redis::Subtree
server::talk::Root::rfcMessageIdRoot()
{
    return afl::net::redis::Subtree(m_db, RFC_MSGID_ROOT);
}

bool
server::talk::Root::checkUserPermission(String_t privString, String_t user)
{
    if (privString.empty()) {
        return false;
    }

    // Process
    do {
        String_t me(afl::string::strFirst(privString, ","));
        bool result = true;
        if (me.size() > 0 && me[0] == '-') {
            // Invert result
            result = false;
            me.erase(0, 1);
        }
        if (me == "all") {
            // blanket permission
            return result;
        } else if (me.size() > 2 && me.compare(0, 2, "p:", 2) == 0) {
            // check profile permission
            std::auto_ptr<afl::data::Value> value(User(*this, user).getProfileRaw(me.substr(2)));
            if (toInteger(value.get()) > 0) {
                return result;
            }
        } else if (me.size() > 2 && me.compare(0, 2, "u:", 2) == 0) {
            // check user name
            if (me.compare(2, String_t::npos, user) == 0) {
                return result;
            }
        } else if (me.size() > 2 && me.compare(0, 2, "g:", 2) == 0) {
            // check game membership
            if (gameRoot().subtree(me.substr(2)).hashKey("users").field(user).exists()) {
                return result;
            }
        } else {
            // unknown privilege token
        }
    } while (afl::string::strRemove(privString, ","));
    return false;
}
