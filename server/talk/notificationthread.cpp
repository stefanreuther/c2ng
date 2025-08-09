/**
  *  \file server/talk/notificationthread.cpp
  *  \brief Class server::talk::NotificationThread
  */

#include "server/talk/notificationthread.hpp"

#include "afl/string/format.hpp"
#include "afl/sys/mutexguard.hpp"
#include "server/talk/message.hpp"
#include "server/talk/notify.hpp"
#include "server/talk/root.hpp"

using afl::string::Format;
using afl::sys::Duration;
using afl::sys::LogListener;
using afl::sys::Time;

const char*const LOG_NAME = "talk.notify";

server::talk::NotificationThread::NotificationThread(Root& root, server::interface::MailQueue& mq)
    : m_root(root),
      m_mailQueue(mq),
      m_semaphore(1),
      m_shutdown(0),
      m_thread("NotificationThread", *this)
{
    m_thread.start();
}

server::talk::NotificationThread::~NotificationThread()
{
    stop();
    m_thread.join();
}

void
server::talk::NotificationThread::notifyMessage(Message& msg)
{
    m_root.messageNotificationQueue().add(msg.getId());
    wake();
}

void
server::talk::NotificationThread::notifyPM(UserPM& msg, const afl::data::StringList_t& notifyIndividual, const afl::data::StringList_t& notifyGroup)
{
    server::talk::notifyPM(msg, notifyIndividual, notifyGroup, m_root, m_mailQueue);
}

void
server::talk::NotificationThread::stop()
{
    m_shutdown = 1;
    wake();
}

void
server::talk::NotificationThread::run()
{
    while (!m_shutdown) {
        try {
            uint32_t time = tick();
            m_semaphore.wait(time);
        }
        catch (std::exception& e) {
            m_root.log().write(LogListener::Warn, LOG_NAME, "", e);
        }
    }
}

inline void
server::talk::NotificationThread::wake()
{
    m_semaphore.post();
}

uint32_t
server::talk::NotificationThread::tick()
{
    afl::sys::MutexGuard g(m_root.mutex());

    // Determine oldest message. If there is none, wait indefinitely.
    afl::data::IntegerList_t msgList;
    m_root.messageNotificationQueue().sort().limit(0, 1).getResult(msgList);
    if (msgList.empty()) {
        return afl::sys::INFINITE_TIMEOUT;
    }

    // Does this message exist? If not, remove and retry.
    const int32_t msgId = msgList[0];
    Message msg(m_root, msgId);
    if (!msg.exists()) {
        m_root.log().write(LogListener::Info, LOG_NAME, Format("message %d lost", msgId));
        m_root.messageNotificationQueue().remove(msgId);
        return 0;
    }

    // Determine time for notification. If reached, remove and notify.
    const Time now = Time::getCurrentTime();
    const Time t = unpackTime(msg.postTime().get()) + Duration::fromMinutes(m_root.config().notificationDelay);
    if (t <= now) {
        // First unqueue the message, then send.
        // This means we prefer losing a notification over having one get stuck.
        // We ignore errors talking to the mailqueue, but not errors talking to the DB.
        m_root.log().write(LogListener::Info, LOG_NAME, Format("notifying message %d", msgId));
        m_root.messageNotificationQueue().remove(msgId);
        try {
            server::talk::notifyMessage(msg, m_root, m_mailQueue);
        }
        catch (std::exception& e) {
            m_root.log().write(LogListener::Error, LOG_NAME, "error during notification", e);
        }
        return 0;
    }

    // We intentionally oversleep to avoid missing the time slightly
    return static_cast<uint32_t>((t - now).getMilliseconds() + 500);
}
