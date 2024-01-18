/**
  *  \file test/game/v3/udata/messagebuildertest.cpp
  *  \brief Test for game::v3::udata::MessageBuilder
  */

#include "game/v3/udata/messagebuilder.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/v3/udata/nameprovider.hpp"

namespace {
    /*
     *  NameProvider implementation
     */
    class TestNameProvider : public game::v3::udata::NameProvider {
     public:
        virtual String_t getName(Type type, int id) const
            {
                switch (type) {
                 case HullFunctionName:     return afl::string::Format("func%d", id);
                 case HullName:             return afl::string::Format("hull%d", id);
                 case NativeGovernmentName: return afl::string::Format("gov%d", id);
                 case NativeRaceName:       return afl::string::Format("race%d", id);
                 case PlanetName:           return afl::string::Format("planet%d", id);
                 case ShortRaceName:        return afl::string::Format("player%d", id);
                }
                return "badName";
            }
    };

    /*
     *  Test environment
     */
    struct Environment {
        Environment()
            : provider(), charset(afl::charset::g_codepage437), tx(),
              builder(provider, charset, tx),
              mbox()
            { }

        void load(afl::base::ConstBytes_t file, const char* spec)
            {
                afl::sys::Log log;
                afl::io::ConstMemoryStream specStream(afl::string::toBytes(spec));
                builder.loadDefinition(specStream, log);

                afl::io::ConstMemoryStream fileStream(file);
                builder.loadFile(fileStream, mbox);
            }

        size_t getNumMessages()
            { return mbox.getNumMessages(); }

        String_t getMessageText(size_t index)
            {
                game::PlayerList players;
                return mbox.getMessageText(index, tx, players);
            }

        int getMessageTurnNumber(size_t index)
            {
                game::PlayerList players;
                return mbox.getMessageMetadata(index, tx, players).turnNumber;
            }

        TestNameProvider provider;
        afl::charset::CodepageCharset charset;
        afl::string::NullTranslator tx;
        game::v3::udata::MessageBuilder builder;
        game::msg::Inbox mbox;
    };
}


/** Normal, broad usage test.
    Tests decoding of an actual util.dat record against the actual definition for it. */
AFL_TEST("game.v3.udata.MessageBuilder:normal", a)
{
    static const uint8_t FILE[] = {
        0x0d, 0x00, 0x59, 0x00, 0x30, 0x33, 0x2d, 0x30, 0x31, 0x2d, 0x32, 0x30, 0x31, 0x38, 0x32, 0x30,
        0x3a, 0x30, 0x30, 0x3a, 0x30, 0x32, 0x1e, 0x00, 0x06, 0x00, 0x04, 0x01, 0x23, 0xcd, 0x28, 0x9d,
        0x22, 0xc6, 0x2a, 0x0e, 0x66, 0x1c, 0xf0, 0x1d, 0x8d, 0x2a, 0xde, 0x4a, 0xb7, 0x62, 0x36, 0x6a,
        0x18, 0x97, 0xa2, 0xb2, 0x6e, 0x3f, 0x0e, 0xae, 0xd3, 0xab, 0xdf, 0x91, 0x4e, 0x6f, 0x72, 0x74,
        0x68, 0x20, 0x53, 0x74, 0x61, 0x72, 0x20, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68,
    };

    const char*const SPEC =
        "; comment\n"
        "13,Control Record\n"
        "        h = (-h0000)\n"
        "        t = Turn %18w for player %20w\n"
        "        t =\n"
        "        t = Host Time: %0S10 at %10S08\n"
        "        t = Version:   PHost %22b.%23b%88?S01\n"
        "        t = Game Name: %56S32\n"
        "        t =\n"
        "        t = Host file digests:\n"
        "        t = %|HUL=%24X, ENG=%X,\n"
        "        t = %|BEA=%X, TOR=%X,\n"
        "        t = %|TRU=%X, PXY=%X,\n"
        "        t = %|CFG=%X, NAM=%X.\n";

    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Control Record >>>\n"
                 "\n"
                 "Record type 13, 89 bytes\n"
                 "\n"
                 "Turn 30 for player 6\n"
                 "\n"
                 "Host Time: 03-01-2018 at 20:00:02\n"
                 "Version:   PHost 4.1h\n"
                 "Game Name: North Star 4\n"
                 "\n"
                 "Host file digests:\n"
                 "  HUL=9D28CD23, ENG=0E2AC622,\n"
                 "  BEA=1DF01C66, TOR=4ADE2A8D,\n"
                 "  TRU=6A3662B7, PXY=B2A29718,\n"
                 "  CFG=AE0E3F6E, NAM=91DFABD3.\n");
    a.checkEqual("03. getMessageTurnNumber", env.getMessageTurnNumber(0), 30);
}

/** Test undefined type. */
AFL_TEST("game.v3.udata.MessageBuilder:undefined", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x05, 0x00, };
    const char*const SPEC = "";

    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Unknown >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Unknown record type.");
    a.checkEqual("03. getMessageTurnNumber", env.getMessageTurnNumber(0), 0);
}

/** Test aliased type.
    Uses text from alias target, but title/header from original definition. */
AFL_TEST("game.v3.udata.MessageBuilder:alias", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x05, 0x00, };
    const char*const SPEC =
        "16,Link\n"
        "a=17\n"
        "h = (Y)\n"
        "17,Target\n"
        "t = Value %d\n"
        "t = End\n"
        "h = (X)\n";

    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(Y)<<< Link >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value 5\n"
                 "End\n");
}

/** Test bad alias (undefined target). */
AFL_TEST("game.v3.udata.MessageBuilder:alias:bad-link", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x05, 0x00, };
    const char*const SPEC =
        "16,Link\n"
        "a=17\n";

    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Link >>>\n"
                 "\n"
                 "Record type 16, 2 bytes\n"
                 "\n"
                 "Unknown reference target in record definition.");
}

/** Test alias loop.
    Loop must be broken. */
AFL_TEST("game.v3.udata.MessageBuilder:alias:loop", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x05, 0x00, };
    const char*const SPEC =
        "16,Loop\n"
        "a=17\n"
        "17,Infinite\n"
        "a=17\n";

    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Loop >>>\n"
                 "\n"
                 "Record type 16, 2 bytes\n\n");
}

/** Test loop in content.
    If l= is given, the record is broken into multiple parts that are individually formatted. */
AFL_TEST("game.v3.udata.MessageBuilder:content-loop", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x08, 0x00,
                                    0x05, 0x00, 0x07, 0x00, 0x20, 0x00, 0x30, 0x00, };
    const char*const SPEC =
        "16,Loop\n"
        "f = 4\n"
        "l = 2\n"
        "t = fixed %d %d\n"
        "t = looped %d\n";

    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 2U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Loop >>>\n"
                 "\n"
                 "Record type 16, part 1\n"
                 "\n"
                 "fixed 5 7\n"
                 "looped 32\n");
    a.checkEqual("03. getMessageText", env.getMessageText(1),
                 "(-h0000)<<< Loop >>>\n"
                 "\n"
                 "Record type 16, part 2\n"
                 "\n"
                 "fixed 5 7\n"
                 "looped 48\n");
}

/** Test format code 'S': string. */
AFL_TEST("game.v3.udata.MessageBuilder:format:S", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x08, 0x00, 'T', 0x89, 'x', 't', 0x20, 0x20, 0x20, 0x20 };
    const char*const SPEC =
        "16,String\n"
        "t = Text '%S08'\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< String >>>\n\n"
                 "Record type 16, 8 bytes\n\n"
                 "Text 'T\xC3\xABxt'\n"
                 "End\n");
}

/** Test format code 'S': string, missing/incomplete data. */
AFL_TEST("game.v3.udata.MessageBuilder:format:S:partial", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x07, 0x00, 'T', 0x89, 'x', 't', 0x20, 0x20, 0x20 };
    const char*const SPEC =
        "16,String\n"
        "t = Text '%S08'\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< String >>>\n\n"
                 "Record type 16, 7 bytes\n\n"
                 "End\n");
}

/** Test format code 'X': 32-bit hex. */
AFL_TEST("game.v3.udata.MessageBuilder:format:X", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x04, 0x00, 0x12, 0x34, 0x56, 0x78 };
    const char*const SPEC =
        "16,Hex\n"
        "t = Value %X\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Hex >>>\n\n"
                 "Record type 16, 4 bytes\n\n"
                 "Value 78563412\n"
                 "End\n");
}

/** Test format code 'l': 32-bit decimal. */
AFL_TEST("game.v3.udata.MessageBuilder:format:l", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x04, 0x00, 0x12, 0x34, 0x56, 0x78 };
    const char*const SPEC =
        "16,Long\n"
        "t = Value %l\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Long >>>\n\n"
                 "Record type 16, 4 bytes\n\n"
                 "Value 2018915346\n"
                 "End\n");
}

/** Test format code 'F': 32-bit fixed-point. */
AFL_TEST("game.v3.udata.MessageBuilder:format:F", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x04, 0x00, 0x12, 0x34, 0x56, 0x78 };
    const char*const SPEC =
        "16,Fixed\n"
        "t = Value %F\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Fixed >>>\n\n"
                 "Record type 16, 4 bytes\n\n"
                 "Value 2018915.346\n"
                 "End\n");
}

/** Test format code 'F': 32-bit fixed-point, negative value. */
AFL_TEST("game.v3.udata.MessageBuilder:format:F:negative", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x04, 0x00, 0xFE, 0xFF, 0xFF, 0xFF };
    const char*const SPEC =
        "16,Fixed\n"
        "t = Value %F\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Fixed >>>\n\n"
                 "Record type 16, 4 bytes\n\n"
                 "Value -0.002\n"
                 "End\n");
}

/** Test format code 'l': 32-bit decimal, missing value. */
AFL_TEST("game.v3.udata.MessageBuilder:format:l:missing", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x12, 0x34 };
    const char*const SPEC =
        "16,Long\n"
        "t = Value %l\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Long >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "End\n");
}

/** Test format code 'b': byte. */
AFL_TEST("game.v3.udata.MessageBuilder:format:b", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x01, 0x00, 0x05 };
    const char*const SPEC =
        "16,Byte\n"
        "t = Value %b\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Byte >>>\n\n"
                 "Record type 16, 1 byte\n\n"
                 "Value 5\n"
                 "End\n");
}

/** Test format code 'b': byte, missing value. */
AFL_TEST("game.v3.udata.MessageBuilder:format:b:missing", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x00, 0x00 };
    const char*const SPEC =
        "16,Byte\n"
        "t = Value %b\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Byte >>>\n\n"
                 "Record type 16, 0 bytes\n\n"
                 "End\n");
}

/** Test format code '%': literal (no conversion). */
AFL_TEST("game.v3.udata.MessageBuilder:format:percent", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x00, 0x00 };
    const char*const SPEC =
        "16,Text\n"
        "t = 100%%\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Text >>>\n\n"
                 "Record type 16, 0 bytes\n\n"
                 "100%\n"
                 "End\n");
}

/** Test format code '|': literal (no conversion).
    '%|' provides a space that is not trimmed, for indentation. */
AFL_TEST("game.v3.udata.MessageBuilder:format:space", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x00, 0x00 };
    const char*const SPEC =
        "16,Text\n"
        "t = %|a\n"
        "t =   b\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Text >>>\n\n"
                 "Record type 16, 0 bytes\n\n"
                 "  a\n"
                 "b\n"
                 "End\n");
}

/** Test format code 'g': 16-bit, government name. */
AFL_TEST("game.v3.udata.MessageBuilder:format:g", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x03, 0x00 };
    const char*const SPEC =
        "16,Government\n"
        "t = Value %g\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Government >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value gov3\n"
                 "End\n");
}

/** Test format code 'h': 16-bit, hull name. */
AFL_TEST("game.v3.udata.MessageBuilder:format:h", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x03, 0x00 };
    const char*const SPEC =
        "16,Hull\n"
        "t = Value %h\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Hull >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value hull3\n"
                 "End\n");
}

/** Test format code 'H': 16-bit, hull function name. */
AFL_TEST("game.v3.udata.MessageBuilder:format:H", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x03, 0x00 };
    const char*const SPEC =
        "16,Hullfunc\n"
        "t = Value %H\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Hullfunc >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value func3\n"
                 "End\n");
}

/** Test format code 'n': 16-bit, native race name. */
AFL_TEST("game.v3.udata.MessageBuilder:format:n", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x07, 0x00 };
    const char*const SPEC =
        "16,Native Race\n"
        "t = Value %n\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Native Race >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value race7\n"
                 "End\n");
}

/** Test format code 'B': 16-bit, bit set. */
AFL_TEST("game.v3.udata.MessageBuilder:format:B", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x05, 0x80 };
    const char*const SPEC =
        "16,Bits\n"
        "t = Value %B\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Bits >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value 0 2 15\n"
                 "End\n");
}

/** Test format code 'B': 16-bit, bit set, special case: no bits set. */
AFL_TEST("game.v3.udata.MessageBuilder:format:B:empty", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x00, 0x00 };
    const char*const SPEC =
        "16,Bits\n"
        "t = Value %B\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Bits >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value none\n"
                 "End\n");
}

/** Test format code 'd': 16-bit, decimal. */
AFL_TEST("game.v3.udata.MessageBuilder:format:d", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0xF4, 0x01 };
    const char*const SPEC =
        "16,Decimal\n"
        "t = Value %d\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Decimal >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value 500\n"
                 "End\n");
}

/** Test format code 'd': 16-bit, decimal, missing value. */
AFL_TEST("game.v3.udata.MessageBuilder:format:d:missing", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x01, 0x00, 0xF4 };
    const char*const SPEC =
        "16,Decimal\n"
        "t = Value %d\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Decimal >>>\n\n"
                 "Record type 16, 1 byte\n\n"
                 "End\n");
}

/** Test format code 'p': 16-bit, planet name. */
AFL_TEST("game.v3.udata.MessageBuilder:format:p", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x04, 0x01 };
    const char*const SPEC =
        "16,Planet\n"
        "t = Value %p\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Planet >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value planet260\n"
                 "End\n");
}

/** Test format code 'r': 16-bit, player name. */
AFL_TEST("game.v3.udata.MessageBuilder:format:r", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x04, 0x00 };
    const char*const SPEC =
        "16,Player\n"
        "t = Value %r\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Player >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value player4\n"
                 "End\n");
}

/** Test format code 'u': 16-bit, record type name. */
AFL_TEST("game.v3.udata.MessageBuilder:format:u", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x04, 0x00 };
    const char*const SPEC =
        "16,Util\n"
        "t = Value %u\n"
        "t = End\n"
        "4,Target\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Util >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value Target\n"
                 "End\n");
}

/** Test format code 'r': 16-bit, record type name, nonexistant name. */
AFL_TEST("game.v3.udata.MessageBuilder:format:u:missing", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x04, 0x00 };
    const char*const SPEC =
        "16,Util\n"
        "t = Value %u\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Util >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value 4\n"
                 "End\n");
}

/** Test format code 'W': 16-bit, formatted to 4 digits (primarily for headers). */
AFL_TEST("game.v3.udata.MessageBuilder:format:W", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x07, 0x00 };
    const char*const SPEC =
        "16,Word\n"
        "t = Value %W\n"
        "t = End\n"
        "h = (-q%W)";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-q0007)<<< Word >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value 0007\n"
                 "End\n");
}

/** Test format code 'R': 16-bit, right-justified decimal. */
AFL_TEST("game.v3.udata.MessageBuilder:format:R", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x07, 0x00 };
    const char*const SPEC =
        "16,Right\n"
        "t = Value %R\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Right >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value     7\n"
                 "End\n");
}

/** Test format code 'x': 16-bit, hex. */
AFL_TEST("game.v3.udata.MessageBuilder:format:x", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x07, 0x89 };
    const char*const SPEC =
        "16,Hex\n"
        "t = Value %x\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Hex >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value 8907\n"
                 "End\n");
}

/** Test format code '(...)': 16-bit, enum. */
AFL_TEST("game.v3.udata.MessageBuilder:format:enum", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x03, 0x00 };
    const char*const SPEC =
        "16,Enum\n"
        "t = Value %(zero,one,two,\n"
        "t = three,four)!\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Enum >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value three!\n"
                 "End\n");
}

/** Test format code '(...)': 16-bit, enum, value not present in list. */
AFL_TEST("game.v3.udata.MessageBuilder:format:enum:mismatch", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x02, 0x00, 0x77, 0x00 };
    const char*const SPEC =
        "16,Enum\n"
        "t = Value %(zero,one,two,\n"
        "t = three,four)!\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Enum >>>\n\n"
                 "Record type 16, 2 bytes\n\n"
                 "Value 119!\n"
                 "End\n");
}

/** Test handling unset value, value not present
    Default is to ignore a line with unset values. */
AFL_TEST("game.v3.udata.MessageBuilder:format:missing-vaule", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x06, 0x00, 1,0, 2,0, 3,0 };
    const char*const SPEC =
        "16,Empty\n"
        "t = First %d,%d\n"
        "t = Second %d,%d\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Empty >>>\n\n"
                 "Record type 16, 6 bytes\n\n"
                 "First 1,2\n"
                 "End\n");
}

/** Test handling unset value, value present but explicitly unset.
    Default is to ignore a line with unset values. */
AFL_TEST("game.v3.udata.MessageBuilder:format:unset-value", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x08, 0x00, 1,0, 0xFF,0xFF, 3,0, 4,0 };
    const char*const SPEC =
        "16,Empty\n"
        "t = First %d,%d\n"
        "t = Second %d,%d\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Empty >>>\n\n"
                 "Record type 16, 8 bytes\n\n"
                 "Second 3,4\n"
                 "End\n");
}

/** Test handling unset value, value present but explicitly unset, Ids.
    For Ids, 0 counts as empty. */
AFL_TEST("game.v3.udata.MessageBuilder:format:unset-id", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x08, 0x00, 1,0, 0,0, 3,0, 4,0 };
    const char*const SPEC =
        "16,Empty\n"
        "t = First %p,%p\n"
        "t = Second %p,%p\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Empty >>>\n\n"
                 "Record type 16, 8 bytes\n\n"
                 "Second planet3,planet4\n"
                 "End\n");
}

/** Test handling unset value: '!' modifier.
    '!' forces the values to be output. */
AFL_TEST("game.v3.udata.MessageBuilder:format:force-unset", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x06, 0x00, 1,0, 2,0, 3,0 };
    const char*const SPEC =
        "16,Empty\n"
        "t = First %!d,%!d\n"
        "t = Second %!d,%!d\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Empty >>>\n\n"
                 "Record type 16, 6 bytes\n\n"
                 "First 1,2\n"
                 "Second 3,-1\n"
                 "End\n");
}

/** Test handling unset value: '?' modifier.
    '?' hides the value but not the line. */
AFL_TEST("game.v3.udata.MessageBuilder:format:hide-unset", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x06, 0x00, 1,0, 2,0, 3,0 };
    const char*const SPEC =
        "16,Empty\n"
        "t = First %?d,%?d\n"
        "t = Second %?d,%?d\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Empty >>>\n\n"
                 "Record type 16, 6 bytes\n\n"
                 "First 1,2\n"
                 "Second 3,\n"
                 "End\n");
}

/** Test reordering.
    A number before the format character resets the read pointer. */
AFL_TEST("game.v3.udata.MessageBuilder:format:reordering", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x06, 0x00, 1,0, 2,0, 3,0 };
    const char*const SPEC =
        "16,Reorder\n"
        "t = Values %4d,%0d,%d\n"
        "t = End\n";
    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Reorder >>>\n\n"
                 "Record type 16, 6 bytes\n\n"
                 "Values 3,1,2\n"
                 "End\n");
}

/** Test load limit.
    "m=" limits the number of bytes loaded.
    The original size is still shown. */
AFL_TEST("game.v3.udata.MessageBuilder:load-limit", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x0E, 0x00, 1,0, 2,0, 3,0, 4,0, 5,0, 6,0, 7,0 };
    const char*const SPEC =
        "16,Limit\n"
        "m = 10\n"
        "t = one %d\n"
        "t = two %d\n"
        "t = three %d\n"
        "t = four %d\n"
        "t = five %d\n"
        "t = six %d\n"
        "t = seven %d\n";

    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 1U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Limit >>>\n"
                 "\n"
                 "Record type 16, 14 bytes\n"
                 "\n"
                 "one 1\n"
                 "two 2\n"
                 "three 3\n"
                 "four 4\n"
                 "five 5\n");
}

/** Test load limit in combination with looping. */
AFL_TEST("game.v3.udata.MessageBuilder:loop-with-fixed", a)
{
    static const uint8_t FILE[] = { 0x10, 0x00, 0x0E, 0x00, 1,0, 2,0, 3,0, 4,0, 5,0, 6,0, 7,0 };
    const char*const SPEC =
        "16,Limit\n"
        "f = 4\n"
        "l = 2\n"
        "m = 10\n"
        "t = fixed %d %d\n"
        "t = looped %d\n";

    Environment env;
    env.load(FILE, SPEC);
    a.checkEqual("01. getNumMessages", env.getNumMessages(), 3U);
    a.checkEqual("02. getMessageText", env.getMessageText(0),
                 "(-h0000)<<< Limit >>>\n"
                 "\n"
                 "Record type 16, part 1\n"
                 "\n"
                 "fixed 1 2\n"
                 "looped 3\n");
    a.checkEqual("03. getMessageText", env.getMessageText(1),
                 "(-h0000)<<< Limit >>>\n"
                 "\n"
                 "Record type 16, part 2\n"
                 "\n"
                 "fixed 1 2\n"
                 "looped 4\n");
    a.checkEqual("04. getMessageText", env.getMessageText(2),
                 "(-h0000)<<< Limit >>>\n"
                 "\n"
                 "Record type 16, part 3\n"
                 "\n"
                 "fixed 1 2\n"
                 "looped 5\n");
}
