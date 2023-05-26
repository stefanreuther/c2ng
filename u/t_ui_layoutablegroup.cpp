/**
  *  \file u/t_ui_layoutablegroup.cpp
  *  \brief Test for ui::LayoutableGroup
  */

#include "ui/layoutablegroup.hpp"

#include "t_ui.hpp"
#include "ui/spacer.hpp"
#include "ui/layout/hbox.hpp"

/** Simple test. */
void
TestUiLayoutableGroup::testIt()
{
    // Test implementation
    class Tester : public ui::LayoutableGroup {
     public:
        Tester()
            : LayoutableGroup(ui::layout::HBox::instance5)
            { }
        virtual gfx::Rectangle transformSize(gfx::Rectangle size, Transformation kind) const
            {
                if (kind == OuterToInner) {
                    size.grow(-10, -5);
                } else {
                    size.grow(+10, +5);
                }
                return size;
            }
        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
            { return defaultHandleMouse(pt, pressedButtons); }
        virtual void draw(gfx::Canvas& can)
            { defaultDrawChildren(can); }
        virtual void handleStateChange(State, bool)
            { }
    };

    Tester t;

    // Add a widget with given layout
    ui::Spacer content(ui::layout::Info(gfx::Point(40, 55), ui::layout::Info::GrowBoth));
    t.add(content);

    // Verify layout produced by pack()
    t.pack();
    TS_ASSERT_EQUALS(t.getExtent(), gfx::Rectangle(0, 0, 60, 65));
    TS_ASSERT_EQUALS(content.getExtent(), gfx::Rectangle(10, 5, 40, 55));
    TS_ASSERT_EQUALS(content.getExtent().getWidth(), 40);
    TS_ASSERT_EQUALS(content.getExtent().getHeight(), 55);

    // Set fixed layout
    t.setExtent(gfx::Rectangle(20, 30, 100, 120));
    TS_ASSERT_EQUALS(content.getExtent(), gfx::Rectangle(30, 35, 80, 110));
}

