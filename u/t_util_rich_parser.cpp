/**
  *  \file u/t_util_rich_parser.cpp
  *  \brief Test for util::rich::Parser
  */

#include "util/rich/parser.hpp"

#include "t_util_rich.hpp"
#include "util/rich/text.hpp"
#include "util/rich/visitor.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/rich/colorattribute.hpp"
#include "util/rich/linkattribute.hpp"
#include "util/rich/alignmentattribute.hpp"
#include "afl/string/format.hpp"
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
void
TestUtilRichParser::testParseXml()
{
    util::rich::Text result = util::rich::Parser::parseXml("Hello, <b>bold</b> world!");
    TS_ASSERT_EQUALS(result.getText(), "Hello, bold world!");
    TS_ASSERT_EQUALS(result.getNumAttributes(), 1U);

    class MyVisitor : public util::rich::Visitor {
     public:
        virtual bool handleText(String_t /*text*/)
            { return true; }
        virtual bool startAttribute(const util::rich::Attribute& att)
            {
                const util::rich::StyleAttribute* a = dynamic_cast<const util::rich::StyleAttribute*>(&att);
                TS_ASSERT(a != 0);
                TS_ASSERT_EQUALS(a->getStyle(), util::rich::StyleAttribute::Bold);
                return true;
            }
        virtual bool endAttribute(const util::rich::Attribute& att)
            { return startAttribute(att); }
    };
    MyVisitor v;
    result.visit(v);
}

/** Test parseXml(), all variants. */
void
TestUtilRichParser::testAll()
{
    // Simple text
    TS_ASSERT_EQUALS(transform(""), "");
    TS_ASSERT_EQUALS(transform("x"), "x");
    TS_ASSERT_EQUALS(transform("a &amp; b"), "a & b");
    TS_ASSERT_EQUALS(transform("a &lt; b"), "a < b");
    TS_ASSERT_EQUALS(transform("a &gt; b"), "a > b");
    TS_ASSERT_EQUALS(transform("a&#48;b"), "a0b");

    // Unknown tag
    TS_ASSERT_EQUALS(transform("a <fancy>b</fancy> c"), "a b c");

    // Unknown tag: parseXml does not handle <br>
    TS_ASSERT_EQUALS(transform("a<br />b"), "ab");

    // <a>
    TS_ASSERT_EQUALS(transform("<a href=\"http://x\">link</a>"), "{LINK http://x:link}");
    TS_ASSERT_EQUALS(transform("x<a href=\"http://x\">link</a>y"), "x{LINK http://x:link}y");
    TS_ASSERT_EQUALS(transform("x<a what=\"http://x\">link</a>y"), "xlinky");
    TS_ASSERT_EQUALS(transform("x<a>link</a>y"), "xlinky");

    // Styles
    TS_ASSERT_EQUALS(transform("<b>x</b>y"), "{BOLD:x}y");
    // TS_ASSERT_EQUALS(transform("<em>x</em>y"), "{BOLD:x}y");  // FIXME: should be italic!
    TS_ASSERT_EQUALS(transform("<u>x</u>y"), "{UNDERLINE:x}y");
    TS_ASSERT_EQUALS(transform("a<tt>b</tt>"), "a{FIXED:b}");
    TS_ASSERT_EQUALS(transform("<b>this is <u>important</u></b>!"), "{BOLD:this is {UNDERLINE:important}}!");
    TS_ASSERT_EQUALS(transform("a<big>b</big>"), "a{BIG:b}");
    TS_ASSERT_EQUALS(transform("a<small>b</small>"), "a{SMALL:b}");
    TS_ASSERT_EQUALS(transform("<font color='red'>Red</font> alert"), "{RED:Red} alert");
    TS_ASSERT_EQUALS(transform("<font>Colorless</font> alert"), "Colorless alert");
    TS_ASSERT_EQUALS(transform("<font color='whatever'>Whatever</font> alert"), "{STATIC:Whatever} alert");

    // <kbd>
    TS_ASSERT_EQUALS(transform("use <kbd>x</kbd> to..."), "use {KEY:x} to...");
    TS_ASSERT_EQUALS(transform("use <kbd>ctrl-x</kbd> to..."), "use {KEY:ctrl}" UTF_HYPHEN "{KEY:x} to...");
    TS_ASSERT_EQUALS(transform("use <key>ctrl-x</key> to..."), "use {KEY:ctrl}" UTF_HYPHEN "{KEY:x} to...");
    TS_ASSERT_EQUALS(transform("use <kbd>Alt-.</kbd> to..."), "use {KEY:Alt}" UTF_HYPHEN "{KEY:.} to...");
    TS_ASSERT_EQUALS(transform("use <kbd>Alt+1</kbd> to..."), "use {KEY:Alt}+{KEY:1} to...");
    TS_ASSERT_EQUALS(transform("use <kbd>Up/Down</kbd> to..."), "use {KEY:Up}/{KEY:Down} to...");
    TS_ASSERT_EQUALS(transform("use <kbd>Alt-Up/Alt-Down</kbd> to..."), "use {KEY:Alt}" UTF_HYPHEN "{KEY:Up}/{KEY:Alt}" UTF_HYPHEN "{KEY:Down} to...");
    TS_ASSERT_EQUALS(transform("press <kbd>A, B, A, B, ...</kbd> to..."), "press {KEY:A}, {KEY:B}, {KEY:A}, {KEY:B}, {KEY:.}.. to...");
    TS_ASSERT_EQUALS(transform("use <kbd>ctrl-<b>x</b></kbd> to..."), "use {KEY:ctrl}" UTF_HYPHEN "{KEY:x} to...");

    // <align>
    TS_ASSERT_EQUALS(transform("<align width=100 align=right>boxy text</align>"), "{ALIGN 100,2:boxy text}");
    TS_ASSERT_EQUALS(transform("<align width=100 align=left>boxy text</align>"), "{ALIGN 100,0:boxy text}");
    TS_ASSERT_EQUALS(transform("<align width=42 align='where'>boxy text</align>"), "{ALIGN 42,0:boxy text}");
    TS_ASSERT_EQUALS(transform("<align align=right width=100>boxy text</align>"), "{ALIGN 100,2:boxy text}");
    TS_ASSERT_EQUALS(transform("<align>boxy text</align>"), "{ALIGN 0,0:boxy text}");
}

