/**
  *  \file test/util/rich/attributetest.cpp
  *  \brief Test for util::rich::Attribute
  */

#include "util/rich/attribute.hpp"
#include "afl/test/testrunner.hpp"

/** Simple interface test. */
AFL_TEST_NOARG("util.rich.Attribute")
{
    class TestAttribute : public util::rich::Attribute {
     public:
        virtual TestAttribute * clone() const
            { return new TestAttribute(); }
    };
    TestAttribute att;
}
