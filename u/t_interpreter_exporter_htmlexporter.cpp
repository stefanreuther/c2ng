/**
  *  \file u/t_interpreter_exporter_htmlexporter.cpp
  *  \brief Test for interpreter::exporter::HtmlExporter
  */

#include "interpreter/exporter/htmlexporter.hpp"

#include "t_interpreter_exporter.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/textfile.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"

using afl::data::IntegerValue;
using afl::data::StringValue;

/** Simple test with values known to possibly cause trouble. */
void
TestInterpreterExporterHtmlExporter::testIt()
{
    // Prepare a field list
    interpreter::exporter::FieldList list;
    list.addList("left,right");

    // Output receiver
    afl::io::InternalStream outputStream;
    afl::io::TextFile outputText(outputStream);
    outputText.setSystemNewline(false);

    // Testee
    interpreter::exporter::HtmlExporter testee(outputText);
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
        StringValue sv("<x & y>");
        testee.addField(&iv, "left", interpreter::thInt);
        testee.addField(&sv, "right", interpreter::thString);
    }
    testee.endRecord();

    testee.startRecord();
    {
        IntegerValue iv(3);
        StringValue sv("\xC3\xBC""nic\xC3\xB6""de");
        testee.addField(&iv, "left", interpreter::thInt);
        testee.addField(&sv, "right", interpreter::thString);
    }
    testee.endRecord();
    testee.endTable();

    // Verify
    outputText.flush();

    TS_ASSERT_EQUALS(afl::string::fromBytes(outputStream.getContent()),
                     "<!DOCTYPE html>\n"
                     "<html>\n"
                     " <head>\n"
                     "  <title>PCC2 export</title>\n"
                     " </head>\n"
                     " <body>\n"
                     "  <table>\n"
                     "   <tr>\n"
                     "    <th>LEFT</th>\n"
                     "    <th>RIGHT</th>\n"
                     "   </tr>\n"
                     "   <tr>\n"
                     "    <td>1</td>\n"
                     "    <td>a</td>\n"
                     "   </tr>\n"
                     "   <tr>\n"
                     "    <td>2</td>\n"
                     "    <td>&lt;x &amp; y&gt;</td>\n"
                     "   </tr>\n"
                     "   <tr>\n"
                     "    <td>3</td>\n"
                     "    <td>&#252;nic&#246;de</td>\n"
                     "   </tr>\n"
                     "  </table>\n"
                     " </body>\n"
                     "</html>\n");
}

