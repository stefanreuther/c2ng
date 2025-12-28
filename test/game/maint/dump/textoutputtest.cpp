/**
  *  \file test/game/maint/dump/textoutputtest.cpp
  *  \brief Test for game::maint::dump::TextOutput
  */

#include "game/maint/dump/textoutput.hpp"

#include "afl/io/internalstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/test/testrunner.hpp"
#include "util/io.hpp"

AFL_TEST("game.maint.dump.TextOutput", a)
{
    afl::io::InternalStream out;
    afl::io::TextFile tf(out);
    game::maint::dump::TextOutput testee(tf);

    testee.startRecord("outer");
    testee.addField("firstField", "v1");
    testee.addField(String_t("secondField"), "v2");
    testee.startRecord("inner");
    testee.addField("innerField", "99");
    testee.endRecord();
    testee.endRecord();
    testee.addUnparsedData("12 34 56");
    tf.flush();

    String_t result = util::normalizeLinefeeds(out.getContent());
    String_t expected =
        "outer:\n"
        "  firstField                     = v1\n"
        "  secondField                    = v2\n"
        "  inner:\n"
        "    innerField                     = 99\n"
        "\n"
        "\n"
        "Unparsed = 12 34 56\n";
    a.checkEqual("expected result", result, expected);
}
