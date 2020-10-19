/**
  *  \file u/t_util_rich_text.cpp
  *  \brief Test for util::rich::Text
  */

#include "util/rich/text.hpp"

#include "t_util_rich.hpp"
#include "util/rich/visitor.hpp"
#include "util/rich/colorattribute.hpp"

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

        void check(int att)
            {
                TS_ASSERT_EQUALS(m_start, m_end);
                TS_ASSERT_EQUALS(m_start, att);
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
void
TestUtilRichText::testIt()
{
    // ex UiRichTextTestSuite::testRichText
    util::rich::Text s1("hello, world");
    TS_ASSERT_EQUALS(s1.size(), 12U);
    TS_ASSERT_EQUALS(s1.getNumAttributes(), 0U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(s1)).check(0);

    util::rich::Text s2(util::SkinColor::Static, "static text");
    TS_ASSERT_EQUALS(s2.size(), 11U);
    TS_ASSERT_EQUALS(s2.getNumAttributes(), 1U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(s2)).check(1);

    util::rich::Text s3 = s1 + s2 + "raw" + util::rich::Text("background").withNewAttribute(new util::rich::ColorAttribute(util::SkinColor::Background));
    TS_ASSERT_EQUALS(s3.size(), 36U);
    TS_ASSERT_EQUALS(s3.getNumAttributes(), 2U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(s3)).check(2);

    TS_ASSERT_EQUALS(s3.substr(10, 2).getNumAttributes(), 0U);
    TS_ASSERT_EQUALS(s3.substr(10, 3).getNumAttributes(), 1U);
    TS_ASSERT_EQUALS(s3.substr(12, 2).getNumAttributes(), 1U);
}

/** Test various construction methods. */
void
TestUtilRichText::testConstruction()
{
    // Empty
    util::rich::Text a1;
    TS_ASSERT(a1.empty());
    TS_ASSERT_EQUALS(a1.size(), 0U);
    TS_ASSERT_EQUALS(a1.length(), 0U);
    TS_ASSERT_EQUALS(a1.getText(), "");
    TS_ASSERT_EQUALS(a1.getNumAttributes(), 0U);

    // From NTBS
    util::rich::Text a2("x");
    TS_ASSERT(!a2.empty());
    TS_ASSERT_EQUALS(a2.size(), 1U);
    TS_ASSERT_EQUALS(a2.length(), 1U);
    TS_ASSERT_EQUALS(a2.getText(), "x");
    TS_ASSERT_EQUALS(a2.getNumAttributes(), 0U);

    // From string
    util::rich::Text a3(String_t("yz"));
    TS_ASSERT(!a3.empty());
    TS_ASSERT_EQUALS(a3.size(), 2U);
    TS_ASSERT_EQUALS(a3.length(), 2U);
    TS_ASSERT_EQUALS(a3.getText(), "yz");
    TS_ASSERT_EQUALS(a3.getNumAttributes(), 0U);

    // From color and NTBS
    util::rich::Text a4(util::SkinColor::Red, "red");
    TS_ASSERT(!a4.empty());
    TS_ASSERT_EQUALS(a4.size(), 3U);
    TS_ASSERT_EQUALS(a4.length(), 3U);
    TS_ASSERT_EQUALS(a4.getText(), "red");
    TS_ASSERT_EQUALS(a4.getNumAttributes(), 1U);

    // From color and empty NTBS
    util::rich::Text a5(util::SkinColor::Red, "");
    TS_ASSERT(a5.empty());
    TS_ASSERT_EQUALS(a5.size(), 0U);
    TS_ASSERT_EQUALS(a5.length(), 0U);
    TS_ASSERT_EQUALS(a5.getText(), "");
    TS_ASSERT_EQUALS(a5.getNumAttributes(), 0U);

    // From color and string
    util::rich::Text a6(util::SkinColor::Blue, String_t("blue"));
    TS_ASSERT(!a6.empty());
    TS_ASSERT_EQUALS(a6.size(), 4U);
    TS_ASSERT_EQUALS(a6.length(), 4U);
    TS_ASSERT_EQUALS(a6.getText(), "blue");
    TS_ASSERT_EQUALS(a6.getNumAttributes(), 1U);

    // From color and empty string
    util::rich::Text a7(util::SkinColor::Blue, String_t());
    TS_ASSERT(a7.empty());
    TS_ASSERT_EQUALS(a7.size(), 0U);
    TS_ASSERT_EQUALS(a7.length(), 0U);
    TS_ASSERT_EQUALS(a7.getText(), "");
    TS_ASSERT_EQUALS(a7.getNumAttributes(), 0U);

    // From other text
    util::rich::Text a8(a6);
    TS_ASSERT(!a8.empty());
    TS_ASSERT_EQUALS(a8.size(), 4U);
    TS_ASSERT_EQUALS(a8.length(), 4U);
    TS_ASSERT_EQUALS(a8.getText(), "blue");
    TS_ASSERT_EQUALS(a8.getNumAttributes(), 1U);

    // From other text range
    util::rich::Text a9(a6, 1, 2);
    TS_ASSERT(!a9.empty());
    TS_ASSERT_EQUALS(a9.size(), 2U);
    TS_ASSERT_EQUALS(a9.length(), 2U);
    TS_ASSERT_EQUALS(a9.getText(), "lu");
    TS_ASSERT_EQUALS(a9.getNumAttributes(), 1U);

    // From other text range, undelimited
    util::rich::Text a10(a6, 3);
    TS_ASSERT(!a10.empty());
    TS_ASSERT_EQUALS(a10.size(), 1U);
    TS_ASSERT_EQUALS(a10.length(), 1U);
    TS_ASSERT_EQUALS(a10.getText(), "e");
    TS_ASSERT_EQUALS(a10.getNumAttributes(), 1U);

    // From other text range, effectively empty
    util::rich::Text a11(a6, 4);
    TS_ASSERT(a11.empty());
    TS_ASSERT_EQUALS(a11.size(), 0U);
    TS_ASSERT_EQUALS(a11.length(), 0U);
    TS_ASSERT_EQUALS(a11.getText(), "");
    TS_ASSERT_EQUALS(a11.getNumAttributes(), 0U);
}

/** Test withNewAttribute() etc. */
void
TestUtilRichText::testWith()
{
    // Standard case
    {
        util::rich::Text t("x");
        TS_ASSERT_EQUALS(&t.withNewAttribute(new NullAttribute()), &t);
        TS_ASSERT_EQUALS(&t.withColor(util::SkinColor::Green), &t);
        TS_ASSERT_EQUALS(&t.withStyle(util::rich::StyleAttribute::Big), &t);
        TS_ASSERT_EQUALS(t.getText(), "x");
        TS_ASSERT_EQUALS(t.getNumAttributes(), 3U);
    }

    // Empty case
    {
        util::rich::Text t("");
        TS_ASSERT_EQUALS(&t.withNewAttribute(new NullAttribute()), &t);
        TS_ASSERT_EQUALS(&t.withColor(util::SkinColor::Green), &t);
        TS_ASSERT_EQUALS(&t.withStyle(util::rich::StyleAttribute::Big), &t);
        TS_ASSERT_EQUALS(t.getText(), "");
        TS_ASSERT_EQUALS(t.getNumAttributes(), 0U);
    }

    // Error case
    {
        util::rich::Text t("x");
        TS_ASSERT_EQUALS(&t.withNewAttribute(0), &t);
        TS_ASSERT_EQUALS(t.getText(), "x");
        TS_ASSERT_EQUALS(t.getNumAttributes(), 0U);
    }
}

/** Test string operations (erase, find, append). */
void
TestUtilRichText::testStringOps()
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
    TS_ASSERT_EQUALS(t.getText(), "aabbccddeeffgg");
    TS_ASSERT_EQUALS(t.size(), 14U);
    TS_ASSERT_EQUALS(t.length(), 14U);

    TS_ASSERT_EQUALS(t.find('a'), 0U);
    TS_ASSERT_EQUALS(t.find('b'), 2U);
    TS_ASSERT_EQUALS(t.find('g'), 12U);
    TS_ASSERT_EQUALS(t.find('x'), String_t::npos);

    TS_ASSERT_EQUALS(t[0], 'a');
    TS_ASSERT_EQUALS(t[1], 'a');
    TS_ASSERT_EQUALS(t[2], 'b');
    TS_ASSERT_EQUALS(t[13], 'g');
    TS_ASSERT_EQUALS(t[14], '\0');

    TS_ASSERT_EQUALS(t.getNumAttributes(), 3U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(t)).check(3);

    // Make a substring
    util::rich::Text sub1 = t.substr(3, 6);
    TS_ASSERT_EQUALS(sub1.getText(), "bccdde");
    TS_ASSERT_EQUALS(sub1.size(), 6U);
    TS_ASSERT_EQUALS(sub1.getNumAttributes(), 2U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(sub1)).check(2);

    // Make another substring
    util::rich::Text sub2 = t.substr(8);
    TS_ASSERT_EQUALS(sub2.getText(), "eeffgg");
    TS_ASSERT_EQUALS(sub2.size(), 6U);
    TS_ASSERT_EQUALS(sub2.getNumAttributes(), 2U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(sub2)).check(2);

    // Erase substring
    sub2.erase(1, 3);
    TS_ASSERT_EQUALS(sub2.getText(), "egg");
    TS_ASSERT_EQUALS(sub2.size(), 3U);
    TS_ASSERT_EQUALS(sub2.getNumAttributes(), 1U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(sub2)).check(1);

    // Swap
    sub1.swap(sub2);
    TS_ASSERT_EQUALS(sub1.getText(), "egg");
    TS_ASSERT_EQUALS(sub2.getText(), "bccdde");
    TS_ASSERT_EQUALS(sub1.getNumAttributes(), 1U);
    TS_ASSERT_EQUALS(sub2.getNumAttributes(), 2U);

    // Self-append
    t += t;
    TS_ASSERT_EQUALS(t.getText(), "aabbccddeeffggaabbccddeeffgg");
    TS_ASSERT_EQUALS(t.size(), 28U);
    TS_ASSERT_EQUALS(t.getNumAttributes(), 6U);
    static_cast<AttributeCounter&>(AttributeCounter().visit(t)).check(6);

    // Self-Swap
    t.swap(t);
    TS_ASSERT_EQUALS(t.getText(), "aabbccddeeffggaabbccddeeffgg");
    TS_ASSERT_EQUALS(t.getNumAttributes(), 6U);

    // Self-assignment
    t = t;
    TS_ASSERT_EQUALS(t.getText(), "aabbccddeeffggaabbccddeeffgg");
    TS_ASSERT_EQUALS(t.getNumAttributes(), 6U);

    // Clear
    t.clear();
    TS_ASSERT_EQUALS(t.getText(), "");
    TS_ASSERT_EQUALS(t.getNumAttributes(), 0U);
    TS_ASSERT(t.empty());

    // Append
    t += "a";
    TS_ASSERT_EQUALS(t.getText(), "a");
    TS_ASSERT_EQUALS(t.getNumAttributes(), 0U);

    t += String_t("b");
    TS_ASSERT_EQUALS(t.getText(), "ab");
    TS_ASSERT_EQUALS(t.getNumAttributes(), 0U);
}

