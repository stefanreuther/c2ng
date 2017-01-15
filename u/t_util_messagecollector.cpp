/**
  *  \file u/t_util_messagecollector.cpp
  *  \brief Test for util::MessageCollector
  */

#include <stdio.h>
#include "util/messagecollector.hpp"

#include "t_util.hpp"
#include "afl/string/format.hpp"

/** Test forward iteration. */
void
TestUtilMessageCollector::testForward()
{
    const int N = 10;
    util::MessageCollector testee;

    // Populate it
    testee.setConfiguration("keep=keep:drop=drop:hide=hide");
    for (int i = 0; i < N; ++i) {
        testee.write(afl::sys::LogListener::Info, "keep", afl::string::Format("k%d", i));
        testee.write(afl::sys::LogListener::Info, "drop", afl::string::Format("d%d", i));
        testee.write(afl::sys::LogListener::Info, "hide", afl::string::Format("h%d", i));
    }

    // Iterate
    {
        String_t result;
        afl::sys::LogListener::Message msg;
        util::MessageCollector::MessageNumber_t nr = testee.getOldestPosition();
        int limit = 0;
        while (testee.readNewerMessage(nr, &msg, nr)) {
            // Make sure we don't run into an infinite loop
            TS_ASSERT(limit < N);
            ++limit;

            // Collect results
            result += msg.m_message;
        }
        TS_ASSERT_EQUALS(result, "k0k1k2k3k4k5k6k7k8k9");
    }

    // Reconfigure and iterate again
    testee.setConfiguration("*=keep");
    {
        String_t result;
        afl::sys::LogListener::Message msg;
        util::MessageCollector::MessageNumber_t nr = testee.getOldestPosition();
        int limit = 0;
        while (testee.readNewerMessage(nr, &msg, nr)) {
            // Make sure we don't run into an infinite loop
            TS_ASSERT(limit < 2*N);
            ++limit;

            // Collect results
            result += msg.m_message;
        }
        TS_ASSERT_EQUALS(result, "k0h0k1h1k2h2k3h3k4h4k5h5k6h6k7h7k8h8k9h9");
    }
}

/** Test backward iteration. */
void
TestUtilMessageCollector::testBackward()
{
    const int N = 10;
    util::MessageCollector testee;

    // Populate it
    testee.setConfiguration("keep=keep:drop=drop:hide=hide");
    for (int i = 0; i < N; ++i) {
        testee.write(afl::sys::LogListener::Info, "keep", afl::string::Format("k%d", i));
        testee.write(afl::sys::LogListener::Info, "drop", afl::string::Format("d%d", i));
        testee.write(afl::sys::LogListener::Info, "hide", afl::string::Format("h%d", i));
    }

    // Iterate
    {
        String_t result;
        afl::sys::LogListener::Message msg;
        util::MessageCollector::MessageNumber_t nr = testee.getNewestPosition();
        int limit = 0;
        while (testee.readOlderMessage(nr, &msg, nr)) {
            // Make sure we don't run into an infinite loop
            TS_ASSERT(limit < N);
            ++limit;

            // Collect results
            result += msg.m_message;
        }
        TS_ASSERT_EQUALS(result, "k9k8k7k6k5k4k3k2k1k0");
    }

    // Reconfigure and iterate again
    testee.setConfiguration("*=keep");
    {
        String_t result;
        afl::sys::LogListener::Message msg;
        util::MessageCollector::MessageNumber_t nr = testee.getNewestPosition();
        int limit = 0;
        while (testee.readOlderMessage(nr, &msg, nr)) {
            // Make sure we don't run into an infinite loop
            TS_ASSERT(limit < 2*N);
            ++limit;

            // Collect results
            result += msg.m_message;
        }
        TS_ASSERT_EQUALS(result, "h9k9h8k8h7k7h6k6h5k5h4k4h3k3h2k2h1k1h0k0");
    }
}
