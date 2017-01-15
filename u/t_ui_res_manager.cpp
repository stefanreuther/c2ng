/**
  *  \file u/t_ui_res_manager.cpp
  *  \brief Test for ui::res::Manager
  */

#include "ui/res/manager.hpp"

#include "t_ui_res.hpp"
#include "ui/res/provider.hpp"
#include "gfx/rgbapixmap.hpp"

namespace {
    class TestProvider : public ui::res::Provider {
     public:
        TestProvider(String_t name, gfx::ColorQuad_t color)
            : m_name(name),
              m_color(color)
            { }

        virtual afl::base::Ptr<gfx::Canvas> loadImage(String_t name, ui::res::Manager& /*mgr*/)
            {
                afl::base::Ptr<gfx::Canvas> result;
                if (name == m_name) {
                    result = gfx::RGBAPixmap::create(1, 1)->makeCanvas().asPtr();
                    result->drawPixel(gfx::Point(0, 0), m_color, gfx::OPAQUE_ALPHA);
                }
                return result;
            }
     private:
        const String_t m_name;
        const gfx::ColorQuad_t m_color;
    };
}

/** Simple test.
    For now, test just the idle state. */
void
TestUiResManager::testIt()
{
    ui::res::Manager t;

    // set/get
    t.setScreenSize(gfx::Point(100, 120));
    TS_ASSERT_EQUALS(t.getScreenSize(), gfx::Point(100, 120));

    // load
    afl::base::Ptr<gfx::Canvas> c;
    c = t.loadImage("foo");
    TS_ASSERT(c.get() == 0);
}


/** Test loading. */
void
TestUiResManager::testLoad()
{
    // Add some providers
    ui::res::Manager t;
    t.addNewProvider(new TestProvider("a", COLORQUAD_FROM_RGB(1,1,1)), "a");
    t.addNewProvider(new TestProvider("b", COLORQUAD_FROM_RGB(2,2,2)), "a");
    t.addNewProvider(new TestProvider("c", COLORQUAD_FROM_RGB(3,3,3)), "a");

    // Verify that we can access all three
    afl::base::Ptr<gfx::Canvas> can = t.loadImage("a");
    gfx::ColorQuad_t tmp[1];
    TS_ASSERT(can.get() != 0);
    can->getPixels(gfx::Point(0, 0), tmp);
    TS_ASSERT_EQUALS(tmp[0], COLORQUAD_FROM_RGB(1,1,1));

    can = t.loadImage("b");
    TS_ASSERT(can.get() != 0);
    can->getPixels(gfx::Point(0, 0), tmp);
    TS_ASSERT_EQUALS(tmp[0], COLORQUAD_FROM_RGB(2,2,2));

    can = t.loadImage("c");
    TS_ASSERT(can.get() != 0);
    can->getPixels(gfx::Point(0, 0), tmp);
    TS_ASSERT_EQUALS(tmp[0], COLORQUAD_FROM_RGB(3,3,3));

    // Verify that for a conflicting name, last wins (also, no caching)
    t.addNewProvider(new TestProvider("a", COLORQUAD_FROM_RGB(4,4,4)), "a");
    can = t.loadImage("a");
    TS_ASSERT(can.get() != 0);
    can->getPixels(gfx::Point(0, 0), tmp);
    TS_ASSERT_EQUALS(tmp[0], COLORQUAD_FROM_RGB(4,4,4));

    // Invalid
    can = t.loadImage("");
    TS_ASSERT(can.get() == 0);

    can = t.loadImage("a.a");
    TS_ASSERT(can.get() == 0);
}

/** Test removal of providers. */
void
TestUiResManager::testRemove()
{
    ui::res::Manager t;
    t.addNewProvider(new TestProvider("a", COLORQUAD_FROM_RGB(1,1,1)), "a");
    t.addNewProvider(new TestProvider("a", COLORQUAD_FROM_RGB(2,2,2)), "b");
    t.addNewProvider(new TestProvider("b", COLORQUAD_FROM_RGB(3,3,3)), "b");
    t.addNewProvider(new TestProvider("b", COLORQUAD_FROM_RGB(4,4,4)), "a");

    // Initial access
    afl::base::Ptr<gfx::Canvas> can = t.loadImage("a");
    gfx::ColorQuad_t tmp[1];
    TS_ASSERT(can.get() != 0);
    can->getPixels(gfx::Point(0, 0), tmp);
    TS_ASSERT_EQUALS(tmp[0], COLORQUAD_FROM_RGB(2,2,2));
    can = t.loadImage("b");
    TS_ASSERT(can.get() != 0);
    can->getPixels(gfx::Point(0, 0), tmp);
    TS_ASSERT_EQUALS(tmp[0], COLORQUAD_FROM_RGB(4,4,4));

    // Remove series 'b' (second parameter)
    t.removeProvidersByKey("b");
    can = t.loadImage("a");
    TS_ASSERT(can.get() != 0);
    can->getPixels(gfx::Point(0, 0), tmp);
    TS_ASSERT_EQUALS(tmp[0], COLORQUAD_FROM_RGB(1,1,1));
    can = t.loadImage("b");
    TS_ASSERT(can.get() != 0);
    can->getPixels(gfx::Point(0, 0), tmp);
    TS_ASSERT_EQUALS(tmp[0], COLORQUAD_FROM_RGB(4,4,4));
}
