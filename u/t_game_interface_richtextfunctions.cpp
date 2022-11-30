/**
  *  \file u/t_game_interface_richtextfunctions.cpp
  *  \brief Test for game::interface::RichTextFunctions
  */

#include "game/interface/richtextfunctions.hpp"

#include "t_game_interface.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/interface/richtextvalue.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "util/rich/visitor.hpp"
#include "util/unicodechars.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/rich/colorattribute.hpp"

namespace {
    typedef std::auto_ptr<afl::data::Value> Value_t;
    typedef game::interface::RichTextValue::Ptr_t Ptr_t;
}

/** Test IFRAdd. */
void
TestGameInterfaceRichTextFunctions::testRAdd()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session s(tx, fs);

    // Build a bunch of parameters
    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackNew(interpreter::makeIntegerValue(1));
    seg.pushBackNew(interpreter::makeIntegerValue(2));
    seg.pushBackNew(interpreter::makeStringValue("three"));
    seg.pushBackNew(interpreter::makeStringValue("four"));
    seg.pushBackNew(new game::interface::RichTextValue(*new util::rich::Text(util::SkinColor::Red, "red")));

    // Test a number of invocations
    {
        // RAdd() ==> ''
        interpreter::Arguments args(seg, 0, 0);
        Value_t result(game::interface::IFRAdd(s, args));
        Ptr_t p;
        TS_ASSERT(game::interface::checkRichArg(p, result.get()));
        TS_ASSERT_EQUALS(p->size(), 0U);
    }
    {
        // RAdd(EMPTY) ==> EMPTY
        interpreter::Arguments args(seg, 0, 1);
        Value_t result(game::interface::IFRAdd(s, args));
        Ptr_t p;
        TS_ASSERT(!game::interface::checkRichArg(p, result.get()));
        TS_ASSERT(result.get() == 0);
    }
    {
        // RAdd(EMPTY, 1) ==> EMPTY
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRAdd(s, args));
        Ptr_t p;
        TS_ASSERT(!game::interface::checkRichArg(p, result.get()));
        TS_ASSERT(result.get() == 0);
    }
    {
        // RAdd(1, 2) ==> "12"
        interpreter::Arguments args(seg, 1, 2);
        Value_t result(game::interface::IFRAdd(s, args));
        Ptr_t p;
        TS_ASSERT(game::interface::checkRichArg(p, result.get()));
        TS_ASSERT_EQUALS(p->getText(), "12");
        TS_ASSERT_EQUALS(p->getNumAttributes(), 0U);
    }
    {
        // RAdd(2, "three", "four") ==> "2threefour"
        interpreter::Arguments args(seg, 2, 3);
        Value_t result(game::interface::IFRAdd(s, args));
        Ptr_t p;
        TS_ASSERT(game::interface::checkRichArg(p, result.get()));
        TS_ASSERT_EQUALS(p->getText(), "2threefour");
        TS_ASSERT_EQUALS(p->getNumAttributes(), 0U);
    }
    {
        // RAdd("four", RStyle("red", "red")) ==> "fourred"
        interpreter::Arguments args(seg, 4, 2);
        Value_t result(game::interface::IFRAdd(s, args));
        Ptr_t p;
        TS_ASSERT(game::interface::checkRichArg(p, result.get()));
        TS_ASSERT_EQUALS(p->getText(), "fourred");
        TS_ASSERT_EQUALS(p->getNumAttributes(), 1U);
    }
}

/** Test IFRMid. */
void
TestGameInterfaceRichTextFunctions::testRMid()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session s(tx, fs);

    // Test a number of invocations
    {
        // RMid("foo", 2) = "oo"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("foo"));
        seg.setNew(1, interpreter::makeIntegerValue(2));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRMid(s, args));
        Ptr_t p;
        TS_ASSERT(game::interface::checkRichArg(p, result.get()));
        TS_ASSERT_EQUALS(p->getText(), "oo");
    }
    {
        // RMid("foo", 100) = ""
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("foo"));
        seg.setNew(1, interpreter::makeIntegerValue(100));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRMid(s, args));
        Ptr_t p;
        TS_ASSERT(game::interface::checkRichArg(p, result.get()));
        TS_ASSERT_EQUALS(p->getText(), "");
    }
    {
        // RMid("foo", 1, 2) = "fo"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("foo"));
        seg.setNew(1, interpreter::makeIntegerValue(1));
        seg.setNew(2, interpreter::makeIntegerValue(2));
        interpreter::Arguments args(seg, 0, 3);
        Value_t result(game::interface::IFRMid(s, args));
        Ptr_t p;
        TS_ASSERT(game::interface::checkRichArg(p, result.get()));
        TS_ASSERT_EQUALS(p->getText(), "fo");
    }
    {
        // RMid("<unicode1><unicode2>", 2) = "<unicode2>"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue(UTF_BULLET UTF_UP_ARROW));
        seg.setNew(1, interpreter::makeIntegerValue(2));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRMid(s, args));
        Ptr_t p;
        TS_ASSERT(game::interface::checkRichArg(p, result.get()));
        TS_ASSERT_EQUALS(p->getText(), UTF_UP_ARROW);
    }
    {
        // RMid(?,?,?,?) = too many args
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS(game::interface::IFRMid(s, args), interpreter::Error);
    }
}

/** Test IFRString. */
void
TestGameInterfaceRichTextFunctions::testRString()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session s(tx, fs);

    // Build a bunch of parameters
    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackNew(interpreter::makeIntegerValue(2));
    seg.pushBackNew(interpreter::makeStringValue("three"));
    seg.pushBackNew(new game::interface::RichTextValue(*new util::rich::Text(util::SkinColor::Red, "four")));

    // Test a number of invocations
    {
        // RString() -> arity error
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFRString(s, args), interpreter::Error);
    }
    {
        // RString(?,?) -> arity error
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFRString(s, args), interpreter::Error);
    }
    {
        // RString(EMPTY) => EMPTY
        interpreter::Arguments args(seg, 0, 1);
        Value_t result(game::interface::IFRString(s, args));
        TS_ASSERT(result.get() == 0);
    }
    {
        // RString(2) => "2"
        interpreter::Arguments args(seg, 1, 1);
        Value_t result(game::interface::IFRString(s, args));
        String_t sv;
        TS_ASSERT(interpreter::checkStringArg(sv, result.get()));
        TS_ASSERT_EQUALS(sv, "2");
    }
    {
        // RString("three") => "three"
        interpreter::Arguments args(seg, 2, 1);
        Value_t result(game::interface::IFRString(s, args));
        String_t sv;
        TS_ASSERT(interpreter::checkStringArg(sv, result.get()));
        TS_ASSERT_EQUALS(sv, "three");
    }
    {
        // RString(RStyle("red","four") => "four"
        interpreter::Arguments args(seg, 3, 1);
        Value_t result(game::interface::IFRString(s, args));
        String_t sv;
        TS_ASSERT(interpreter::checkStringArg(sv, result.get()));
        TS_ASSERT_EQUALS(sv, "four");
    }
}

/** Test IFRLen. */
void
TestGameInterfaceRichTextFunctions::testRLen()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session s(tx, fs);

    // Build a bunch of parameters
    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackNew(interpreter::makeIntegerValue(2));
    seg.pushBackNew(interpreter::makeStringValue("three"));
    seg.pushBackNew(new game::interface::RichTextValue(*new util::rich::Text(util::SkinColor::Red, "four")));

    // Test a number of invocations
    {
        // RLen() -> arity error
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFRLen(s, args), interpreter::Error);
    }
    {
        // RLen(?,?) -> arity error
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFRLen(s, args), interpreter::Error);
    }
    {
        // RLen(EMPTY) => EMPTY
        interpreter::Arguments args(seg, 0, 1);
        Value_t result(game::interface::IFRLen(s, args));
        TS_ASSERT(result.get() == 0);
    }
    {
        // RLen(2) => 1
        interpreter::Arguments args(seg, 1, 1);
        Value_t result(game::interface::IFRLen(s, args));
        int32_t iv;
        TS_ASSERT(interpreter::checkIntegerArg(iv, result.get()));
        TS_ASSERT_EQUALS(iv, 1);
    }
    {
        // RLen("three") => 5
        interpreter::Arguments args(seg, 2, 1);
        Value_t result(game::interface::IFRLen(s, args));
        int32_t iv;
        TS_ASSERT(interpreter::checkIntegerArg(iv, result.get()));
        TS_ASSERT_EQUALS(iv, 5);
    }
    {
        // RLen(RStyle("red","four") => 4
        interpreter::Arguments args(seg, 3, 1);
        Value_t result(game::interface::IFRLen(s, args));
        int32_t iv;
        TS_ASSERT(interpreter::checkIntegerArg(iv, result.get()));
        TS_ASSERT_EQUALS(iv, 4);
    }
    {
        // Unicode
        afl::data::Segment seg2;
        seg2.pushBackNew(new game::interface::RichTextValue(*new util::rich::Text("\xE2\x86\x90")));
        interpreter::Arguments args(seg2, 0, 1);
        Value_t result(game::interface::IFRLen(s, args));
        int32_t iv;
        TS_ASSERT(interpreter::checkIntegerArg(iv, result.get()));
        TS_ASSERT_EQUALS(iv, 1);
    }
}

/** Test IFRStyle. */
void
TestGameInterfaceRichTextFunctions::testRStyle()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session s(tx, fs);

    class AttributeLister : public util::rich::Visitor {
     public:
        virtual bool handleText(String_t /*text*/)
            { return true; }
        virtual bool startAttribute(const util::rich::Attribute& att)
            { m_attributes.push_back(&att); return true; }
        virtual bool endAttribute(const util::rich::Attribute& /*att*/)
            { return true; }
        size_t size() const
            { return m_attributes.size(); }
        const util::rich::Attribute* operator[](size_t x) const
            { return m_attributes[x]; }
     private:
        std::vector<const util::rich::Attribute*> m_attributes;
    };

    // Test a number of invocations
    {
        // RStyle("red", "the text") = "the text"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("red"));
        seg.setNew(1, interpreter::makeStringValue("the text"));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRStyle(s, args));
        Ptr_t p;
        TS_ASSERT(game::interface::checkRichArg(p, result.get()));
        TS_ASSERT_EQUALS(p->getText(), "the text");
        TS_ASSERT_EQUALS(p->getNumAttributes(), 1U);

        // Verify attribute
        AttributeLister att;
        att.visit(*p);
        TS_ASSERT_EQUALS(att.size(), 1U);
        const util::rich::ColorAttribute* catt = dynamic_cast<const util::rich::ColorAttribute*>(att[0]);
        TS_ASSERT(catt != 0);
        TS_ASSERT_EQUALS(catt->getColor(), util::SkinColor::Red);
    }
    {
        // RStyle("red", "a", "b", 3) = "ab3"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("red"));
        seg.setNew(1, interpreter::makeStringValue("a"));
        seg.setNew(2, interpreter::makeStringValue("b"));
        seg.setNew(3, interpreter::makeIntegerValue(3));
        interpreter::Arguments args(seg, 0, 4);
        Value_t result(game::interface::IFRStyle(s, args));
        Ptr_t p;
        TS_ASSERT(game::interface::checkRichArg(p, result.get()));
        TS_ASSERT_EQUALS(p->getText(), "ab3");
        TS_ASSERT_EQUALS(p->getNumAttributes(), 1U);

        // Verify attribute
        AttributeLister att;
        att.visit(*p);
        TS_ASSERT_EQUALS(att.size(), 1U);
        const util::rich::ColorAttribute* catt = dynamic_cast<const util::rich::ColorAttribute*>(att[0]);
        TS_ASSERT(catt != 0);
        TS_ASSERT_EQUALS(catt->getColor(), util::SkinColor::Red);
    }
    {
        // RStyle("big", "the text") = "the text"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("big"));
        seg.setNew(1, interpreter::makeStringValue("the text"));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRStyle(s, args));
        Ptr_t p;
        TS_ASSERT(game::interface::checkRichArg(p, result.get()));
        TS_ASSERT_EQUALS(p->getText(), "the text");
        TS_ASSERT_EQUALS(p->getNumAttributes(), 1U);

        // Verify attribute
        AttributeLister att;
        att.visit(*p);
        TS_ASSERT_EQUALS(att.size(), 1U);
        const util::rich::StyleAttribute* satt = dynamic_cast<const util::rich::StyleAttribute*>(att[0]);
        TS_ASSERT(satt != 0);
        TS_ASSERT_EQUALS(satt->getStyle(), util::rich::StyleAttribute::Big);
    }
}

/** Test IFRLink. */
void
TestGameInterfaceRichTextFunctions::testRLink()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session s(tx, fs);

    // This is essentially the same as RStyle...
    {
        // RStyle("link", "the text") = "the text"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("link"));
        seg.setNew(1, interpreter::makeStringValue("the text"));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRLink(s, args));
        Ptr_t p;
        TS_ASSERT(game::interface::checkRichArg(p, result.get()));
        TS_ASSERT_EQUALS(p->getText(), "the text");
        TS_ASSERT_EQUALS(p->getNumAttributes(), 1U);
    }
}

/** Test IFRXml. */
void
TestGameInterfaceRichTextFunctions::testRXml()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session s(tx, fs);

    {
        // RXml("<b>&0;</b>&gt;<b>&1;</b>", "x", 3) = "x>3"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("<b>&0;</b>&gt;<b>&1;</b>"));
        seg.setNew(1, interpreter::makeStringValue("x"));
        seg.setNew(2, interpreter::makeIntegerValue(3));
        interpreter::Arguments args(seg, 0, 3);
        Value_t result(game::interface::IFRXml(s, args));
        Ptr_t p;
        TS_ASSERT(game::interface::checkRichArg(p, result.get()));
        TS_ASSERT_EQUALS(p->getText(), "x>3");
        TS_ASSERT_EQUALS(p->getNumAttributes(), 2U);
    }

}
