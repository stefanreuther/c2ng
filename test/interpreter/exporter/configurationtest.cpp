/**
  *  \file test/interpreter/exporter/configurationtest.cpp
  *  \brief Test for interpreter::exporter::Configuration
  */

#include "interpreter/exporter/configuration.hpp"

#include <memory>
#include "afl/charset/charset.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/internaltextwriter.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/simplecontext.hpp"
#include "interpreter/values.hpp"
#include "util/io.hpp"

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
        virtual afl::base::Deletable* getObject()
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
AFL_TEST("interpreter.exporter.Configuration:basics", a)
{
    interpreter::exporter::Configuration testee;
    afl::string::NullTranslator tx;

    // Charset
    testee.setCharsetIndex(util::CharsetFactory::UNICODE_INDEX);
    a.checkEqual("01. getCharsetIndex", testee.getCharsetIndex(), util::CharsetFactory::UNICODE_INDEX);

    testee.setCharsetByName("latin1", tx);
    a.checkEqual("11. getCharsetIndex", testee.getCharsetIndex(), util::CharsetFactory::LATIN1_INDEX);

    std::auto_ptr<afl::charset::Charset> p(testee.createCharset());
    a.checkNonNull("21. createCharset", p.get());
    a.checkEqual("22. charset", p->decode(afl::string::toBytes("\xa5")), "\xc2\xa5");

    AFL_CHECK_THROWS(a("31. setCharsetByName"), testee.setCharsetByName("wqielkjsad", tx), std::exception);

    // Format
    testee.setFormat(interpreter::exporter::CommaSVFormat);
    a.checkEqual("41. getFormat", testee.getFormat(), interpreter::exporter::CommaSVFormat);

    testee.setFormatByName("json", tx);
    a.checkEqual("51. getFormat", testee.getFormat(), interpreter::exporter::JSONFormat);

    AFL_CHECK_THROWS(a("61. setFormatByName"), testee.setFormatByName("wqielkjsad", tx), std::exception);

    // Field list initially empty
    a.checkEqual("71. fieldList", testee.fieldList().size(), 0U);

    // Constness (coverage)
    a.checkEqual("81. fieldList", &testee.fieldList(), &const_cast<const interpreter::exporter::Configuration&>(testee).fieldList());

    // Copying (coverage)
    interpreter::exporter::Configuration copy(testee);
    a.checkEqual("91. getCharsetIndex", copy.getCharsetIndex(), testee.getCharsetIndex());
    testee.setCharsetByName("cp437", tx);
    a.checkDifferent("92. getCharsetIndex", copy.getCharsetIndex(), testee.getCharsetIndex());

    copy = testee;
    a.checkEqual("101. getCharsetIndex", copy.getCharsetIndex(), testee.getCharsetIndex());
}

/*
 *  load()
 */

// Good case
AFL_TEST("interpreter.exporter.Configuration:load:success", a)
{
    afl::string::NullTranslator tx;
    interpreter::exporter::Configuration testee;
    afl::io::ConstMemoryStream stream(afl::string::toBytes("# config\n"
                                                           "fields = a,b,c\n"
                                                           "format = dbf\n"
                                                           "ignore = me\n"
                                                           "charset = koi8-r\n"));
    testee.load(stream, tx);

    a.checkEqual("01. getFormat", testee.getFormat(), interpreter::exporter::DBaseFormat);
    a.checkEqual("02. fieldList", testee.fieldList().toString(), "A,B,C");

    std::auto_ptr<afl::charset::Charset> p(testee.createCharset());
    a.checkNonNull("11. createCharset", p.get());
    a.checkEqual("12. charset", p->decode(afl::string::toBytes("\xc1")), "\xD0\xB0");  // U+0430, cyrillic 'a'
}

// Bad case - syntax error on ConfigurationFileParser
AFL_TEST("interpreter.exporter.Configuration:error:file-syntax", a)
{
    afl::string::NullTranslator tx;
    interpreter::exporter::Configuration testee;
    afl::io::ConstMemoryStream stream(afl::string::toBytes("; syntax error"));
    AFL_CHECK_THROWS(a, testee.load(stream, tx), afl::except::FileProblemException);
}

// Bad case - syntax error in fields
AFL_TEST("interpreter.exporter.Configuration:error:field-syntax", a)
{
    afl::string::NullTranslator tx;
    interpreter::exporter::Configuration testee;
    afl::io::ConstMemoryStream stream(afl::string::toBytes("fields = -1@x"));
    AFL_CHECK_THROWS(a, testee.load(stream, tx), afl::except::FileProblemException);
}

/*
 *  save()
 */

AFL_TEST("interpreter.exporter.Configuration:save", a)
{
    afl::string::NullTranslator tx;
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("A@10,B,X@-5");
    AFL_CHECK_SUCCEEDS(a("01. setCharsetByName"), testee.setCharsetByName("cp850", tx));
    testee.setFormat(interpreter::exporter::HTMLFormat);

    afl::io::InternalStream out;
    testee.save(out);

    a.checkEqual("11. file content", util::normalizeLinefeeds(out.getContent()),
                 "Fields=A@10\n"
                 "Fields=B\n"
                 "Fields=X@-5\n"
                 "Charset=cp850\n"
                 "Format=html\n");
}

/** Test exportText(), text file format. */
AFL_TEST("interpreter.exporter.Configuration:exportText:TextFormat", a)
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::TextFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    a.checkEqual("01. exportText", testee.exportText(ctx, out), true);

    a.checkEqual("11. file content", afl::string::fromMemory(out.getContent()),
                 "        ID NAME\n"
                 "-----------------------------------------\n"
                 "        42 Fred\n");
}

/** Test exportText(), boxy-table file format. */
AFL_TEST("interpreter.exporter.Configuration:exportText:TableFormat", a)
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::TableFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    a.checkEqual("01. exportText", testee.exportText(ctx, out), true);

    a.checkEqual("11. file content", afl::string::fromMemory(out.getContent()),
                 "|         ID | NAME                           |\n"
                 "-----------------------------------------------\n"
                 "|         42 | Fred                           |\n"
                 "-----------------------------------------------\n");
}

/** Test exportText(), comma-separated file format. */
AFL_TEST("interpreter.exporter.Configuration:exportText:CommaSVFormat", a)
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::CommaSVFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    a.checkEqual("01. exportText", testee.exportText(ctx, out), true);

    a.checkEqual("11. file content", afl::string::fromMemory(out.getContent()),
                 "\"ID\",\"NAME\"\n"
                 "42,Fred\n");
}

/** Test exportText(), semicolon-separated file format. */
AFL_TEST("interpreter.exporter.Configuration:exportText:SemicolonSVFormat", a)
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::SemicolonSVFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    a.checkEqual("01. exportText", testee.exportText(ctx, out), true);

    a.checkEqual("11. file content", afl::string::fromMemory(out.getContent()),
                 "\"ID\";\"NAME\"\n"
                 "42;Fred\n");
}

/** Test exportText(), tab-separated file format. */
AFL_TEST("interpreter.exporter.Configuration:exportText:TabSVFormat", a)
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::TabSVFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    a.checkEqual("01. exportText", testee.exportText(ctx, out), true);

    a.checkEqual("11. file content", afl::string::fromMemory(out.getContent()),
                 "\"ID\"\t\"NAME\"\n"
                 "42\tFred\n");
}

/** Test exportText(), JSON file format. */
AFL_TEST("interpreter.exporter.Configuration:exportText:JSONFormat", a)
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::JSONFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    a.checkEqual("01. exportText", testee.exportText(ctx, out), true);

    a.checkEqual("11", removeCharacter(removeCharacter(afl::string::fromMemory(out.getContent()), '\r'), '\n'),
                 "[{\"ID\":42,\"NAME\":\"Fred\"}]");
}

/** Test exportText(), HTML file format. */
AFL_TEST("interpreter.exporter.Configuration:exportText:HTMLFormat", a)
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::HTMLFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    a.checkEqual("01. exportText", testee.exportText(ctx, out), true);

    a.checkEqual("11. file content", afl::string::fromMemory(out.getContent()),
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
AFL_TEST("interpreter.exporter.Configuration:exportText:DBaseFormat", a)
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::DBaseFormat);

    TestContext ctx;
    afl::io::InternalTextWriter out;
    a.checkEqual("01. exportText", testee.exportText(ctx, out), false);

    a.checkEqual("11. file content", out.getContent().size(), 0U);
}

/** Test exportFile(), JSON (as specimen for text).
    Since the text file will have a system-specific newline format,
    using JSON works well here because we strip its newlines for checking, anyway. */
AFL_TEST("interpreter.exporter.Configuration:exportFile:JSONFormat", a)
{
    interpreter::exporter::Configuration testee;
    testee.fieldList().addList("ID,NAME");
    testee.setFormat(interpreter::exporter::JSONFormat);

    TestContext ctx;
    afl::io::InternalStream out;
    testee.exportFile(ctx, out);

    a.checkEqual("01. file content", removeCharacter(removeCharacter(afl::string::fromBytes(out.getContent()), '\r'), '\n'),
                 "[{\"ID\":42,\"NAME\":\"Fred\"}]");
}

/** Test exportFile(), DBF format. */
AFL_TEST("interpreter.exporter.Configuration:exportFile:DBaseFormat", a)
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
    a.checkEqualContent<uint8_t>("01. file content", out.getContent(), DATA);
}
