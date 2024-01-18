/**
  *  \file test/ui/layoutablegrouptest.cpp
  *  \brief Test for ui::LayoutableGroup
  */

#include "ui/layoutablegroup.hpp"

#include "afl/test/testrunner.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/spacer.hpp"

/** Simple test. */
AFL_TEST("ui.LayoutableGroup", a)
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
    a.checkEqual("01. group",     t.getExtent(), gfx::Rectangle(0, 0, 60, 65));
    a.checkEqual("02. content",   content.getExtent(), gfx::Rectangle(10, 5, 40, 55));
    a.checkEqual("03. getWidth",  content.getExtent().getWidth(), 40);
    a.checkEqual("04. getHeight", content.getExtent().getHeight(), 55);

    // Set fixed layout
    t.setExtent(gfx::Rectangle(20, 30, 100, 120));
    a.checkEqual("11. getExtent", content.getExtent(), gfx::Rectangle(30, 35, 80, 110));
}
