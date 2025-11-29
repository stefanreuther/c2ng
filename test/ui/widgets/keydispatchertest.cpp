/**
  *  \file test/ui/widgets/keydispatchertest.cpp
  *  \brief Test for ui::widgets::KeyDispatcher
  */

#include "ui/widgets/keydispatcher.hpp"

#include "afl/test/testrunner.hpp"
#include "game/test/counter.hpp"

using ui::widgets::KeyDispatcher;
using game::test::Counter;

namespace {
    int globalCounter;
    void globalIncrement(int n)
    {
        globalCounter += n;
    }
}

// Test 2-argument add()
AFL_TEST("ui.widgets.KeyDispatcher:add/2", a)
{
    KeyDispatcher testee;
    testee.add('a', &globalIncrement);
    globalCounter = 0;

    a.check("good key", testee.handleKey('a', 100));
    a.check("bad key", !testee.handleKey('b', 200));
    a.checkEqual("count", globalCounter, 100);
}

// Test 3-argument add()
AFL_TEST("ui.widgets.KeyDispatcher:add/3", a)
{
    KeyDispatcher testee;
    Counter ctr;
    testee.add('x', &ctr, &Counter::increment);

    a.check("good key", testee.handleKey('x', 100));
    a.check("bad key", !testee.handleKey('y', 100));
    a.checkEqual("count", ctr.get(), 1);
}

// Test addNewClosure()
AFL_TEST("ui.widgets.KeyDispatcher:addNewClosure", a)
{
    class Closure : public KeyDispatcher::Closure_t {
     public:
        Closure(int& n)
            : m_n(n)
            { }
        void call(int n)
            { m_n += n; }
     private:
        int& m_n;
    };

    KeyDispatcher testee;
    int count = 7;
    testee.addNewClosure('z', new Closure(count));

    a.check("good key", testee.handleKey('z', 100));
    a.check("bad key", !testee.handleKey('q', 100));
    a.checkEqual("count", count, 107);
}
