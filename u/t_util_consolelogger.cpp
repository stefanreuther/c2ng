/**
  *  \file u/t_util_consolelogger.cpp
  *  \brief Test for util::ConsoleLogger
  */

#include "util/consolelogger.hpp"

#include "t_util.hpp"
#include "afl/io/internaltextwriter.hpp"
#include "afl/base/ref.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/parsedtime.hpp"

using afl::base::Ref;
using afl::io::InternalTextWriter;
using afl::sys::LogListener;
using afl::sys::ParsedTime;
using afl::sys::Time;

namespace {
    LogListener::Message makeMessage(Time t, LogListener::Level level, String_t channel, String_t message)
    {
        LogListener::Message msg;
        msg.m_time = t;
        msg.m_level = level;
        msg.m_channel = channel;
        msg.m_message = message;
        return msg;
    }
}

/** Test default configuration. */
void
TestUtilConsoleLogger::testDefault()
{
    Ref<InternalTextWriter> err = *new InternalTextWriter();
    Ref<InternalTextWriter> out = *new InternalTextWriter();

    util::ConsoleLogger testee;
    testee.attachWriter(true, err.asPtr());
    testee.attachWriter(false, out.asPtr());

    ParsedTime pt = { 2017, 7, 14, 4, 40, 0, 0, 0 };
    testee.handleMessage(makeMessage(Time(pt, Time::LocalTime), LogListener::Info, "ch.inf", "Informational message"));
    ++pt.m_second;
    testee.handleMessage(makeMessage(Time(pt, Time::LocalTime), LogListener::Error, "ch.err", "Error message"));

    TS_ASSERT_EQUALS(afl::string::fromMemory(out->getContent()), "04:40:00 [ch.inf] <Info> Informational message\n");
    TS_ASSERT_EQUALS(afl::string::fromMemory(err->getContent()), "04:40:01 [ch.err] <Error> Error message\n");
}

/** Test manual configuration. */
void
TestUtilConsoleLogger::testConfig()
{
    Ref<InternalTextWriter> out = *new InternalTextWriter();

    afl::string::NullTranslator tx;
    util::ConsoleLogger testee;
    testee.attachWriter(false, out.asPtr());
    testee.setConfiguration("ch.hidden=hide:ch.plain=raw:ch.normal=show", tx);

    ParsedTime pt = { 2020, 9, 13, 14, 26, 40, 0, 0 };
    testee.handleMessage(makeMessage(Time(pt, Time::LocalTime), LogListener::Info, "ch.hidden", "Hidden message"));
    ++pt.m_second;
    testee.handleMessage(makeMessage(Time(pt, Time::LocalTime), LogListener::Info, "ch.plain", "Raw message"));
    ++pt.m_second;
    testee.handleMessage(makeMessage(Time(pt, Time::LocalTime), LogListener::Trace, "ch.normal", "Normal message"));
    ++pt.m_second;
    testee.handleMessage(makeMessage(Time(pt, Time::LocalTime), LogListener::Debug, "ch.unmapped", "Unmapped message"));
    ++pt.m_second;
    testee.handleMessage(makeMessage(Time(pt, Time::LocalTime), LogListener::Warn, "ch.normal", "Warning message"));      // not shown, wrong channel

    TS_ASSERT_EQUALS(afl::string::fromMemory(out->getContent()),
                     "Raw message\n"
                     "14:26:42 [ch.normal] <Trace> Normal message\n"
                     "14:26:43 [ch.unmapped] <Debug> Unmapped message\n");
}

