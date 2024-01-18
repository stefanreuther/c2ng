/**
  *  \file test/util/rich/visitortest.cpp
  *  \brief Test for util::rich::Visitor
  */

#include "util/rich/visitor.hpp"

#include "afl/test/testrunner.hpp"
#include "util/rich/text.hpp"

/** Simple test. */
AFL_TEST("util.rich.Visitor", a)
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

    a.checkEqual("01", &v.visit("hi"), static_cast<util::rich::Visitor*>(&v));
}
