/**
  *  \file test/util/waitindicatortest.cpp
  *  \brief Test for util::WaitIndicator
  */

#include "util/waitindicator.hpp"

#include "afl/test/testrunner.hpp"
#include "util/requestdispatcher.hpp"
#include "util/simplerequestdispatcher.hpp"

/** Interface test.
    Actual functionality is tested by other tests;
    this only tests the interface for well-formedness. */
AFL_TEST_NOARG("util.WaitIndicator")
{
    // Test object
    class Tester : public util::WaitIndicator {
     public:
        Tester(util::RequestDispatcher& disp)
            : WaitIndicator(disp)
            { }
        virtual void post(bool /*success*/)
            { }
        virtual bool wait()
            { return false; }
    };
    util::SimpleRequestDispatcher disp;
    Tester t(disp);
}
