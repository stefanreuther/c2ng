/**
  *  \file test/util/rich/texttest.cpp
  *  \brief Test for util::rich::Text
  */

#include "util/rich/text.hpp"

#include "afl/test/testrunner.hpp"
#include "util/rich/colorattribute.hpp"
#include "util/rich/visitor.hpp"

namespace {
    class AttributeCounter : public util::rich::Visitor {
     public:
        AttributeCounter()
            : m_start(0), m_end(0)
            { }
        bool handleText(String_t)
            { return true; }
        bool startAttribute(const util::rich::Attribute&)
            { ++m_start; return true; }
        bool endAttribute(const util::rich::Attribute&)
            { ++m_end; return true; }

        void check(afl::test::Assert a, int att)
            {
                a.checkEqual("01. start=end", m_start, m_end);
                a.checkEqual("02. start=att", m_start, att);
            }
     private:
        int m_start, m_end;
    };

    class NullAttribute : public util::rich::Attribute {
     public:
        virtual NullAttribute* clone() const
            { return new NullAttribute(); }
    };
}

/** Simple test. */
AFL_TEST("util.rich.Text:basics", a)
{
    // ex UiRichTextTestSuite::testRichText
    util::rich::Text s1("hello, world");
    a.checkEqual("01. size", s1.size(), 12U);
    a.checkEqual("02. getNumAttributes", s1.getNumAttributes(), 0U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(s1)).check(a("03. att"), 0);

    util::rich::Text s2(util::SkinColor::Static, "static text");
    a.checkEqual("11. size", s2.size(), 11U);
    a.checkEqual("12. getNumAttributes", s2.getNumAttributes(), 1U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(s2)).check(a("13. att"), 1);

    util::rich::Text s3 = s1 + s2 + "raw" + util::rich::Text("background").withNewAttribute(new util::rich::ColorAttribute(util::SkinColor::Background));
    a.checkEqual("21. size", s3.size(), 36U);
    a.checkEqual("22. getNumAttributes", s3.getNumAttributes(), 2U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(s3)).check(a("23. att"), 2);

    a.checkEqual("31. substr", s3.substr(10, 2).getNumAttributes(), 0U);
    a.checkEqual("32. substr", s3.substr(10, 3).getNumAttributes(), 1U);
    a.checkEqual("33. substr", s3.substr(12, 2).getNumAttributes(), 1U);
}

/** Test various construction methods. */
AFL_TEST("util.rich.Text:construction", a)
{
    // Empty
    util::rich::Text a1;
    a.check     ("01. empty",            a1.empty());
    a.checkEqual("02. size",             a1.size(), 0U);
    a.checkEqual("03. length",           a1.length(), 0U);
    a.checkEqual("04. getText",          a1.getText(), "");
    a.checkEqual("05. getNumAttributes", a1.getNumAttributes(), 0U);

    // From NTBS
    util::rich::Text a2("x");
    a.check     ("11. empty",           !a2.empty());
    a.checkEqual("12. size",             a2.size(), 1U);
    a.checkEqual("13. length",           a2.length(), 1U);
    a.checkEqual("14. getText",          a2.getText(), "x");
    a.checkEqual("15. getNumAttributes", a2.getNumAttributes(), 0U);

    // From string
    util::rich::Text a3(String_t("yz"));
    a.check     ("21. empty",           !a3.empty());
    a.checkEqual("22. size",             a3.size(), 2U);
    a.checkEqual("23. length",           a3.length(), 2U);
    a.checkEqual("24. getText",          a3.getText(), "yz");
    a.checkEqual("25. getNumAttributes", a3.getNumAttributes(), 0U);

    // From color and NTBS
    util::rich::Text a4(util::SkinColor::Red, "red");
    a.check     ("31. empty",           !a4.empty());
    a.checkEqual("32. size",             a4.size(), 3U);
    a.checkEqual("33. length",           a4.length(), 3U);
    a.checkEqual("34. getText",          a4.getText(), "red");
    a.checkEqual("35. getNumAttributes", a4.getNumAttributes(), 1U);

    // From color and empty NTBS
    util::rich::Text a5(util::SkinColor::Red, "");
    a.check     ("41. empty",            a5.empty());
    a.checkEqual("42. size",             a5.size(), 0U);
    a.checkEqual("43. length",           a5.length(), 0U);
    a.checkEqual("44. getText",          a5.getText(), "");
    a.checkEqual("45. getNumAttributes", a5.getNumAttributes(), 0U);

    // From color and string
    util::rich::Text a6(util::SkinColor::Blue, String_t("blue"));

    a.check     ("51. empty",           !a6.empty());
    a.checkEqual("52. size",             a6.size(), 4U);
    a.checkEqual("53. length",           a6.length(), 4U);
    a.checkEqual("54. getText",          a6.getText(), "blue");
    a.checkEqual("55. getNumAttributes", a6.getNumAttributes(), 1U);

    // From color and empty string
    util::rich::Text                     a7(util::SkinColor::Blue, String_t());
    a.check     ("61. empty",            a7.empty());
    a.checkEqual("62. size",             a7.size(), 0U);
    a.checkEqual("63. length",           a7.length(), 0U);
    a.checkEqual("64. getText",          a7.getText(), "");
    a.checkEqual("65. getNumAttributes", a7.getNumAttributes(), 0U);

    // From other text
    util::rich::Text                     a8(a6);
    a.check     ("71. empty",           !a8.empty());
    a.checkEqual("72. size",             a8.size(), 4U);
    a.checkEqual("73. length",           a8.length(), 4U);
    a.checkEqual("74. getText",          a8.getText(), "blue");
    a.checkEqual("75. getNumAttributes", a8.getNumAttributes(), 1U);

    // From other text range
    util::rich::Text a9(a6, 1, 2);
    a.check     ("81. empty",           !a9.empty());
    a.checkEqual("82. size",             a9.size(), 2U);
    a.checkEqual("83. length",           a9.length(), 2U);
    a.checkEqual("84. getText",          a9.getText(), "lu");
    a.checkEqual("85. getNumAttributes", a9.getNumAttributes(), 1U);

    // From other text range, undelimited
    util::rich::Text a10(a6, 3);
    a.check     ("91. empty",           !a10.empty());
    a.checkEqual("92. size",             a10.size(), 1U);
    a.checkEqual("93. length",           a10.length(), 1U);
    a.checkEqual("94. getText",          a10.getText(), "e");
    a.checkEqual("95. getNumAttributes", a10.getNumAttributes(), 1U);

    // From other text range, effectively empty
    util::rich::Text a11(a6, 4);
    a.check     ("101. empty",            a11.empty());
    a.checkEqual("102. size",             a11.size(), 0U);
    a.checkEqual("103. length",           a11.length(), 0U);
    a.checkEqual("104. getText",          a11.getText(), "");
    a.checkEqual("105. getNumAttributes", a11.getNumAttributes(), 0U);
}

/*
 *  Test withNewAttribute() etc.
 */

// Standard case
AFL_TEST("util.rich.Text:with:normal", a)
{
    util::rich::Text t("x");
    a.checkEqual("01. withNewAttribute", &t.withNewAttribute(new NullAttribute()), &t);
    a.checkEqual("02. withColor",        &t.withColor(util::SkinColor::Green), &t);
    a.checkEqual("03. withStyle",        &t.withStyle(util::rich::StyleAttribute::Big), &t);
    a.checkEqual("04. getText",          t.getText(), "x");
    a.checkEqual("05. getNumAttributes", t.getNumAttributes(), 3U);
}

// Empty case
AFL_TEST("util.rich.Text:with:empty", a)
{
    util::rich::Text t("");
    a.checkEqual("11. withNewAttribute", &t.withNewAttribute(new NullAttribute()), &t);
    a.checkEqual("12. withColor",        &t.withColor(util::SkinColor::Green), &t);
    a.checkEqual("13. withStyle",        &t.withStyle(util::rich::StyleAttribute::Big), &t);
    a.checkEqual("14. getText",          t.getText(), "");
    a.checkEqual("15. getNumAttributes", t.getNumAttributes(), 0U);
}

// Error case
AFL_TEST("util.rich.Text:with:null", a)
{
    util::rich::Text t("x");
    a.checkEqual("21. withNewAttribute", &t.withNewAttribute(0), &t);
    a.checkEqual("22. getText",          t.getText(), "x");
    a.checkEqual("23. getNumAttributes", t.getNumAttributes(), 0U);
}


/** Test string operations (erase, find, append). */
AFL_TEST("util.rich.Text:string-ops", a)
{
    // Build a string
    util::rich::Text t("aa");
    t.append(util::rich::Text(util::SkinColor::Yellow, "bb"));
    t.append("cc");
    t.append(String_t("dd"));
    t.append(util::SkinColor::Green, "ee");
    t.append(util::SkinColor::Green, String_t("ff"));

    t = t + "gg";

    // Verify
    a.checkEqual("01. getText", t.getText(), "aabbccddeeffgg");
    a.checkEqual("02. size",    t.size(), 14U);
    a.checkEqual("03. length",  t.length(), 14U);

    a.checkEqual("11. find", t.find('a'), 0U);
    a.checkEqual("12. find", t.find('b'), 2U);
    a.checkEqual("13. find", t.find('g'), 12U);
    a.checkEqual("14. find", t.find('x'), String_t::npos);

    a.checkEqual("21. op[]", t[0], 'a');
    a.checkEqual("22. op[]", t[1], 'a');
    a.checkEqual("23. op[]", t[2], 'b');
    a.checkEqual("24. op[]", t[13], 'g');
    a.checkEqual("25. op[]", t[14], '\0');

    a.checkEqual("31. getNumAttributes", t.getNumAttributes(), 3U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(t)).check(a("32. att"), 3);

    // Make a substring
    util::rich::Text sub1 = t.substr(3, 6);
    a.checkEqual("41. getText", sub1.getText(), "bccdde");
    a.checkEqual("42. size", sub1.size(), 6U);
    a.checkEqual("43. getNumAttributes", sub1.getNumAttributes(), 2U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(sub1)).check(a("44. att"), 2);

    // Make another substring
    util::rich::Text sub2 = t.substr(8);
    a.checkEqual("51. getText", sub2.getText(), "eeffgg");
    a.checkEqual("52. size", sub2.size(), 6U);
    a.checkEqual("53. getNumAttributes", sub2.getNumAttributes(), 2U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(sub2)).check(a("54. att"), 2);

    // Erase substring
    sub2.erase(1, 3);
    a.checkEqual("61. getText", sub2.getText(), "egg");
    a.checkEqual("62. size", sub2.size(), 3U);
    a.checkEqual("63. getNumAttributes", sub2.getNumAttributes(), 1U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(sub2)).check(a("64. att"), 1);

    // Swap
    sub1.swap(sub2);
    a.checkEqual("71. getText", sub1.getText(), "egg");
    a.checkEqual("72. getText", sub2.getText(), "bccdde");
    a.checkEqual("73. getNumAttributes", sub1.getNumAttributes(), 1U);
    a.checkEqual("74. getNumAttributes", sub2.getNumAttributes(), 2U);

    // Self-append
    t += t;
    a.checkEqual("81. getText", t.getText(), "aabbccddeeffggaabbccddeeffgg");
    a.checkEqual("82. size", t.size(), 28U);
    a.checkEqual("83. getNumAttributes", t.getNumAttributes(), 6U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(t)).check(a("84. att"), 6);

    // Self-Swap
    t.swap(t);
    a.checkEqual("91. getText", t.getText(), "aabbccddeeffggaabbccddeeffgg");
    a.checkEqual("92. getNumAttributes", t.getNumAttributes(), 6U);

    // Self-assignment
    t = t;
    a.checkEqual("101. getText", t.getText(), "aabbccddeeffggaabbccddeeffgg");
    a.checkEqual("102. getNumAttributes", t.getNumAttributes(), 6U);

    // Clear
    t.clear();
    a.checkEqual("111. getText", t.getText(), "");
    a.checkEqual("112. getNumAttributes", t.getNumAttributes(), 0U);
    a.check("113. empty", t.empty());

    // Append
    t += "a";
    a.checkEqual("121. getText", t.getText(), "a");
    a.checkEqual("122. getNumAttributes", t.getNumAttributes(), 0U);

    t += String_t("b");
    a.checkEqual("131. getText", t.getText(), "ab");
    a.checkEqual("132. getNumAttributes", t.getNumAttributes(), 0U);
}
