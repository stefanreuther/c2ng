/**
  *  \file util/messagecollector.cpp
  */

#include "util/messagecollector.hpp"
#include "afl/sys/mutexguard.hpp"

namespace {
    const char DROP_TEXT[] = "drop";
    const char HIDE_TEXT[] = "hide";

    const size_t MAX_SIZE = 2000;
    const size_t MIN_SIZE = 1000;
}

util::MessageCollector::MessageCollector()
    : m_mutex(),
      m_config(),
      m_messages(),
      m_firstMessageNumber(0)
{ }

util::MessageCollector::~MessageCollector()
{ }

void
util::MessageCollector::handleMessage(const Message& msg)
{
    afl::sys::MutexGuard g(m_mutex);

    // Check config.
    // If match fails, it does not modify action, which is just what we need
    String_t action;
    m_config.match(msg, action);

    if (action != DROP_TEXT) {
        // Store the message
        String_t::size_type n = msg.m_message.find('\n');
        const bool show = action != HIDE_TEXT;
        if (n != String_t::npos) {
            String_t::size_type p = 0;
            while (n != String_t::npos) {
                Message copy = { msg.m_time, msg.m_level, msg.m_channel, msg.m_message.substr(p, n-p) };
                m_messages.pushBackNew(new Item(copy, show));
                p = n+1;
                n = msg.m_message.find('\n', p);
            }
            Message copy = { msg.m_time, msg.m_level, msg.m_channel, msg.m_message.substr(p) };
            m_messages.pushBackNew(new Item(copy, show));
        } else {
            m_messages.pushBackNew(new Item(msg, show));
        }

        // If this overflows the buffer, clean up.
        // FIXME: make a nicer cleanup scheme
        if (m_messages.size() > MAX_SIZE) {
            m_messages.erase(m_messages.begin(), m_messages.begin() + MAX_SIZE - MIN_SIZE);
            m_firstMessageNumber += MAX_SIZE - MIN_SIZE;
        }
    }
}

void
util::MessageCollector::setConfiguration(String_t filter, afl::string::Translator& tx)
{
    // Preparation: update configuration in a temporary object so we don't change the state if it fails midway.
    MessageMatcher mm;
    mm.setConfiguration(filter, tx);

    // Change state
    afl::sys::MutexGuard g(m_mutex);
    m_config = mm;

    // Update message visibility state
    for (size_t i = 0, n = m_messages.size(); i < n; ++i) {
        String_t action;
        m_messages[i]->visible = !(m_config.match(m_messages[i]->message, action)
                                   && (action == DROP_TEXT || action == HIDE_TEXT));
    }
}

util::MessageCollector::MessageNumber_t
util::MessageCollector::getOldestPosition() const
{
    afl::sys::MutexGuard g(const_cast<afl::sys::Mutex&>(m_mutex));
    return m_firstMessageNumber;
}

util::MessageCollector::MessageNumber_t
util::MessageCollector::getNewestPosition() const
{
    afl::sys::MutexGuard g(const_cast<afl::sys::Mutex&>(m_mutex));
    return m_firstMessageNumber + m_messages.size();
}

bool
util::MessageCollector::readNewerMessage(MessageNumber_t startAt, Message* storeMessage, MessageNumber_t& next) const
{
    afl::sys::MutexGuard g(const_cast<afl::sys::Mutex&>(m_mutex));

    // Find starting index
    MessageNumber_t startingIndex;
    if (startAt < m_firstMessageNumber) {
        startingIndex = 0;
    } else {
        startingIndex = startAt - m_firstMessageNumber;
    }

    // Range check
    size_t n = m_messages.size();
    if (startingIndex >= n) {
        return false;
    }

    // Find message
    size_t i = static_cast<size_t>(startingIndex);
    while (i < n && !m_messages[i]->visible) {
        ++i;
    }

    // Produce output
    if (i < n) {
        if (storeMessage) {
            *storeMessage = m_messages[i]->message;
        }
        next = m_firstMessageNumber + i + 1;
        return true;
    } else {
        return false;
    }
}

bool
util::MessageCollector::readOlderMessage(MessageNumber_t startAt, Message* storeMessage, MessageNumber_t& next) const
{
    afl::sys::MutexGuard g(const_cast<afl::sys::Mutex&>(m_mutex));

    // Range check
    if (startAt <= m_firstMessageNumber) {
        return false;
    }

    // Find starting index
    MessageNumber_t startingIndex = startAt - m_firstMessageNumber;
    size_t n = m_messages.size();
    size_t i = (startingIndex < n ? static_cast<size_t>(startingIndex) : n);

    // Find message
    while (i > 0 && !m_messages[i-1]->visible) {
        --i;
    }

    // Produce output
    if (i > 0) {
        --i;
        if (storeMessage) {
            *storeMessage = m_messages[i]->message;
        }
        next = m_firstMessageNumber + i;
        return true;
    } else {
        return false;
    }
}
