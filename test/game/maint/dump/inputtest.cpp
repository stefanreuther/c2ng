/**
  *  \file test/game/maint/dump/inputtest.cpp
  *  \brief Test for game::maint::dump::Input
  */

#include "game/maint/dump/input.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/test/testrunner.hpp"
#include "game/maint/dump/output.hpp"

/* readByte, data available */
AFL_TEST("game.maint.dump.Input:readByte", a)
{
    static const uint8_t BYTES[] = { 42 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);
    a.checkEqual("result", testee.readByte(), "42");
}

/* readByte, data missing */
AFL_TEST("game.maint.dump.Input:readByte:missing", a)
{
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(afl::base::ConstBytes_t(), cs);
    a.checkEqual("result", testee.readByte(), "<missing>");
}

/* readWord, data available */
AFL_TEST("game.maint.dump.Input:readWord", a)
{
    static const uint8_t BYTES[] = { 1, 2, 0, 0x80 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);
    a.checkEqual("result 1", testee.readWord(), "513");
    a.checkEqual("result 2", testee.readWord(), "-32768");
}

/* readWord, data missing */
AFL_TEST("game.maint.dump.Input:readWord:missing", a)
{
    static const uint8_t BYTES[] = { 1 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);
    a.checkEqual("result", testee.readWord(), "<missing>");
}

/* readLong, data available */
AFL_TEST("game.maint.dump.Input:readLong", a)
{
    static const uint8_t BYTES[] = { 4, 5, 6, 7, 0xfe, 0xff, 0xff, 0xff };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);
    a.checkEqual("result 1", testee.readLong(), "117835012");
    a.checkEqual("result 2", testee.readLong(), "-2");
}

/* readLong, data missing */
AFL_TEST("game.maint.dump.Input:readLong:missing", a)
{
    static const uint8_t BYTES[] = { 4, 5, 6 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);
    a.checkEqual("result", testee.readLong(), "<missing>");
}

/* readCoordinate, data available */
AFL_TEST("game.maint.dump.Input:readCoordinate", a)
{
    static const uint8_t BYTES[] = { 5, 0, 7, 0 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);
    a.checkEqual("result", testee.readCoordinate(), "(5,7)");
}

/* readCoordinate, data missing */
AFL_TEST("game.maint.dump.Input:readCoordinate:missing", a)
{
    static const uint8_t BYTES[] = { 1, 2, 3 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);
    a.checkEqual("result", testee.readCoordinate(), "<missing>");
}

/* readString, data available */
AFL_TEST("game.maint.dump.Input:readString", a)
{
    static const uint8_t BYTES[] = { 97, 98, 99, 32, 32, 32 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);
    a.checkEqual("result", testee.readString(6), "'abc'");
}

/* readString, data missing */
AFL_TEST("game.maint.dump.Input:readString:missing", a)
{
    static const uint8_t BYTES[] = { 97, 98, 99, 32, 32 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);
    a.checkEqual("result", testee.readString(6), "<missing>");
}

/* readPascalString, data available */
AFL_TEST("game.maint.dump.Input:readPascalString", a)
{
    static const uint8_t BYTES[] = { 3, 97, 98, 99, 2, 48, 49 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);
    a.checkEqual("result 1", testee.readPascalString(), "'abc'");
    a.checkEqual("result 2", testee.readPascalString(), "'01'");
}

/* readPascalString, length byte missing */
AFL_TEST("game.maint.dump.Input:readPascalString:length-missing", a)
{
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(afl::base::ConstBytes_t(), cs);
    a.checkEqual("result", testee.readPascalString(), "<missing>");
}

/* readPascalString, payload missing */
AFL_TEST("game.maint.dump.Input:readPascalString:data-missing", a)
{
    static const uint8_t BYTES[] = { 3, 97, 98 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);
    a.checkEqual("result", testee.readPascalString(), "<missing>");
}

/* readUnparsed */
AFL_TEST("game.maint.dump.Input:readUnparsed", a)
{
    static const uint8_t BYTES[] = { 1, 2, 3, 4, 5, 6 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);
    a.checkEqual("result (full)",    testee.readUnparsed(4), "01 02 03 04");
    a.checkEqual("avail (first)",    testee.getRemainingSize(), 2U);
    a.checkEqual("result (partial)", testee.readUnparsed(4), "05 06");
    a.checkEqual("avail (second)",   testee.getRemainingSize(), 0U);
    a.checkEqual("result (empty)",   testee.readUnparsed(4), "");
    a.checkEqual("avail (third)",    testee.getRemainingSize(), 0U);
}

/* peek/read/skip */
AFL_TEST("game.maint.dump.Input:read", a)
{
    static const uint8_t BYTES[] = { 1, 2, 3, 4, 5, 6 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);

    uint8_t tmp[4] = {9,9,9,9};
    a.checkEqual("peek first",     testee.peek(tmp), 4U);
    a.checkEqual("peek result 0",  tmp[0], 1U);
    a.checkEqual("peek result 1",  tmp[1], 2U);
    a.checkEqual("peek result 2",  tmp[2], 3U);
    a.checkEqual("peek result 3",  tmp[3], 4U);
    a.checkEqual("avail (first)",  testee.getRemainingSize(), 6U);

    uint8_t tmp2[3] = {8,8,8};
    a.checkEqual("read first",     testee.read(tmp2), 3U);
    a.checkEqual("read result 0",  tmp2[0], 1U);
    a.checkEqual("read result 1",  tmp2[1], 2U);
    a.checkEqual("read result 2",  tmp2[2], 3U);
    a.checkEqual("avail (second)", testee.getRemainingSize(), 3U);

    a.checkEqual("skip",           testee.skip(2), 2U);
    a.checkEqual("avail (third)",  testee.getRemainingSize(), 1U);
    a.checkEqual("readByte",       testee.readByte(), "6");

    a.checkEqual("peek empty", testee.peek(tmp), 0U);
    a.checkEqual("read empty", testee.read(tmp), 0U);
    a.checkEqual("skip empty", testee.skip(99), 0U);
}

/* split */
AFL_TEST("game.maint.dump.Input:split-constructor", a)
{
    static const uint8_t BYTES[] = { 1, 2, 3, 4, 5, 6 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input first(BYTES, cs);
    game::maint::dump::Input second(first, 4);

    a.checkEqual("first size", first.getRemainingSize(), 2U);
    a.checkEqual("first byte", first.readByte(), "5");
    a.checkEqual("second size", second.getRemainingSize(), 4U);
    a.checkEqual("second byte", second.readByte(), "1");
}

/* dumpRemainder */
AFL_TEST("game.maint.dump.Input:dumpRemainder", a)
{
    class Mock : public game::maint::dump::Output {
     public:
        Mock(String_t& result)
            : m_result(result)
            { }
        virtual void startRecord(String_t /*header*/)
            { m_result += "startRecord\n"; }
        virtual void addField(String_t /*name*/, String_t /*formattedValue*/)
            { m_result += "addField\n"; }
        virtual void addUnparsedData(String_t formattedValue)
            {
                m_result += "addUnparsedData:";
                m_result += formattedValue;
                m_result += "\n";
            }
        virtual void endRecord()
            { m_result += "endRecord\n"; }
     private:
        String_t& m_result;
    };

    String_t result;
    Mock out(result);

    static const uint8_t BYTES[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
    afl::charset::Utf8Charset cs;
    game::maint::dump::Input testee(BYTES, cs);
    testee.dumpRemainder(out);

    a.checkEqual("result", result,
                 "addUnparsedData:01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10\n"
                 "addUnparsedData:11 12 13\n");
}
