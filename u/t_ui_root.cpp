/**
  *  \file u/t_ui_root.cpp
  *  \brief Test for ui::Root
  */

#include "ui/root.hpp"

#include "t_ui.hpp"
#include "ui/invisiblewidget.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"

/** Test interaction of various channels that generate key events. */
void
TestUiRoot::testKeys()
{
    // A widget that collects keystrokes
    class CollectorWidget : public ui::InvisibleWidget {
     public:
        virtual bool handleKey(util::Key_t key, int prefix)
            {
                TS_ASSERT(key >= 'a' && key <= 'z');
                TS_ASSERT_EQUALS(prefix, 0);
                m_accumulator += char(key);
                return true;
            }

        const String_t& get() const
            { return m_accumulator; }

     private:
        String_t m_accumulator;
    };
    
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, 100, 100, 8, gfx::Engine::WindowFlags_t());

    // Test widget
    CollectorWidget w;
    root.add(w);
    TS_ASSERT_EQUALS(w.get(), "");

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
        TS_ASSERT(i < 20);
        ++i;
        root.handleEvent();
    }

    // Verify result
    TS_ASSERT_EQUALS(w.get(), "fcbead");
}
