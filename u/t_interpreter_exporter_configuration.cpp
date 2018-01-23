/**
  *  \file u/t_interpreter_exporter_configuration.cpp
  *  \brief Test for interpreter::exporter::Configuration
  */

#include <memory>
#include "interpreter/exporter/configuration.hpp"

#include "t_interpreter_exporter.hpp"
#include "afl/charset/charset.hpp"

/** Simple test. */
void
TestInterpreterExporterConfiguration::testIt()
{
    interpreter::exporter::Configuration testee;

    // Charset
    testee.setCharsetIndex(util::CharsetFactory::UNICODE_INDEX);
    TS_ASSERT_EQUALS(testee.getCharsetIndex(), util::CharsetFactory::UNICODE_INDEX);

    testee.setCharsetByName("latin1");
    TS_ASSERT_EQUALS(testee.getCharsetIndex(), util::CharsetFactory::LATIN1_INDEX);

    std::auto_ptr<afl::charset::Charset> p(testee.createCharset());
    TS_ASSERT(p.get() != 0);
    TS_ASSERT_EQUALS(p->decode(afl::string::toBytes("\xa5")), "\xc2\xa5");

    TS_ASSERT_THROWS(testee.setCharsetByName("wqielkjsad"), std::exception);

    // Format
    testee.setFormat(interpreter::exporter::CommaSVFormat);
    TS_ASSERT_EQUALS(testee.getFormat(), interpreter::exporter::CommaSVFormat);

    testee.setFormatByName("json");
    TS_ASSERT_EQUALS(testee.getFormat(), interpreter::exporter::JSONFormat);

    TS_ASSERT_THROWS(testee.setFormatByName("wqielkjsad"), std::exception);

    // Field list initially empty
    TS_ASSERT_EQUALS(testee.fieldList().size(), 0U);
}

