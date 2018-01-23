/**
  *  \file u/t_interpreter_exporter_textexporter.cpp
  *  \brief Test for interpreter::exporter::TextExporter
  */

#include "interpreter/exporter/textexporter.hpp"

#include "t_interpreter_exporter.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/textfile.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"

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
        list.addList("a@5,b@10,c@5");

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
void
TestInterpreterExporterTextExporter::testEmpty()
{
    TS_ASSERT_EQUALS(renderEmpty(false),
                     "      LEFT RIGHT\n");
}

/** Test table with content. */
void
TestInterpreterExporterTextExporter::testSimple()
{
    TS_ASSERT_EQUALS(renderNormal(false),
                     "    A B              C\n"
                     "----------------------\n"
                     "   10 hi            -7\n"
                     "   10 this is re 11111\n");
}

/** Test long table with content. */
void
TestInterpreterExporterTextExporter::testSimpleLong()
{
    TS_ASSERT_EQUALS(renderLong(false),
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
void
TestInterpreterExporterTextExporter::testEmptyBox()
{
    TS_ASSERT_EQUALS(renderEmpty(true),
                     "|       LEFT | RIGHT                          |\n"
                     "-----------------------------------------------\n");
}

void
TestInterpreterExporterTextExporter::testBox()
{
    TS_ASSERT_EQUALS(renderNormal(true),
                     "|     A | B          |     C |\n"
                     "------------------------------\n"
                     "|    10 | hi         |    -7 |\n"
                     "|    10 | this is re | 11111 |\n"
                     "------------------------------\n");
}

void
TestInterpreterExporterTextExporter::testLongBox()
{
    TS_ASSERT_EQUALS(renderLong(true),
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

