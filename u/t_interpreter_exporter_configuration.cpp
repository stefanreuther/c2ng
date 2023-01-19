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
#include "afl/io/internalstream.hpp"
#include "afl/io/internaltextwriter.hpp"
#include "afl/string/nulltranslator.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/simplecontext.hpp"
#include "interpreter/values.hpp"

namespace {
    String_t removeCharacter(String_t s, char ch)
    {
        size_t i = 0;
        while (i < s.size()) {
            if (s[i] == ch) {
                s.erase(i, 1);
            } else {
                ++i;
            }
        }
        return s;
    }

    class TestContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                if (name.match("ID")) {
                    result = 1;
                    return this;
                } else if (name.match("NAME")) {
                    result = 2;
                    return this;
                } else {
                    return 0;
                }
            }
        virtual afl::data::Value* get(PropertyIndex_t index)
            { return index == 1 ? interpreter::makeIntegerValue(42) : interpreter::makeStringValue("Fred"); }
        virtual bool next()
            { return false; }
        virtual TestContext* clone() const
            { return new TestContext(); }
        virtual game::map::Object* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const
            {
                acceptor.addProperty("ID", interpreter::thInt);
                acceptor.addProperty("NAME", interpreter::thString);
            }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "<TestContext>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { }
    };
}


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

/** Test save(). */
void
TestInterpreterExporterConfiguration::testSave()
{
    afl::string::NullTranslator tx;
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("A@10,B,X@-5");
    TS_ASSERT_THROWS_NOTHING(testee.setCharsetByName("cp850", tx));
    testee.setFormat(interpreter::exporter::HTMLFormat);

    afl::io::InternalStream out;
    testee.save(out);

    TS_ASSERT_EQUALS(removeCharacter(afl::string::fromBytes(out.getContent()), '\r'),
                     "Fields=A@10\n"
                     "Fields=B\n"
                     "Fields=X@-5\n"
                     "Charset=cp850\n"
                     "Format=html\n");
}

/** Test exportText(), text file format. */
void
TestInterpreterExporterConfiguration::testText()
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::TextFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    TS_ASSERT_EQUALS(testee.exportText(ctx, out), true);

    TS_ASSERT_EQUALS(afl::string::fromMemory(out.getContent()),
                     "        ID NAME\n"
                     "-----------------------------------------\n"
                     "        42 Fred\n");
}

/** Test exportText(), boxy-table file format. */
void
TestInterpreterExporterConfiguration::testTextTable()
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::TableFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    TS_ASSERT_EQUALS(testee.exportText(ctx, out), true);

    TS_ASSERT_EQUALS(afl::string::fromMemory(out.getContent()),
                     "|         ID | NAME                           |\n"
                     "-----------------------------------------------\n"
                     "|         42 | Fred                           |\n"
                     "-----------------------------------------------\n");
}

/** Test exportText(), comma-separated file format. */
void
TestInterpreterExporterConfiguration::testTextCSV()
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::CommaSVFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    TS_ASSERT_EQUALS(testee.exportText(ctx, out), true);

    TS_ASSERT_EQUALS(afl::string::fromMemory(out.getContent()),
                     "\"ID\",\"NAME\"\n"
                     "42,Fred\n");
}

/** Test exportText(), semicolon-separated file format. */
void
TestInterpreterExporterConfiguration::testTextSSV()
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::SemicolonSVFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    TS_ASSERT_EQUALS(testee.exportText(ctx, out), true);

    TS_ASSERT_EQUALS(afl::string::fromMemory(out.getContent()),
                     "\"ID\";\"NAME\"\n"
                     "42;Fred\n");
}

/** Test exportText(), tab-separated file format. */
void
TestInterpreterExporterConfiguration::testTextTSV()
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::TabSVFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    TS_ASSERT_EQUALS(testee.exportText(ctx, out), true);

    TS_ASSERT_EQUALS(afl::string::fromMemory(out.getContent()),
                     "\"ID\"\t\"NAME\"\n"
                     "42\tFred\n");
}

/** Test exportText(), JSON file format. */
void
TestInterpreterExporterConfiguration::testTextJSON()
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::JSONFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    TS_ASSERT_EQUALS(testee.exportText(ctx, out), true);

    TS_ASSERT_EQUALS(removeCharacter(removeCharacter(afl::string::fromMemory(out.getContent()), '\r'), '\n'),
                     "[{\"ID\":42,\"NAME\":\"Fred\"}]");
}

/** Test exportText(), HTML file format. */
void
TestInterpreterExporterConfiguration::testTextHTML()
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::HTMLFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    TS_ASSERT_EQUALS(testee.exportText(ctx, out), true);

    TS_ASSERT_EQUALS(afl::string::fromMemory(out.getContent()),
                     "<!DOCTYPE html>\n"
                     "<html>\n"
                     " <head>\n"
                     "  <title>PCC2 export</title>\n"
                     " </head>\n"
                     " <body>\n"
                     "  <table>\n"
                     "   <tr>\n"
                     "    <th>ID</th>\n"
                     "    <th>NAME</th>\n"
                     "   </tr>\n"
                     "   <tr>\n"
                     "    <td>42</td>\n"
                     "    <td>Fred</td>\n"
                     "   </tr>\n"
                     "  </table>\n"
                     " </body>\n"
                     "</html>\n"
                     "");
}

/** Test exportText(), DBF file format.
    This fails. */
void
TestInterpreterExporterConfiguration::testTextDBF()
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::DBaseFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    TS_ASSERT_EQUALS(testee.exportText(ctx, out), false);

    TS_ASSERT_EQUALS(out.getContent().size(), 0U);
}

/** Test exportFile(), JSON (as specimen for text).
    Since the text file will have a system-specific newline format,
    using JSON works well here because we strip its newlines for checking, anyway. */
void
TestInterpreterExporterConfiguration::testFileJSON()
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::JSONFormat);

    TestContext ctx;
    afl::io::InternalStream out;
    testee.exportFile(ctx, out);

    TS_ASSERT_EQUALS(removeCharacter(removeCharacter(afl::string::fromBytes(out.getContent()), '\r'), '\n'),
                     "[{\"ID\":42,\"NAME\":\"Fred\"}]");
}

/** Test exportFile(), DBF format. */
void
TestInterpreterExporterConfiguration::testFileDBF()
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::DBaseFormat);

    TestContext ctx;
    afl::io::InternalStream out;
    testee.exportFile(ctx, out);

    static const uint8_t DATA[139] = {
        0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x61, 0x00, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x49, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x00, 0x00,
        0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x4E, 0x41, 0x4D, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x00,
        0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x0D, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x34, 0x32, 0x46, 0x72, 0x65, 0x64,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00,
    };
    TS_ASSERT_EQUALS(out.getContent().size(), sizeof(DATA));
    TS_ASSERT_SAME_DATA(out.getContent().unsafeData(), DATA, sizeof(DATA));
}

