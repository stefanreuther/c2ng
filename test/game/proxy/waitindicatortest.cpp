/**
  *  \file test/game/proxy/waitindicatortest.cpp
  *  \brief Test for game::proxy::WaitIndicator
  */

#include "game/proxy/waitindicator.hpp"

#include "afl/test/testrunner.hpp"
#include "util/requestdispatcher.hpp"
#include "util/simplerequestdispatcher.hpp"

/** Interface test.
    Actual functionality is tested by other tests;
    this only tests the interface for well-formedness. */
AFL_TEST_NOARG("game.proxy.WaitIndicator")
{
    // Test object
    class Tester : public game::proxy::WaitIndicator {
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

