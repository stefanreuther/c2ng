/**
  *  \file u/t_util_rich_visitor.cpp
  *  \brief Test for util::rich::Visitor
  */

#include "util/rich/visitor.hpp"

#include "t_util_rich.hpp"
#include "util/rich/text.hpp"

/** Simple test. */
void
TestUtilRichVisitor::testIt()
{
    class MyVisitor : public util::rich::Visitor {
        virtual bool handleText(String_t /*text*/)
            { return true; }
        virtual bool startAttribute(const util::rich::Attribute& /*att*/)
            { return true; }
        virtual bool endAttribute(const util::rich::Attribute& /*att*/)
            { return true; }
    };

    // Can be instantiated
    MyVisitor v;

    // Can be called
    v.visit("hi");
    v.visit(util::rich::Text());

    TS_ASSERT_EQUALS(&v.visit("hi"), static_cast<util::rich::Visitor*>(&v));
}

