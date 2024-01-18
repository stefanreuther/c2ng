/**
  *  \file test/game/v3/outboxreadertest.cpp
  *  \brief Test for game::v3::OutboxReader
  */

#include "game/v3/outboxreader.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    class Tester : public game::v3::OutboxReader, public afl::test::CallReceiver {
     public:
        Tester(const afl::test::Assert& a)
            : CallReceiver(a)
            { }
        virtual void addMessage(String_t text, game::PlayerSet_t receivers)
            { checkCall(afl::string::Format("addMessage('%s', %d)", text, receivers.toInteger())); }
    };
}

/** Test reading an empty file.
    Should not generate any callbacks or errors. */
AFL_TEST("game.v3.OutboxReader:loadOutbox:empty", a)
{
    Tester t(a);

    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    afl::io::ConstMemoryStream ms(afl::base::Nothing);
    AFL_CHECK_SUCCEEDS(a, t.loadOutbox(ms, cs, tx));
}

/** Test reading a file containing a zero.
    Should not generate any callbacks or errors. */
AFL_TEST("game.v3.OutboxReader:loadOutbox:zero", a)
{
    Tester t(a);

    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    static const uint8_t DATA[] = {0,0};
    afl::io::ConstMemoryStream ms(DATA);
    AFL_CHECK_SUCCEEDS(a, t.loadOutbox(ms, cs, tx));
}

/** Test reading a file containing a zero-length message.
    Should not generate any callbacks or errors. */
AFL_TEST("game.v3.OutboxReader:loadOutbox:zero-length", a)
{
    Tester t(a);

    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    static const uint8_t DATA[] = {
        1,0,                    // numMessages
        13,0,0,0,               // address
        0,0,                    // length
        7,0,                    // from
        2,0,                    // to
    };
    afl::io::ConstMemoryStream ms(DATA);
    AFL_CHECK_SUCCEEDS(a, t.loadOutbox(ms, cs, tx));
}

/** Test reading a file containing a single message. */
AFL_TEST("game.v3.OutboxReader:loadOutbox:one-message", a)
{
    Tester t(a);

    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    static const uint8_t DATA[] = {
        1,0,                    // numMessages
        13,0,0,0,               // address
        6,0,                    // length
        7,0,                    // from
        2,0,                    // to
        'n','o','p',26,'q','r',
    };
    afl::io::ConstMemoryStream ms(DATA);
    t.expectCall("addMessage('abc\nde', 4)");
    t.loadOutbox(ms, cs, tx);
    t.checkFinish();
}

/** Test reading a file containing a single message to host (special case). */
AFL_TEST("game.v3.OutboxReader:loadOutbox:message-to-host", a)
{
    Tester t(a);

    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    static const uint8_t DATA[] = {
        1,0,                    // numMessages
        13,0,0,0,               // address
        6,0,                    // length
        7,0,                    // from
        12,0,                   // to
        'n','o','p',26,'q','r',
    };
    afl::io::ConstMemoryStream ms(DATA);
    t.expectCall("addMessage('abc\nde', 1)");
    t.loadOutbox(ms, cs, tx);
    t.checkFinish();
}

/** Test reading an empty 3.5 file.
    Should not generate any callbacks or errors. */
AFL_TEST("game.v3.OutboxReader:loadOutbox35:empty", a)
{
    Tester t(a);

    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    afl::io::ConstMemoryStream ms(afl::base::Nothing);
    AFL_CHECK_SUCCEEDS(a, t.loadOutbox35(ms, cs, tx));
}

/** Test reading a 3.5 file containing a zero.
    Should not generate any callbacks or errors. */
AFL_TEST("game.v3.OutboxReader:loadOutbox35:zero", a)
{
    Tester t(a);

    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    static const uint8_t DATA[] = {
        0,0,                                               // length
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0  // sufficient padding
    };
    afl::io::ConstMemoryStream ms(DATA);
    AFL_CHECK_SUCCEEDS(a, t.loadOutbox35(ms, cs, tx));
}

/** Test reading a 3.5 file containing a zero-length message.
    Should not generate any callbacks or errors.
    This case does not normally appear, empty messages are still allocated with 600 bytes. */
AFL_TEST("game.v3.OutboxReader:loadOutbox35:zero-length", a)
{
    Tester t(a);

    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    static const uint8_t DATA[] = {
        1,0,                                              // count,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,                // pad
        3,                                                // pad
        '1',                                              // valid
        '1','1','0','0','0','0','0','0','0','0','0','0',  // receivers
        0,0,                                              // length
    };
    afl::io::ConstMemoryStream ms(DATA);
    AFL_CHECK_SUCCEEDS(a, t.loadOutbox35(ms, cs, tx));
}

/** Test reading a 3.5 file containing a single message. */
AFL_TEST("game.v3.OutboxReader:loadOutbox35:one-message", a)
{
    Tester t(a);

    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    static const uint8_t DATA[] = {
        1,0,                                              // count,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,                // pad
        3,                                                // pad
        '1',                                              // valid
        '1','1','0','0','0','0','0','0','0','0','0','0',  // receivers
        10,0,                                             // length
        'n','o','p',26,'q','r','-','-','-','-',
    };
    afl::io::ConstMemoryStream ms(DATA);
    t.expectCall("addMessage('abc\nde', 6)");
    t.loadOutbox35(ms, cs, tx);
    t.checkFinish();
}

/** Test reading a 3.5 file containing two messages. */
AFL_TEST("game.v3.OutboxReader:loadOutbox35:two-messages", a)
{
    Tester t(a);

    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    static const uint8_t DATA[] = {
        2,0,                                              // count,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,                // pad
        3,                                                // pad
        '1',                                              // valid
        '1','1','1','0','0','0','0','0','0','0','0','0',  // receivers
        10,0,                                             // length
        'n','o','p',26,'q','r','-','-','-','-',
        4,
        '1',
        '0','0','0','0','0','0','0','0','0','0','0','1',  // receivers
        5,0,                                              // length
        's','t','u',26,'-',
    };
    afl::io::ConstMemoryStream ms(DATA);
    t.expectCall("addMessage('abc\nde', 14)");
    t.expectCall("addMessage('fgh', 1)");
    t.loadOutbox35(ms, cs, tx);
    t.checkFinish();
}

/** Test reading a 3.5 file containing a message marked as invalid. */
AFL_TEST("game.v3.OutboxReader:loadOutbox35:invalid", a)
{
    Tester t(a);

    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    static const uint8_t DATA[] = {
        2,0,                                              // count,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,                // pad
        3,                                                // pad
        '0',                                              // not valid
        '1','1','1','0','0','0','0','0','0','0','0','0',  // receivers
        10,0,                                             // length
        'n','o','p',26,'q','r','-','-','-','-',
        4,
        '1',
        '0','0','0','0','0','0','0','0','0','0','0','1',  // receivers
        5,0,                                              // length
        's','t','u',26,'-',
    };
    afl::io::ConstMemoryStream ms(DATA);
    t.expectCall("addMessage('fgh', 1)");
    t.loadOutbox35(ms, cs, tx);
    t.checkFinish();
}
