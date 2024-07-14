/**
  *  \file server/talk/talkuser.cpp
  *  \brief Class server::talk::TalkUser
  */

#include "server/talk/talkuser.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/talkforum.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/user.hpp"
#include "server/talk/newsrc.hpp"
#include "server/errors.hpp"

namespace {
    class NewsrcAction {
     public:
        NewsrcAction(afl::net::redis::Subtree n);
        void process(int32_t messageId);
        void process(afl::net::redis::IntegerSetKey set);
        void save();

        void setModification(server::interface::TalkUser::Modification modif);
        void markGet();
        void markFind(bool value);
        void markWantId();
        bool isStopped() const;
        afl::data::Value* getResult();

     private:
        server::talk::Newsrc n;

        bool get;               // we want to get the values
        server::interface::TalkUser::Modification m_modification : 8;
        bool find;              // we want to find items (requires get)
        bool value;             // value of the items we want to find
        bool found;             // set if we found a matching item
        bool stop;              // set if we can stop processing
        bool wantId;            // set if we want the Id of the first item
        String_t result;
        int32_t firstId;
    };
}

NewsrcAction::NewsrcAction(afl::net::redis::Subtree n)
    : n(n),
      get(false),
      m_modification(server::talk::TalkUser::NoModification),
      find(false),
      value(false),
      found(false),
      stop(false),
      wantId(false),
      result(),
      firstId(0)
{ }

void
NewsrcAction::process(int32_t messageId)
{
    if (get) {
        bool me = n.get(messageId);
        if (find) {
            if (me == value) {
                found = true;
                if (m_modification == server::talk::TalkUser::NoModification) {
                    stop = true;
                }
                if (firstId == 0) {
                    firstId = messageId;
                }
            }
        } else {
            result += char('0' + me);
        }
    }

    switch (m_modification) {
     case server::talk::TalkUser::NoModification:                      break;
     case server::talk::TalkUser::MarkRead:        n.set(messageId);   break;
     case server::talk::TalkUser::MarkUnread:      n.clear(messageId); break;
    }
}

void
NewsrcAction::process(afl::net::redis::IntegerSetKey set)
{
    // FIXME: possible optimisation: if we're just checking for any unread message
    // (find && !value && !wantId && !set && !clear), process the list backwards.
    afl::data::IntegerList_t result;
    set.getAll(result);
    std::sort(result.begin(), result.end());
    for (size_t i = 0, n = result.size(); i < n && !stop; ++i) {
        process(result[i]);
    }
}

inline void
NewsrcAction::save()
{
    n.save();
}

inline void
NewsrcAction::setModification(server::interface::TalkUser::Modification modif)
{
    m_modification = modif;
}

inline void
NewsrcAction::markGet()
{
    get = true;
}

inline void
NewsrcAction::markFind(bool value)
{
    this->get   = true;
    this->find  = true;
    this->value = value;
}

inline void
NewsrcAction::markWantId()
{
    wantId = true;
}

inline bool
NewsrcAction::isStopped() const
{
    return stop;
}

afl::data::Value*
NewsrcAction::getResult()
{
    if (get) {
        if (wantId) {
            return server::makeIntegerValue(firstId);
        } else if (find) {
            // Searching for 1, found it --> result 1
            // Searching for 0, found it --> result 0
            // Searching for 1, none found --> result 0
            // Searching for 0, none found --> result 1
            return server::makeIntegerValue(found == value);
        } else {
            return server::makeStringValue(result);
        }
    } else {
        return server::makeStringValue("OK");
    }
}

/******************************** TalkUser *******************************/

server::talk::TalkUser::TalkUser(Session& session, Root& root)
    : m_session(session), m_root(root)
{ }

afl::data::Value*
server::talk::TalkUser::accessNewsrc(Modification modif, Result res, afl::base::Memory<const Selection> selections, afl::base::Memory<const int32_t> posts)
{
    m_session.checkUser();

    NewsrcAction action(User(m_root, m_session.getUser()).newsrc());
    int32_t limit = -1;

    action.setModification(modif);
    switch (res) {
     case NoResult:                                                    break;
     case GetAll:         action.markGet();                            break;
     case CheckIfAnyRead: action.markFind(true);                       break;
     case CheckIfAllRead: action.markFind(false);                      break;
     case GetFirstRead:   action.markFind(true);  action.markWantId(); break;
     case GetFirstUnread: action.markFind(false); action.markWantId(); break;
    }

    const Selection* p;
    while (!action.isStopped() && (p = selections.eat()) != 0) {
        switch (p->scope) {
         case RangeScope:
            if (limit == -1) {
                limit = m_root.lastMessageId().get();
            }
            if (p->id <= 0 || p->lastId > limit) {
                throw std::runtime_error(MESSAGE_NOT_FOUND);   // @change: was 413 Range error in PCC2, but actually means Message not found.
            }
            for (int32_t i = p->id; i <= p->lastId && !action.isStopped(); ++i) {
                action.process(i);
            }
            break;

         case ForumScope:
            action.process(Forum(m_root, p->id).messages());
            break;

         case ThreadScope:
            action.process(Topic(m_root, p->id).messages());
            break;
        }
    }

    if (!posts.empty()) {
        if (limit == -1) {
            limit = m_root.lastMessageId().get();
        }

        const int32_t* p;
        while (!action.isStopped() && (p = posts.eat()) != 0) {
            if (*p <= 0 || *p > limit) {
                throw std::runtime_error(MESSAGE_NOT_FOUND);   // @change: was 413 Range error in PCC2, but actually means Message not found.
            }
            action.process(*p);
        }
    }

    action.save();
    return action.getResult();
}

void
server::talk::TalkUser::watch(afl::base::Memory<const Selection> selections)
{
    processWatch(Watch, selections);
}

void
server::talk::TalkUser::unwatch(afl::base::Memory<const Selection> selections)
{
    processWatch(Unwatch, selections);
}

void
server::talk::TalkUser::markSeen(afl::base::Memory<const Selection> selections)
{
    processWatch(MarkSeen, selections);
}

afl::data::Value*
server::talk::TalkUser::getWatchedThreads(const ListParameters& params)
{
    // ex doUserListWatchedThreads
    m_session.checkUser();
    return TalkForum::executeListOperation(params, User(m_root, m_session.getUser()).watchedTopics(), Topic::TopicSorter(m_root));
}

afl::data::Value*
server::talk::TalkUser::getWatchedForums(const ListParameters& params)
{
    // ex doUserListWatchedForums
    m_session.checkUser();
    return TalkForum::executeListOperation(params, User(m_root, m_session.getUser()).watchedForums(), Forum::ForumSorter(m_root));
}

afl::data::Value*
server::talk::TalkUser::getPostedMessages(String_t user, const ListParameters& params)
{
    // ex doUserListPosted
    return TalkForum::executeListOperation(params, User(m_root, user).postedMessages(), Message::MessageSorter(m_root));
}

void
server::talk::TalkUser::processWatch(WatchAction action, afl::base::Memory<const Selection> selections)
{
    // ex WatchAction::process
    // User context
    const String_t user = m_session.getUser();
    m_session.checkUser();
    User u(m_root, user);

    // Permission checks are done when a post is submitted, so there's no need
    // to do them here. The user interface will not let users subscribe to topics
    // they are not allowed to read, so there'll not be too many database zombies.
    while (const Selection* p = selections.eat()) {
        switch (p->scope) {
         case ThreadScope: {
            int32_t tid = p->id;
            Topic t(m_root, tid);
            if (!t.exists()) {
                throw std::runtime_error(TOPIC_NOT_FOUND);
            }
            if (action == Watch) {
                t.watchers().add(user);
                u.watchedTopics().add(tid);
            }
            if (action == Unwatch) {
                t.watchers().remove(user);
                u.watchedTopics().remove(tid);
            }
            u.notifiedTopics().remove(tid);
            break;
         }
         case ForumScope: {
            int32_t fid = p->id;
            Forum f(m_root, fid);
            if (!f.exists(m_root)) {
                throw std::runtime_error(FORUM_NOT_FOUND);
            }
            if (action == Watch) {
                f.watchers().add(user);
                u.watchedForums().add(fid);
            }
            if (action == Unwatch) {
                f.watchers().remove(user);
                u.watchedForums().remove(fid);
            }
            u.notifiedForums().remove(fid);
            break;
         }
         case RangeScope:
            throw std::runtime_error(INVALID_OPTION);
        }
    }
}
