/**
  *  \file test/util/messagecollectortest.cpp
  *  \brief Test for util::MessageCollector
  */

#include "util/messagecollector.hpp"

#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Test forward iteration. */
AFL_TEST("util.MessageCollector:forward", a)
{
    const int N = 10;
    util::MessageCollector testee;
    afl::string::NullTranslator tx;

    // Populate it
    testee.setConfiguration("keep=keep:drop=drop:hide=hide", tx);
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
            a.check("01. limit", limit < N);
            ++limit;

            // Collect results
            result += msg.m_message;
        }
        a.checkEqual("11. result", result, "k0k1k2k3k4k5k6k7k8k9");
    }

    // Reconfigure and iterate again
    testee.setConfiguration("*=keep", tx);
    {
        String_t result;
        afl::sys::LogListener::Message msg;
        util::MessageCollector::MessageNumber_t nr = testee.getOldestPosition();
        int limit = 0;
        while (testee.readNewerMessage(nr, &msg, nr)) {
            // Make sure we don't run into an infinite loop
            a.check("21. limit", limit < 2*N);
            ++limit;

            // Collect results
            result += msg.m_message;
        }
        a.checkEqual("31. result", result, "k0h0k1h1k2h2k3h3k4h4k5h5k6h6k7h7k8h8k9h9");
    }
}

/** Test backward iteration. */
AFL_TEST("util.MessageCollector:backward", a)
{
    const int N = 10;
    util::MessageCollector testee;
    afl::string::NullTranslator tx;

    // Populate it
    testee.setConfiguration("keep=keep:drop=drop:hide=hide", tx);
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
            a.check("01. limit", limit < N);
            ++limit;

            // Collect results
            result += msg.m_message;
        }
        a.checkEqual("11. result", result, "k9k8k7k6k5k4k3k2k1k0");
    }

    // Reconfigure and iterate again
    testee.setConfiguration("*=keep", tx);
    {
        String_t result;
        afl::sys::LogListener::Message msg;
        util::MessageCollector::MessageNumber_t nr = testee.getNewestPosition();
        int limit = 0;
        while (testee.readOlderMessage(nr, &msg, nr)) {
            // Make sure we don't run into an infinite loop
            a.check("21. limit", limit < 2*N);
            ++limit;

            // Collect results
            result += msg.m_message;
        }
        a.checkEqual("31. result", result, "h9k9h8k8h7k7h6k6h5k5h4k4h3k3h2k2h1k1h0k0");
    }
}

/** Test message collection with embedded wrap. */
AFL_TEST("util.MessageCollector:wrap", a)
{
    util::MessageCollector testee;
    afl::string::NullTranslator tx;

    const int N = 10;

    // Populate it
    testee.setConfiguration("keep=keep:drop=drop:hide=hide", tx);
    testee.write(afl::sys::LogListener::Info, "keep", "kpre\nkmid\nkfinal");
    testee.write(afl::sys::LogListener::Info, "drop", "dpre\ndmid\ndfinal");
    testee.write(afl::sys::LogListener::Info, "hide", "hpre\nhmid\nhfinal");

    // Iterate
    {
        String_t result;
        afl::sys::LogListener::Message msg;
        util::MessageCollector::MessageNumber_t nr = testee.getOldestPosition();
        int limit = 0;
        while (testee.readNewerMessage(nr, &msg, nr)) {
            // Make sure we don't run into an infinite loop
            a.check("01. limit", limit < N);
            ++limit;

            // Collect results
            result += msg.m_message;
            result += ".";
        }
        a.checkEqual("11. result", result, "kpre.kmid.kfinal.");
    }

    // Reconfigure and iterate again
    testee.setConfiguration("*=keep", tx);
    {
        String_t result;
        afl::sys::LogListener::Message msg;
        util::MessageCollector::MessageNumber_t nr = testee.getOldestPosition();
        int limit = 0;
        while (testee.readNewerMessage(nr, &msg, nr)) {
            // Make sure we don't run into an infinite loop
            a.check("21. limit", limit < 2*N);
            ++limit;

            // Collect results
            result += msg.m_message;
            result += ".";
        }
        a.checkEqual("31. result", result, "kpre.kmid.kfinal.hpre.hmid.hfinal.");
    }
}
