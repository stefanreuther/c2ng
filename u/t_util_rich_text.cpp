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
