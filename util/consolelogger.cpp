/**
  *  \file util/consolelogger.cpp
  *  \brief Class util::ConsoleLogger
  */

#include "util/consolelogger.hpp"
#include "afl/sys/mutexguard.hpp"
#include "afl/string/format.hpp"

/***************************** ConsoleLogger *****************************/

util::ConsoleLogger::ConsoleLogger()
    : m_mutex(),
      m_matcher()
{ }

util::ConsoleLogger::~ConsoleLogger()
{ }

void
util::ConsoleLogger::handleMessage(const Message& msg)
{
    afl::sys::MutexGuard g(m_mutex);

    // Filter
    String_t mode;
    if (!m_matcher.match(msg, mode)) {
        mode = "keep";
    }

    if (mode == "hide" || mode == "drop") {
        // Ignore
    } else {
        String_t line;
        if (mode == "raw") {
            // Just the text
            line = msg.m_message;
        } else {
            // Build the text
            const char* level = "";
            switch (msg.m_level) {
             case Trace: level = "Trace";  break;
             case Debug: level = "Debug";  break;
             case Info:  level = "Info";   break;
             case Warn:  level = "Warn";   break;
             case Error: level = "Error";  break;
            }
            line = afl::string::Format("%s [%s] <%s> %s",
                                       msg.m_time.toString(afl::sys::Time::LocalTime, afl::sys::Time::TimeFormat),
                                       msg.m_channel,
                                       level,
                                       msg.m_message);
        }

        // Write it
        int which = (msg.m_level >= Warn);
        if (afl::io::TextWriter* w = m_writers[which].get()) {
            w->writeLine(line);
            w->flush();
        }
    }
}

void
util::ConsoleLogger::attachWriter(bool error, afl::base::Ptr<afl::io::TextWriter> w)
{
    afl::sys::MutexGuard g(m_mutex);
    m_writers[error] = w;
}

void
util::ConsoleLogger::setConfiguration(String_t config, afl::string::Translator& tx)
{
    m_matcher.setConfiguration(config, tx);
}
