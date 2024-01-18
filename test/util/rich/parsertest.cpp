/**
  *  \file test/util/rich/parsertest.cpp
  *  \brief Test for util::rich::Parser
  */

#include "util/rich/parser.hpp"

#include "afl/charset/defaultcharsetfactory.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/xml/defaultentityhandler.hpp"
#include "afl/io/xml/reader.hpp"
#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
#include "util/rich/alignmentattribute.hpp"
#include "util/rich/colorattribute.hpp"
#include "util/rich/linkattribute.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/rich/text.hpp"
#include "util/rich/visitor.hpp"
#include "util/unicodechars.hpp"

namespace {
    class TestVisitor : public util::rich::Visitor {
     public:
        TestVisitor(String_t& result)
            : m_result(result)
            { }
        virtual bool handleText(String_t text)
            { m_result += text; return true; }
        virtual bool startAttribute(const util::rich::Attribute& att)
            {
                if (const util::rich::StyleAttribute* p = dynamic_cast<const util::rich::StyleAttribute*>(&att)) {
                    switch (p->getStyle()) {
                     case util::rich::StyleAttribute::Bold:      m_result += "{BOLD:";      break;
                     case util::rich::StyleAttribute::Italic:    m_result += "{ITALIC:";    break;
                     case util::rich::StyleAttribute::Underline: m_result += "{UNDERLINE:"; break;
                     case util::rich::StyleAttribute::Big:       m_result += "{BIG:";       break;
                     case util::rich::StyleAttribute::Small:     m_result += "{SMALL:";     break;
                     case util::rich::StyleAttribute::Fixed:     m_result += "{FIXED:";     break;
                     case util::rich::StyleAttribute::Key:       m_result += "{KEY:";       break;
                    }
                } else if (const util::rich::ColorAttribute* p = dynamic_cast<const util::rich::ColorAttribute*>(&att)) {
                    switch (p->getColor()) {
                     case util::SkinColor::Static:     m_result += "{STATIC:";     break;
                     case util::SkinColor::Green:      m_result += "{GREEN:";      break;
                     case util::SkinColor::Yellow:     m_result += "{YELLOW:";     break;
                     case util::SkinColor::Red:        m_result += "{RED:";        break;
                     case util::SkinColor::White:      m_result += "{WHITE:";      break;
                     case util::SkinColor::Contrast:   m_result += "{CONTRAST:";   break;
                     case util::SkinColor::Input:      m_result += "{INPUT:";      break;
                     case util::SkinColor::Blue:       m_result += "{BLUE:";       break;
                     case util::SkinColor::Faded:      m_result += "{FADED:";      break;
                     case util::SkinColor::Heading:    m_result += "{HEADING:";    break;
                     case util::SkinColor::Selection:  m_result += "{SELECTION:";  break;
                     case util::SkinColor::InvStatic:  m_result += "{INVSTATIC:";  break;
                     case util::SkinColor::Background: m_result += "{BACKGROUND:"; break;
                     case util::SkinColor::Link:       m_result += "{LINK:";       break;
                     case util::SkinColor::LinkShade:  m_result += "{LINKSHADE:";  break;
                     case util::SkinColor::LinkFocus:  m_result += "{LINKFOCUS:";  break;
                    }
                } else if (const util::rich::AlignmentAttribute* p = dynamic_cast<const util::rich::AlignmentAttribute*>(&att)) {
                    m_result += afl::string::Format("{ALIGN %d,%d:", p->getWidth(), p->getAlignment());
                } else if (const util::rich::LinkAttribute* p = dynamic_cast<const util::rich::LinkAttribute*>(&att)) {
                    m_result += afl::string::Format("{LINK %s:", p->getTarget());
                } else {
                    m_result += "{UNKNOWN:";
                }
                return true;
            }
        virtual bool endAttribute(const util::rich::Attribute& /*att*/)
            { m_result += "}"; return true; }
     private:
        String_t& m_result;
    };

    String_t transform(String_t in)
    {
        String_t result;
        TestVisitor v(result);
        util::rich::Parser::parseXml(in).visit(v);
        return result;
    }
}


/** Test the "parseXml" function, first version.
    This is the initial test to make sure it works somehow. */
AFL_TEST("util.rich.Parser:parseXml", a)
{
    util::rich::Text result = util::rich::Parser::parseXml("Hello, <b>bold</b> world!");
    a.checkEqual("01. getText", result.getText(), "Hello, bold world!");
    a.checkEqual("02. getNumAttributes", result.getNumAttributes(), 1U);

    class MyVisitor : public util::rich::Visitor {
     public:
        MyVisitor(afl::test::Assert a)
            : m_assert(a)
            { }
        virtual bool handleText(String_t /*text*/)
            { return true; }
        virtual bool startAttribute(const util::rich::Attribute& att)
            {
                const util::rich::StyleAttribute* a = dynamic_cast<const util::rich::StyleAttribute*>(&att);
                m_assert.checkNonNull("11. StyleAttribute", a);
                m_assert.checkEqual("12. getStyle", a->getStyle(), util::rich::StyleAttribute::Bold);
                return true;
            }
        virtual bool endAttribute(const util::rich::Attribute& att)
            { return startAttribute(att); }
     private:
        afl::test::Assert m_assert;
    };
    MyVisitor v(a);
    result.visit(v);
}

/** Test parseXml(), all variants. */
AFL_TEST("util.rich.Parser:parseXml:variants", a)
{
    // Simple text
    a.checkEqual("01", transform(""), "");
    a.checkEqual("02", transform("x"), "x");
    a.checkEqual("03", transform("a &amp; b"), "a & b");
    a.checkEqual("04", transform("a &lt; b"), "a < b");
    a.checkEqual("05", transform("a &gt; b"), "a > b");
    a.checkEqual("06", transform("a&#48;b"), "a0b");

    // Unknown tag
    a.checkEqual("11", transform("a <fancy>b</fancy> c"), "a b c");
    a.checkEqual("12", transform("a <?pi>b c"), "a b c");
    a.checkEqual("13", transform("a <?pi x=y>b c"), "a b c");

    // Unknown tag: parseXml does not handle <br>
    a.checkEqual("21", transform("a<br />b"), "ab");

    // <a>
    a.checkEqual("31", transform("<a href=\"http://x\">link</a>"), "{LINK http://x:link}");
    a.checkEqual("32", transform("x<a href=\"http://x\">link</a>y"), "x{LINK http://x:link}y");
    a.checkEqual("33", transform("x<a what=\"http://x\">link</a>y"), "xlinky");
    a.checkEqual("34", transform("x<a>link</a>y"), "xlinky");

    // Styles
    a.checkEqual("41", transform("<b>x</b>y"), "{BOLD:x}y");
    // a.checkEqual("42", transform("<em>x</em>y"), "{BOLD:x}y");  // FIXME: should be italic!
    a.checkEqual("43", transform("<u>x</u>y"), "{UNDERLINE:x}y");
    a.checkEqual("44", transform("a<tt>b</tt>"), "a{FIXED:b}");
    a.checkEqual("45", transform("<b>this is <u>important</u></b>!"), "{BOLD:this is {UNDERLINE:important}}!");
    a.checkEqual("46", transform("a<big>b</big>"), "a{BIG:b}");
    a.checkEqual("47", transform("a<small>b</small>"), "a{SMALL:b}");
    a.checkEqual("48", transform("<font color='red'>Red</font> alert"), "{RED:Red} alert");
    a.checkEqual("49", transform("<font>Colorless</font> alert"), "Colorless alert");
    a.checkEqual("50", transform("<font color='whatever'>Whatever</font> alert"), "{STATIC:Whatever} alert");

    // <kbd>
    a.checkEqual("51", transform("use <kbd>x</kbd> to..."), "use {KEY:x} to...");
    a.checkEqual("52", transform("use <kbd>ctrl-x</kbd> to..."), "use {KEY:ctrl}" UTF_HYPHEN "{KEY:x} to...");
    a.checkEqual("53", transform("use <key>ctrl-x</key> to..."), "use {KEY:ctrl}" UTF_HYPHEN "{KEY:x} to...");
    a.checkEqual("54", transform("use <kbd>Alt-.</kbd> to..."), "use {KEY:Alt}" UTF_HYPHEN "{KEY:.} to...");
    a.checkEqual("55", transform("use <kbd>Alt+1</kbd> to..."), "use {KEY:Alt}+{KEY:1} to...");
    a.checkEqual("56", transform("use <kbd>Up/Down</kbd> to..."), "use {KEY:Up}/{KEY:Down} to...");
    a.checkEqual("57", transform("use <kbd>Alt-Up/Alt-Down</kbd> to..."), "use {KEY:Alt}" UTF_HYPHEN "{KEY:Up}/{KEY:Alt}" UTF_HYPHEN "{KEY:Down} to...");
    a.checkEqual("58", transform("press <kbd>A, B, A, B, ...</kbd> to..."), "press {KEY:A}, {KEY:B}, {KEY:A}, {KEY:B}, {KEY:.}.. to...");
    a.checkEqual("59", transform("use <kbd>ctrl-<b>x</b></kbd> to..."), "use {KEY:ctrl}" UTF_HYPHEN "{KEY:x} to...");

    // <align>
    a.checkEqual("61", transform("<align width=100 align=right>boxy text</align>"), "{ALIGN 100,2:boxy text}");
    a.checkEqual("62", transform("<align width=100 align=left>boxy text</align>"), "{ALIGN 100,0:boxy text}");
    a.checkEqual("63", transform("<align width=42 align='where'>boxy text</align>"), "{ALIGN 42,0:boxy text}");
    a.checkEqual("64", transform("<align align=right width=100>boxy text</align>"), "{ALIGN 100,2:boxy text}");
    a.checkEqual("65", transform("<align>boxy text</align>"), "{ALIGN 0,0:boxy text}");
}

/** Test skipTag(). */
AFL_TEST("util.rich.Parser:skipTag", a)
{
    afl::charset::DefaultCharsetFactory csf;
    afl::io::ConstMemoryStream ms(afl::string::toBytes("<this>tag <is /> skipped</this><b>result</b>"));
    afl::io::xml::Reader rdr(ms, afl::io::xml::DefaultEntityHandler::getInstance(), csf);
    util::rich::Parser testee(rdr);
    testee.readNext();          // start
    a.checkEqual("01. reader", &testee.reader(), &rdr);
    a.checkEqual("02. getCurrentToken", testee.getCurrentToken(), afl::io::xml::BaseReader::TagStart);

    testee.skipTag();           // function to test

    util::rich::Text result = testee.parseText(true);
    a.checkEqual("11. getText", result.getText(), "result");
    a.checkEqual("12. getNumAttributes", result.getNumAttributes(), 1U);
}

/** Test space normalisation. */
AFL_TEST("util.rich.Parser:space-normalisation", a)
{
    afl::charset::DefaultCharsetFactory csf;
    afl::io::ConstMemoryStream ms(afl::string::toBytes("hello   there  <br/>  general\nkenobi"));
    afl::io::xml::Reader rdr(ms, afl::io::xml::DefaultEntityHandler::getInstance(), csf);
    util::rich::Parser testee(rdr);
    testee.readNext();          // start

    String_t result;
    TestVisitor v(result);
    testee.parse().visit(v);

    // As of 20200703, this produces a space before the \n\n.
    // We cannot ignore space before a tag in general because that makes 'foo <b>bar</b>' not work;
    // for now, it's not worth adding a special case for <br>.
    a.checkEqual("01", result, "hello there \n\ngeneral kenobi");
}
