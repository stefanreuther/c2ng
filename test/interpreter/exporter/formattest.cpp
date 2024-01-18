/**
  *  \file test/interpreter/exporter/formattest.cpp
  *  \brief Test for interpreter::exporter::Format
  */

#include "interpreter/exporter/format.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("interpreter.exporter.Format", a)
{
    // toString. Test all values to catch the table disagreeing with the enum
    a.checkEqual("01. toString", toString(interpreter::exporter::TextFormat),        "text");
    a.checkEqual("02. toString", toString(interpreter::exporter::TableFormat),       "table");
    a.checkEqual("03. toString", toString(interpreter::exporter::CommaSVFormat),     "csv");
    a.checkEqual("04. toString", toString(interpreter::exporter::TabSVFormat),       "tsv");
    a.checkEqual("05. toString", toString(interpreter::exporter::SemicolonSVFormat), "ssv");
    a.checkEqual("06. toString", toString(interpreter::exporter::JSONFormat),        "json");
    a.checkEqual("07. toString", toString(interpreter::exporter::HTMLFormat),        "html");
    a.checkEqual("08. toString", toString(interpreter::exporter::DBaseFormat),       "dbf");

    // Extensions
    a.checkEqual("11. getFileNameExtension", getFileNameExtension(interpreter::exporter::HTMLFormat),  "html");
    a.checkEqual("12. getFileNameExtension", getFileNameExtension(interpreter::exporter::JSONFormat),  "js");
    a.checkEqual("13. getFileNameExtension", getFileNameExtension(interpreter::exporter::TableFormat), "txt");

    // Description/iteration
    for (size_t i = 0; i < interpreter::exporter::NUM_FORMATS; ++i) {
        afl::string::NullTranslator tx;
        a.checkDifferent("21. getFormatDescription", getFormatDescription(interpreter::exporter::Format(i), tx), "");
    }

    // parseFormat
    interpreter::exporter::Format fmt;
    a.check("31. parseFormat", parseFormat("dbf", fmt));
    a.checkEqual("32. result", fmt, interpreter::exporter::DBaseFormat);

    a.check("41. parseFormat", parseFormat("CSV", fmt));
    a.checkEqual("42. result", fmt, interpreter::exporter::CommaSVFormat);

    a.check("51", !parseFormat(String_t(), fmt));
    a.checkEqual("52. result", fmt, interpreter::exporter::CommaSVFormat); // unchanged

    a.check("61", !parseFormat("js", fmt));
    a.checkEqual("62. result", fmt, interpreter::exporter::CommaSVFormat); // unchanged
}
