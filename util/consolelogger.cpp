/**
  *  \file util/consolelogger.cpp
  *  \brief Class util::ConsoleLogger
  */

/*
 *  Configuration
 */

// Define if you want output to unconditionally go onto stdout/stderr for debugging without afl getting in the way.
#undef USE_STDOUT

// Define to show thread names with each message. As of 20160801, Linux only.
#undef USE_THREADNAME


/*
 *  Includes
 */

#ifdef USE_STDOUT
# include <iostream>
#endif
#include "util/consolelogger.hpp"
#include "afl/sys/mutexguard.hpp"
#include "afl/string/format.hpp"


/*
 *  Thread Names
 */

#if defined(__linux__) && defined(USE_THREADNAME)
# include <pthread.h>
namespace {
    String_t getCurrentThreadName()
    {
        char buf[16];
        if (pthread_getname_np(pthread_self(), buf, sizeof(buf)) == 0) {
            return String_t("@") + buf;
        } else {
            return String_t();
        }
    }
}
# define THREADID getCurrentThreadName()
#else
# define THREADID ""
#endif

/***************************** ConsoleLogger *****************************/

util::ConsoleLogger::ConsoleLogger()
    : m_mutex()
{ }

util::ConsoleLogger::~ConsoleLogger()
{ }

void
util::ConsoleLogger::handleMessage(const Message& msg)
{
    // Filter
    if (msg.m_channel == "interpreter.process" && msg.m_level == Trace) {
        return;
    }

#ifdef USE_STDOUT
    const char* level = "";
    std::ostream* channel = &std::cout;
    switch (msg.m_level) {
     case Trace: level = "\033[30;1m<Trace>\033[0m "; break;
     case Debug: level = "<Debug> "; break;
     case Info:  level = "\033[35m<Info>\033[0m ";  break;
     case Warn:  level = "\033[33m<Warn>\033[0m ";  channel = &std::cerr; break;
     case Error: level = "\033[31m<Error>\033[0m "; channel = &std::cerr; break;
    }

    *channel << msg.m_time.toString(afl::sys::Time::LocalTime, afl::sys::Time::TimeFormat) << " \033[1m[" << msg.m_channel << THREADID << "]\033[0m " << level << msg.m_message << "\n";
#else
    // Build the text
    const char* level = "";
    switch (msg.m_level) {
     case Trace: level = "Trace";  break;
     case Debug: level = "Debug";  break;
     case Info:  level = "Info";   break;
     case Warn:  level = "Warn";   break;
     case Error: level = "Error";  break;
    }
    String_t line = afl::string::Format("%s [%s] <%s> %s",
                                        msg.m_time.toString(afl::sys::Time::LocalTime, afl::sys::Time::TimeFormat),
                                        msg.m_channel,
                                        level,
                                        msg.m_message);

    // Write it
    afl::sys::MutexGuard g(m_mutex);
    int which = (msg.m_level >= Warn);
    if (afl::io::TextWriter* w = m_writers[which].get()) {
        w->writeLine(line);
        w->flush();
    }
#endif
}

void
util::ConsoleLogger::attachWriter(bool error, afl::base::Ptr<afl::io::TextWriter> w)
{
    afl::sys::MutexGuard g(m_mutex);
    m_writers[error] = w;
}
