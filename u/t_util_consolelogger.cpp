/**
  *  \file u/t_util_consolelogger.cpp
  *  \brief Test for util::ConsoleLogger
  */

#include "util/consolelogger.hpp"

#include "t_util.hpp"
#include "afl/io/internaltextwriter.hpp"
#include "afl/base/ref.hpp"
#include "afl/string/nulltranslator.hpp"

using afl::base::Ref;
using afl::io::InternalTextWriter;
using afl::sys::LogListener;
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

    testee.handleMessage(makeMessage(Time(1500000000000), LogListener::Info, "ch.inf", "Informational message"));
    testee.handleMessage(makeMessage(Time(1500000001000), LogListener::Error, "ch.err", "Error message"));

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

    testee.handleMessage(makeMessage(Time(1600000000000), LogListener::Info, "ch.hidden", "Hidden message"));
    testee.handleMessage(makeMessage(Time(1600000001000), LogListener::Info, "ch.plain", "Raw message"));
    testee.handleMessage(makeMessage(Time(1600000002000), LogListener::Trace, "ch.normal", "Normal message"));
    testee.handleMessage(makeMessage(Time(1600000003000), LogListener::Debug, "ch.unmapped", "Unmapped message"));
    testee.handleMessage(makeMessage(Time(1600000002000), LogListener::Warn, "ch.normal", "Warning message"));      // not shown, wrong channel

    TS_ASSERT_EQUALS(afl::string::fromMemory(out->getContent()),
                     "Raw message\n"
                     "14:26:42 [ch.normal] <Trace> Normal message\n"
                     "14:26:43 [ch.unmapped] <Debug> Unmapped message\n");
}

