/**
  *  \file u/t_util_rich_attribute.cpp
  *  \brief Test for util::rich::Attribute
  */

#include "util/rich/attribute.hpp"

#include "t_util_rich.hpp"

/** Simple interface test. */
void
TestUtilRichAttribute::testIt()
{
    class TestAttribute : public util::rich::Attribute {
     public:
        virtual TestAttribute * clone() const
            { return new TestAttribute(); }
    };
    TestAttribute att;
}

