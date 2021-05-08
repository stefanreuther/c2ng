/**
  *  \file u/t_interpreter_exporter_configuration.cpp
  *  \brief Test for interpreter::exporter::Configuration
  */

#include <memory>
#include "interpreter/exporter/configuration.hpp"

#include "t_interpreter_exporter.hpp"
#include "afl/charset/charset.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"

/** Simple test. */
void
TestInterpreterExporterConfiguration::testIt()
{
    interpreter::exporter::Configuration testee;
    afl::string::NullTranslator tx;

    // Charset
    testee.setCharsetIndex(util::CharsetFactory::UNICODE_INDEX);
    TS_ASSERT_EQUALS(testee.getCharsetIndex(), util::CharsetFactory::UNICODE_INDEX);

    testee.setCharsetByName("latin1", tx);
    TS_ASSERT_EQUALS(testee.getCharsetIndex(), util::CharsetFactory::LATIN1_INDEX);

    std::auto_ptr<afl::charset::Charset> p(testee.createCharset());
    TS_ASSERT(p.get() != 0);
    TS_ASSERT_EQUALS(p->decode(afl::string::toBytes("\xa5")), "\xc2\xa5");

    TS_ASSERT_THROWS(testee.setCharsetByName("wqielkjsad", tx), std::exception);

    // Format
    testee.setFormat(interpreter::exporter::CommaSVFormat);
    TS_ASSERT_EQUALS(testee.getFormat(), interpreter::exporter::CommaSVFormat);

    testee.setFormatByName("json", tx);
    TS_ASSERT_EQUALS(testee.getFormat(), interpreter::exporter::JSONFormat);

    TS_ASSERT_THROWS(testee.setFormatByName("wqielkjsad", tx), std::exception);

    // Field list initially empty
    TS_ASSERT_EQUALS(testee.fieldList().size(), 0U);

    // Constness (coverage)
    TS_ASSERT_EQUALS(&testee.fieldList(), &const_cast<const interpreter::exporter::Configuration&>(testee).fieldList());

    // Copying (coverage)
    interpreter::exporter::Configuration copy(testee);
    TS_ASSERT_EQUALS(copy.getCharsetIndex(), testee.getCharsetIndex());
    testee.setCharsetByName("cp437", tx);
    TS_ASSERT_DIFFERS(copy.getCharsetIndex(), testee.getCharsetIndex());

    copy = testee;
    TS_ASSERT_EQUALS(copy.getCharsetIndex(), testee.getCharsetIndex());
}

/** Test load(). */
void
TestInterpreterExporterConfiguration::testLoad()
{
    afl::string::NullTranslator tx;

    // Good case
    {
        interpreter::exporter::Configuration testee;
        afl::io::ConstMemoryStream stream(afl::string::toBytes("# config\n"
                                                               "fields = a,b,c\n"
                                                               "format = dbf\n"
                                                               "ignore = me\n"
                                                               "charset = koi8-r\n"));
        testee.load(stream, tx);

        TS_ASSERT_EQUALS(testee.getFormat(), interpreter::exporter::DBaseFormat);
        TS_ASSERT_EQUALS(testee.fieldList().toString(), "A,B,C");

        std::auto_ptr<afl::charset::Charset> p(testee.createCharset());
        TS_ASSERT(p.get() != 0);
        TS_ASSERT_EQUALS(p->decode(afl::string::toBytes("\xc1")), "\xD0\xB0");  // U+0430, cyrillic 'a'
    }

    // Bad case - syntax error on ConfigurationFileParser
    {
        interpreter::exporter::Configuration testee;
        afl::io::ConstMemoryStream stream(afl::string::toBytes("; syntax error"));
        TS_ASSERT_THROWS(testee.load(stream, tx), afl::except::FileProblemException);
    }

    // Bad case - syntax error in fields
    {
        interpreter::exporter::Configuration testee;
        afl::io::ConstMemoryStream stream(afl::string::toBytes("fields = -1@x"));
        TS_ASSERT_THROWS(testee.load(stream, tx), afl::except::FileProblemException);
    }
}

