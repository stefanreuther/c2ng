/**
  *  \file test/ui/roottest.cpp
  *  \brief Test for ui::Root
  */

#include "ui/root.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/invisiblewidget.hpp"

/** Test interaction of various channels that generate key events. */
AFL_TEST("ui.Root", a)
{
    // A widget that collects keystrokes
    class CollectorWidget : public ui::InvisibleWidget {
     public:
        CollectorWidget(afl::test::Assert a)
            : m_assert(a)
            { }
        virtual bool handleKey(util::Key_t key, int prefix)
            {
                m_assert.check("01", key >= 'a' && key <= 'z');
                m_assert.checkEqual("02. prefix", prefix, 0);
                m_accumulator += char(key);
                return true;
            }

        const String_t& get() const
            { return m_accumulator; }

     private:
        afl::test::Assert m_assert;
        String_t m_accumulator;
    };

    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Test widget
    CollectorWidget w(a);
    root.add(w);
    a.checkEqual("11. get", w.get(), "");

    // Post some key events through various channels
    engine.postKey('a', 0);
    root.postKeyEvent('b', 0);
    root.ungetKeyEvent('c', 0);
    engine.postKey('d', 0);
    root.postKeyEvent('e', 0);
    root.ungetKeyEvent('f', 0);

    // Handle events
    int i = 0;
    while (w.get().size() < 6) {
        a.check("21. handleEvent loop", i < 20);
        ++i;
        root.handleEvent();
    }

    // Verify result
    a.checkEqual("31. get", w.get(), "fcbead");
}
