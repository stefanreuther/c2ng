/**
  *  \file u/t_ui_prefixargument.cpp
  *  \brief Test for ui::PrefixArgument
  */

#include "ui/prefixargument.hpp"

#include "t_ui.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/group.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/root.hpp"

/** Basic test.
    This sets up a "dialog" containing a PrefixArgument and fires a prefix key sequence into it.
    A widget in the dialog must receive an appropriately prefixed key event. */
void
TestUiPrefixArgument::testIt()
{
    // UI stuff
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, 100, 100, 8, gfx::Engine::WindowFlags_t());

    // A simple widget
    class TestWidget : public ui::InvisibleWidget {
     public:
        TestWidget()
            : m_sum(0)
            { }
        virtual bool handleKey(util::Key_t key, int prefix)
            {
                if (key == 'p') {
                    m_sum += prefix;
                    return true;
                } else {
                    return false;
                }
            }

        int get() const
            { return m_sum; }
     private:
        int m_sum;
    };

    // Make a "window" containing the TestWidget and a PrefixArgument
    ui::Group g(ui::layout::HBox::instance0);
    ui::PrefixArgument testee(root);
    TestWidget checker;
    g.add(testee);
    g.add(checker);

    // Put them on the root
    root.add(g);

    // Post some keys
    TS_ASSERT_EQUALS(checker.get(), 0);
    engine.postKey('1', 0);
    engine.postKey('2', 0);
    engine.postKey('*', 0);
    engine.postKey('9', 0);
    engine.postKey('p', 0);
    for (int i = 0; i < 20; ++i) {
        if (checker.get() != 0) {
            break;
        }
        root.handleEvent();
    }

    // Check result
    TS_ASSERT_EQUALS(checker.get(), 12*9);
}
