/**
  *  \file test/game/interface/richtextfunctionstest.cpp
  *  \brief Test for game::interface::RichTextFunctions
  */

#include "game/interface/richtextfunctions.hpp"

#include "afl/data/segment.hpp"
#include "afl/test/testrunner.hpp"
#include "game/interface/richtextvalue.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "util/rich/alignmentattribute.hpp"
#include "util/rich/colorattribute.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/rich/visitor.hpp"
#include "util/unicodechars.hpp"

/**
  *  \file u/t_game_interface_richtextfunctions.cpp
  *  \brief Test for game::interface::RichTextFunctions
  */

namespace {
    typedef std::auto_ptr<afl::data::Value> Value_t;
    typedef game::interface::RichTextValue::Ptr_t Ptr_t;

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
}

/** Test IFRAdd. */
AFL_TEST("game.interface.RichTextFunctions:IFRAdd", a)
{
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
        Value_t result(game::interface::IFRAdd(args));
        Ptr_t p;
        a.check("01. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("02. size", p->size(), 0U);
    }
    {
        // RAdd(EMPTY) ==> EMPTY
        interpreter::Arguments args(seg, 0, 1);
        Value_t result(game::interface::IFRAdd(args));
        Ptr_t p;
        a.check("03. checkRichArg", !game::interface::checkRichArg(p, result.get()));
        a.checkNull("04. result", result.get());
    }
    {
        // RAdd(EMPTY, 1) ==> EMPTY
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRAdd(args));
        Ptr_t p;
        a.check("05. checkRichArg", !game::interface::checkRichArg(p, result.get()));
        a.checkNull("06. result", result.get());
    }
    {
        // RAdd(1, 2) ==> "12"
        interpreter::Arguments args(seg, 1, 2);
        Value_t result(game::interface::IFRAdd(args));
        Ptr_t p;
        a.check("07. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("08. getText", p->getText(), "12");
        a.checkEqual("09. getNumAttributes", p->getNumAttributes(), 0U);
    }
    {
        // RAdd(2, "three", "four") ==> "2threefour"
        interpreter::Arguments args(seg, 2, 3);
        Value_t result(game::interface::IFRAdd(args));
        Ptr_t p;
        a.check("10. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("11. getText", p->getText(), "2threefour");
        a.checkEqual("12. getNumAttributes", p->getNumAttributes(), 0U);
    }
    {
        // RAdd("four", RStyle("red", "red")) ==> "fourred"
        interpreter::Arguments args(seg, 4, 2);
        Value_t result(game::interface::IFRAdd(args));
        Ptr_t p;
        a.check("13. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("14. getText", p->getText(), "fourred");
        a.checkEqual("15. getNumAttributes", p->getNumAttributes(), 1U);
    }
}

/** Test IFRMid. */
AFL_TEST("game.interface.RichTextFunctions:IFRMid", a)
{
    // Test a number of invocations
    {
        // RMid("foo", 2) = "oo"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("foo"));
        seg.setNew(1, interpreter::makeIntegerValue(2));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRMid(args));
        Ptr_t p;
        a.check("01. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("02. getText", p->getText(), "oo");
    }
    {
        // RMid("foo", 100) = ""
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("foo"));
        seg.setNew(1, interpreter::makeIntegerValue(100));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRMid(args));
        Ptr_t p;
        a.check("03. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("04. getText", p->getText(), "");
    }
    {
        // RMid("foo", 1, 2) = "fo"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("foo"));
        seg.setNew(1, interpreter::makeIntegerValue(1));
        seg.setNew(2, interpreter::makeIntegerValue(2));
        interpreter::Arguments args(seg, 0, 3);
        Value_t result(game::interface::IFRMid(args));
        Ptr_t p;
        a.check("05. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("06. getText", p->getText(), "fo");
    }
    {
        // RMid("<unicode1><unicode2>", 2) = "<unicode2>"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue(UTF_BULLET UTF_UP_ARROW));
        seg.setNew(1, interpreter::makeIntegerValue(2));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRMid(args));
        Ptr_t p;
        a.check("07. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("08. getText", p->getText(), UTF_UP_ARROW);
    }
    {
        // RMid(?,?,?,?) = too many args
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 4);
        AFL_CHECK_THROWS(a("09. arity error"), game::interface::IFRMid(args), interpreter::Error);
    }
    {
        // RMid(EMPTY, EMPTY) = EMPTY
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRMid(args));
        a.checkNull("10. result", result.get());
    }
}

/** Test IFRString. */
AFL_TEST("game.interface.RichTextFunctions:IFRString", a)
{
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
        AFL_CHECK_THROWS(a("01. arity error"), game::interface::IFRString(args), interpreter::Error);
    }
    {
        // RString(?,?) -> arity error
        interpreter::Arguments args(seg, 0, 2);
        AFL_CHECK_THROWS(a("02. arity error"), game::interface::IFRString(args), interpreter::Error);
    }
    {
        // RString(EMPTY) => EMPTY
        interpreter::Arguments args(seg, 0, 1);
        Value_t result(game::interface::IFRString(args));
        a.checkNull("03. result", result.get());
    }
    {
        // RString(2) => "2"
        interpreter::Arguments args(seg, 1, 1);
        Value_t result(game::interface::IFRString(args));
        String_t sv;
        a.check("04. checkStringArg", interpreter::checkStringArg(sv, result.get()));
        a.checkEqual("05. value", sv, "2");
    }
    {
        // RString("three") => "three"
        interpreter::Arguments args(seg, 2, 1);
        Value_t result(game::interface::IFRString(args));
        String_t sv;
        a.check("06. checkStringArg", interpreter::checkStringArg(sv, result.get()));
        a.checkEqual("07. value", sv, "three");
    }
    {
        // RString(RStyle("red","four") => "four"
        interpreter::Arguments args(seg, 3, 1);
        Value_t result(game::interface::IFRString(args));
        String_t sv;
        a.check("08. checkStringArg", interpreter::checkStringArg(sv, result.get()));
        a.checkEqual("09. value", sv, "four");
    }
}

/** Test IFRLen. */
AFL_TEST("game.interface.RichTextFunctions:IFRLen", a)
{
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
        AFL_CHECK_THROWS(a("01. arity error"), game::interface::IFRLen(args), interpreter::Error);
    }
    {
        // RLen(?,?) -> arity error
        interpreter::Arguments args(seg, 0, 2);
        AFL_CHECK_THROWS(a("02. arity error"), game::interface::IFRLen(args), interpreter::Error);
    }
    {
        // RLen(EMPTY) => EMPTY
        interpreter::Arguments args(seg, 0, 1);
        Value_t result(game::interface::IFRLen(args));
        a.checkNull("03. result", result.get());
    }
    {
        // RLen(2) => 1
        interpreter::Arguments args(seg, 1, 1);
        Value_t result(game::interface::IFRLen(args));
        int32_t iv;
        a.check("04. checkIntegerArg", interpreter::checkIntegerArg(iv, result.get()));
        a.checkEqual("05. result", iv, 1);
    }
    {
        // RLen("three") => 5
        interpreter::Arguments args(seg, 2, 1);
        Value_t result(game::interface::IFRLen(args));
        int32_t iv;
        a.check("06. checkIntegerArg", interpreter::checkIntegerArg(iv, result.get()));
        a.checkEqual("07. result", iv, 5);
    }
    {
        // RLen(RStyle("red","four") => 4
        interpreter::Arguments args(seg, 3, 1);
        Value_t result(game::interface::IFRLen(args));
        int32_t iv;
        a.check("08. checkIntegerArg", interpreter::checkIntegerArg(iv, result.get()));
        a.checkEqual("09. result", iv, 4);
    }
    {
        // Unicode
        afl::data::Segment seg2;
        seg2.pushBackNew(new game::interface::RichTextValue(*new util::rich::Text("\xE2\x86\x90")));
        interpreter::Arguments args(seg2, 0, 1);
        Value_t result(game::interface::IFRLen(args));
        int32_t iv;
        a.check("10. checkIntegerArg", interpreter::checkIntegerArg(iv, result.get()));
        a.checkEqual("11. result", iv, 1);
    }
}

/** Test IFRStyle. */
AFL_TEST("game.interface.RichTextFunctions:IFRStyle", a)
{
    // Test a number of invocations
    {
        // RStyle("red", "the text") = "the text"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("red"));
        seg.setNew(1, interpreter::makeStringValue("the text"));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRStyle(args));
        Ptr_t p;
        a.check("01. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("02. getText", p->getText(), "the text");
        a.checkEqual("03. getNumAttributes", p->getNumAttributes(), 1U);

        // Verify attribute
        AttributeLister att;
        att.visit(*p);
        a.checkEqual("11. size", att.size(), 1U);
        const util::rich::ColorAttribute* catt = dynamic_cast<const util::rich::ColorAttribute*>(att[0]);
        a.checkNonNull("12. ColorAttribute", catt);
        a.checkEqual("13. getColor", catt->getColor(), util::SkinColor::Red);
    }
    {
        // RStyle("red", "a", "b", 3) = "ab3"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("red"));
        seg.setNew(1, interpreter::makeStringValue("a"));
        seg.setNew(2, interpreter::makeStringValue("b"));
        seg.setNew(3, interpreter::makeIntegerValue(3));
        interpreter::Arguments args(seg, 0, 4);
        Value_t result(game::interface::IFRStyle(args));
        Ptr_t p;
        a.check("14. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("15. getText", p->getText(), "ab3");
        a.checkEqual("16. getNumAttributes", p->getNumAttributes(), 1U);

        // Verify attribute
        AttributeLister att;
        att.visit(*p);
        a.checkEqual("21. size", att.size(), 1U);
        const util::rich::ColorAttribute* catt = dynamic_cast<const util::rich::ColorAttribute*>(att[0]);
        a.checkNonNull("22. ColorAttribute", catt);
        a.checkEqual("23. getColor", catt->getColor(), util::SkinColor::Red);
    }
    {
        // RStyle("big", "the text") = "the text"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("big"));
        seg.setNew(1, interpreter::makeStringValue("the text"));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRStyle(args));
        Ptr_t p;
        a.check("24. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("25. getText", p->getText(), "the text");
        a.checkEqual("26. getNumAttributes", p->getNumAttributes(), 1U);

        // Verify attribute
        AttributeLister att;
        att.visit(*p);
        a.checkEqual("31. size", att.size(), 1U);
        const util::rich::StyleAttribute* satt = dynamic_cast<const util::rich::StyleAttribute*>(att[0]);
        a.checkNonNull("32. StyleAttribute", satt);
        a.checkEqual("33. getStyle", satt->getStyle(), util::rich::StyleAttribute::Big);
    }
    {
        // RStyle("big,red", "the text") = "the text"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("big,red"));
        seg.setNew(1, interpreter::makeStringValue("the text"));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRStyle(args));
        Ptr_t p;
        a.check("34. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("35. getText", p->getText(), "the text");
        a.checkEqual("36. getNumAttributes", p->getNumAttributes(), 2U);
    }
    {
        // RStyle("", "text") = "text", with no attributes
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue(""));
        seg.setNew(1, interpreter::makeStringValue("text"));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRStyle(args));
        Ptr_t p;
        a.check("37. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("38. getText", p->getText(), "text");
        a.checkEqual("39. getNumAttributes", p->getNumAttributes(), 0U);
    }
    {
        // RStyle("<invalid>", "text") -> fails
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("<invalid>"));
        seg.setNew(1, interpreter::makeStringValue("text"));
        interpreter::Arguments args(seg, 0, 2);
        AFL_CHECK_THROWS(a("40. invalid attribute"), game::interface::IFRStyle(args), interpreter::Error);
    }
    {
        // RStyle(EMPTY, "text") -> EMPTY
        afl::data::Segment seg;
        seg.setNew(0, 0);
        seg.setNew(1, interpreter::makeStringValue("text"));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRStyle(args));
        a.checkNull("41. result", result.get());
    }
    {
        // RStyle("red", EMPTY) -> EMPTY
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("red"));
        seg.setNew(1, 0);
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRStyle(args));
        a.checkNull("42. result", result.get());
    }
}

/** Test IFRLink. */
AFL_TEST("game.interface.RichTextFunctions:IFRLink", a)
{
    // This is essentially the same as RStyle...
    {
        // RStyle("link", "the text") = "the text"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("link"));
        seg.setNew(1, interpreter::makeStringValue("the text"));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRLink(args));
        Ptr_t p;
        a.check("01. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("02. getText", p->getText(), "the text");
        a.checkEqual("03. getNumAttributes", p->getNumAttributes(), 1U);
    }
    {
        // RStyle(EMPTY, "the text") = EMTPY
        afl::data::Segment seg;
        seg.setNew(0, 0);
        seg.setNew(1, interpreter::makeStringValue("the text"));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRLink(args));
        a.checkNull("04. result", result.get());
    }
    {
        // RStyle("link", EMPTY) = EMPTY
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("link"));
        seg.setNew(1, 0);
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRLink(args));
        a.checkNull("05. result", result.get());
    }
}

/** Test IFRXml. */
AFL_TEST("game.interface.RichTextFunctions:RXml", a)
{
    {
        // RXml("<b>&0;</b>&gt;<b>&1;</b>", "x", 3) = "x>3"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("<b>&0;</b>&gt;<b>&1;</b>"));
        seg.setNew(1, interpreter::makeStringValue("x"));
        seg.setNew(2, interpreter::makeIntegerValue(3));
        interpreter::Arguments args(seg, 0, 3);
        Value_t result(game::interface::IFRXml(args));
        Ptr_t p;
        a.check("01. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("02. getText", p->getText(), "x>3");
        a.checkEqual("03. getNumAttributes", p->getNumAttributes(), 2U);
    }
    {
        // RXml("<b>&0;</b>&gt;<b>&1;</b>") = "x>3"
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("<b>&0;</b>&gt;<b>&1;</b>"));
        interpreter::Arguments args(seg, 0, 3);
        Value_t result(game::interface::IFRXml(args));
        Ptr_t p;
        a.check("04. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("05. getText", p->getText(), ">");
    }
    {
        // RXml(EMPTY, "x", 3) = EMPTY
        afl::data::Segment seg;
        seg.setNew(0, 0);
        seg.setNew(1, interpreter::makeStringValue("x"));
        seg.setNew(2, interpreter::makeIntegerValue(3));
        interpreter::Arguments args(seg, 0, 3);
        Value_t result(game::interface::IFRXml(args));
        a.checkNull("06. result", result.get());
    }
}

/** Test IFRAlign. */
AFL_TEST("game.interface.RichTextFunctions:IFRAlign", a)
{
    {
        // RAlign("text", 100, 1)
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("text"));
        seg.setNew(1, interpreter::makeIntegerValue(100));
        seg.setNew(2, interpreter::makeIntegerValue(1));
        interpreter::Arguments args(seg, 0, 3);
        Value_t result(game::interface::IFRAlign(args));
        Ptr_t p;
        a.check("01. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("02. getText", p->getText(), "text");
        a.checkEqual("03. getNumAttributes", p->getNumAttributes(), 1U);

        AttributeLister att;
        att.visit(*p);
        a.checkEqual("11. size", att.size(), 1U);
        const util::rich::AlignmentAttribute* aatt = dynamic_cast<const util::rich::AlignmentAttribute*>(att[0]);
        a.checkNonNull("12. AlignmentAttribute", aatt);
        a.checkEqual("13. getWidth", aatt->getWidth(), 100);
        a.checkEqual("14. getAlignment", aatt->getAlignment(), 1);
    }
    {
        // RAlign("text", 200)
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("text"));
        seg.setNew(1, interpreter::makeIntegerValue(100));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRAlign(args));
        Ptr_t p;
        a.check("15. checkRichArg", game::interface::checkRichArg(p, result.get()));
        a.checkEqual("16. getText", p->getText(), "text");
        a.checkEqual("17. getNumAttributes", p->getNumAttributes(), 1U);

        AttributeLister att;
        att.visit(*p);
        a.checkEqual("21. size", att.size(), 1U);
        const util::rich::AlignmentAttribute* aatt = dynamic_cast<const util::rich::AlignmentAttribute*>(att[0]);
        a.checkNonNull("22. AlignmentAttribute", aatt);
        a.checkEqual("23. getWidth", aatt->getWidth(), 100);
        a.checkEqual("24. getAlignment", aatt->getAlignment(), 0);   // default
    }
    {
        // RAlign("text", "x") -> error
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("text"));
        seg.setNew(1, interpreter::makeStringValue("x"));
        interpreter::Arguments args(seg, 0, 2);
        AFL_CHECK_THROWS(a("25. type error"), game::interface::IFRAlign(args), interpreter::Error);
    }
    {
        // RAlign("text", 100, 4) -> error
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("text"));
        seg.setNew(1, interpreter::makeIntegerValue(100));
        seg.setNew(2, interpreter::makeIntegerValue(4));
        interpreter::Arguments args(seg, 0, 3);
        AFL_CHECK_THROWS(a("26. type error"), game::interface::IFRAlign(args), interpreter::Error);
    }
    {
        // RAlign("text", EMPTY) = EMPTY
        afl::data::Segment seg;
        seg.setNew(0, interpreter::makeStringValue("text"));
        seg.setNew(1, 0);
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRAlign(args));
        a.checkNull("27. result", result.get());
    }
    {
        // RAlign(EMPTY, 100) = EMPTY
        afl::data::Segment seg;
        seg.setNew(0, 0);
        seg.setNew(1, interpreter::makeIntegerValue(1));
        interpreter::Arguments args(seg, 0, 2);
        Value_t result(game::interface::IFRAlign(args));
        a.checkNull("28. result", result.get());
    }
}
