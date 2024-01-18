/**
  *  \file test/interpreter/exporter/textexportertest.cpp
  *  \brief Test for interpreter::exporter::TextExporter
  */

#include "interpreter/exporter/textexporter.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/exporter/fieldlist.hpp"

using afl::data::IntegerValue;
using afl::data::StringValue;

namespace {
    String_t renderEmpty(bool boxes)
    {
        // Prepare a field list
        interpreter::exporter::FieldList list;
        list.addList("left,right");

        // Output receiver
        afl::io::InternalStream outputStream;
        afl::io::TextFile outputText(outputStream);
        outputText.setSystemNewline(false);

        // Testee
        interpreter::exporter::TextExporter testee(outputText, boxes);
        static const interpreter::TypeHint hints[] = { interpreter::thInt, interpreter::thString };

        // Test sequence
        testee.startTable(list, hints);
        testee.endTable();

        // Verify
        outputText.flush();
        return afl::string::fromBytes(outputStream.getContent());
    }

    String_t renderNormal(bool boxes)
    {
        // Prepare a field list
        interpreter::exporter::FieldList list;
        list.addList("a@5,b@-10,c@5");

        // Output receiver
        afl::io::InternalStream outputStream;
        afl::io::TextFile outputText(outputStream);
        outputText.setSystemNewline(false);

        // Testee
        interpreter::exporter::TextExporter testee(outputText, boxes);
        static const interpreter::TypeHint hints[] = { interpreter::thInt, interpreter::thString, interpreter::thInt };

        // Test sequence
        testee.startTable(list, hints);
        testee.startRecord();
        {
            IntegerValue a(10);
            StringValue b("hi");
            IntegerValue c(-7);
            testee.addField(&a, "a", interpreter::thInt);
            testee.addField(&b, "b", interpreter::thString);
            testee.addField(&c, "c", interpreter::thInt);
        }
        testee.endRecord();

        testee.startRecord();
        {
            IntegerValue a(10);
            StringValue b("this is really long text");
            IntegerValue c(111111111);
            testee.addField(&a, "a", interpreter::thInt);
            testee.addField(&b, "b", interpreter::thString);
            testee.addField(&c, "c", interpreter::thInt);
        }
        testee.endRecord();
        testee.endTable();

        // Verify
        outputText.flush();
        return afl::string::fromBytes(outputStream.getContent());
    }

    String_t renderLong(bool boxes)
    {
        // Prepare a field list
        interpreter::exporter::FieldList list;
        list.addList("a@5");

        // Output receiver
        afl::io::InternalStream outputStream;
        afl::io::TextFile outputText(outputStream);
        outputText.setSystemNewline(false);

        // Testee
        interpreter::exporter::TextExporter testee(outputText, boxes);
        static const interpreter::TypeHint hints[] = { interpreter::thInt };

        // Test sequence
        testee.startTable(list, hints);
        for (int i = 0; i < 15; ++i) {
            IntegerValue a(i);
            testee.startRecord();
            testee.addField(&a, "a", interpreter::thInt);
            testee.endRecord();
        }
        testee.endTable();

        // Verify
        outputText.flush();
        return afl::string::fromBytes(outputStream.getContent());
    }
}


/** Test empty table. */
AFL_TEST("interpreter.exporter.TextExporter:table:empty", a)
{
    a.checkEqual("result", renderEmpty(false),
                 "      LEFT RIGHT\n");
}

/** Test table with content. */
AFL_TEST("interpreter.exporter.TextExporter:table:normal", a)
{
    a.checkEqual("result", renderNormal(false),
                 "    A B              C\n"
                 "----------------------\n"
                 "   10 hi            -7\n"
                 "   10 this is re 11111\n");
}

/** Test long table with content. */
AFL_TEST("interpreter.exporter.TextExporter:table:long", a)
{
    a.checkEqual("result", renderLong(false),
                 "    A\n"
                 "-----\n"
                 "    0\n"
                 "    1\n"
                 "    2\n"
                 "    3\n"
                 "    4\n"
                 "    5\n"
                 "    6\n"
                 "    7\n"
                 "    8\n"
                 "    9\n"
                 "   10\n"
                 "   11\n"
                 "   12\n"
                 "   13\n"
                 "   14\n");
}

/** Test empty box. */
AFL_TEST("interpreter.exporter.TextExporter:box:empty", a)
{
    a.checkEqual("result", renderEmpty(true),
                 "|       LEFT | RIGHT                          |\n"
                 "-----------------------------------------------\n");
}

AFL_TEST("interpreter.exporter.TextExporter:box:normal", a)
{
    a.checkEqual("result", renderNormal(true),
                 "|     A | B          |     C |\n"
                 "------------------------------\n"
                 "|    10 | hi         |    -7 |\n"
                 "|    10 | this is re | 11111 |\n"
                 "------------------------------\n");
}

AFL_TEST("interpreter.exporter.TextExporter:box:long", a)
{
    a.checkEqual("result", renderLong(true),
                 "|     A |\n"
                 "---------\n"
                 "|     0 |\n"
                 "|     1 |\n"
                 "|     2 |\n"
                 "|     3 |\n"
                 "|     4 |\n"
                 "|     5 |\n"
                 "|     6 |\n"
                 "|     7 |\n"
                 "|     8 |\n"
                 "|     9 |\n"
                 "---------\n"
                 "|    10 |\n"
                 "|    11 |\n"
                 "|    12 |\n"
                 "|    13 |\n"
                 "|    14 |\n"
                 "---------\n");
}
