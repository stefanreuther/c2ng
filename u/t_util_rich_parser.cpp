/**
  *  \file u/t_util_rich_parser.cpp
  *  \brief Test for util::rich::Parser
  */

#include "util/rich/parser.hpp"

#include "t_util_rich.hpp"
#include "util/rich/text.hpp"
#include "util/rich/visitor.hpp"
#include "util/rich/styleattribute.hpp"

/** Test the "parseXml" function. */
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

