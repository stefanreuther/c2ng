/**
  *  \file test/interpreter/exporter/separatedtextexportertest.cpp
  *  \brief Test for interpreter::exporter::SeparatedTextExporter
  */

#include "interpreter/exporter/separatedtextexporter.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/exporter/fieldlist.hpp"

using afl::data::IntegerValue;
using afl::data::StringValue;

/** Simple test with values known to possibly cause trouble. */
AFL_TEST("interpreter.exporter.SeparatedTextExporter", a)
{
    // Prepare a field list
    interpreter::exporter::FieldList list;
    list.addList("left,right");

    // Output receiver
    afl::io::InternalStream outputStream;
    afl::io::TextFile outputText(outputStream);
    outputText.setSystemNewline(false);

    // Testee
    interpreter::exporter::SeparatedTextExporter testee(outputText, ',');
    static const interpreter::TypeHint hints[] = { interpreter::thInt, interpreter::thString };

    // Test sequence
    testee.startTable(list, hints);
    testee.startRecord();
    {
        IntegerValue iv(1);
        StringValue sv("a");
        testee.addField(&iv, "left", interpreter::thInt);
        testee.addField(&sv, "right", interpreter::thString);
    }
    testee.endRecord();

    testee.startRecord();
    {
        IntegerValue iv(2);
        StringValue sv("a,b,c");
        testee.addField(&iv, "left", interpreter::thInt);
        testee.addField(&sv, "right", interpreter::thString);
    }
    testee.endRecord();

    testee.startRecord();
    {
        IntegerValue iv(3);
        StringValue sv("Say \"Hi\"!");
        testee.addField(&iv, "left", interpreter::thInt);
        testee.addField(&sv, "right", interpreter::thString);
    }
    testee.endRecord();
    testee.endTable();

    testee.startRecord();
    {
        IntegerValue iv(4);
        StringValue sv("Long\nText");
        testee.addField(&iv, "left", interpreter::thInt);
        testee.addField(&sv, "right", interpreter::thString);
    }
    testee.endRecord();
    testee.endTable();

    // Verify
    outputText.flush();

    a.checkEqual("result", afl::string::fromBytes(outputStream.getContent()),
                 "\"LEFT\",\"RIGHT\"\n"
                 "1,a\n"
                 "2,\"a,b,c\"\n"
                 "3,\"Say \"\"Hi\"\"!\"\n"
                 "4,Long...\n");
}
