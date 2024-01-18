/**
  *  \file test/ui/res/managertest.cpp
  *  \brief Test for ui::res::Manager
  */

#include "ui/res/manager.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/rgbapixmap.hpp"
#include "ui/res/provider.hpp"

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
AFL_TEST("ui.res.Manager:basics", a)
{
    ui::res::Manager t;

    // set/get
    t.setScreenSize(gfx::Point(100, 120));
    a.checkEqual("01. getScreenSize", t.getScreenSize(), gfx::Point(100, 120));

    // load
    afl::base::Ptr<gfx::Canvas> c;
    c = t.loadImage("foo");
    a.checkNull("11. loadImage", c.get());
}


/** Test loading. */
AFL_TEST("ui.res.Manager:loadImage", a)
{
    // Add some providers
    ui::res::Manager t;
    t.addNewProvider(new TestProvider("a", COLORQUAD_FROM_RGB(1,1,1)), "a");
    t.addNewProvider(new TestProvider("b", COLORQUAD_FROM_RGB(2,2,2)), "a");
    t.addNewProvider(new TestProvider("c", COLORQUAD_FROM_RGB(3,3,3)), "a");

    // Verify that we can access all three
    afl::base::Ptr<gfx::Canvas> can = t.loadImage("a");
    gfx::ColorQuad_t tmp[1];
    a.checkNonNull("01. load a", can.get());
    can->getPixels(gfx::Point(0, 0), tmp);
    a.checkEqual("02. color a", tmp[0], COLORQUAD_FROM_RGB(1,1,1));

    can = t.loadImage("b");
    a.checkNonNull("11. load b", can.get());
    can->getPixels(gfx::Point(0, 0), tmp);
    a.checkEqual("12. color b", tmp[0], COLORQUAD_FROM_RGB(2,2,2));

    can = t.loadImage("c");
    a.checkNonNull("21. load c", can.get());
    can->getPixels(gfx::Point(0, 0), tmp);
    a.checkEqual("22. color c", tmp[0], COLORQUAD_FROM_RGB(3,3,3));

    // Verify that for a conflicting name, last wins (also, no caching)
    t.addNewProvider(new TestProvider("a", COLORQUAD_FROM_RGB(4,4,4)), "a");
    can = t.loadImage("a");
    a.checkNonNull("31. last load a", can.get());
    can->getPixels(gfx::Point(0, 0), tmp);
    a.checkEqual("32. last color a", tmp[0], COLORQUAD_FROM_RGB(4,4,4));

    // Invalid
    can = t.loadImage("");
    a.checkNull("41. load unknown", can.get());

    can = t.loadImage("a.a");
    a.checkNull("51. load unknown", can.get());
}

/** Test removal of providers. */
AFL_TEST("ui.res.Manager:removeProvidersByKey", a)
{
    ui::res::Manager t;
    t.addNewProvider(new TestProvider("a", COLORQUAD_FROM_RGB(1,1,1)), "a");
    t.addNewProvider(new TestProvider("a", COLORQUAD_FROM_RGB(2,2,2)), "b");
    t.addNewProvider(new TestProvider("b", COLORQUAD_FROM_RGB(3,3,3)), "b");
    t.addNewProvider(new TestProvider("b", COLORQUAD_FROM_RGB(4,4,4)), "a");

    // Initial access
    afl::base::Ptr<gfx::Canvas> can = t.loadImage("a");
    gfx::ColorQuad_t tmp[1];
    a.checkNonNull("01. load a", can.get());
    can->getPixels(gfx::Point(0, 0), tmp);
    a.checkEqual("02. color a", tmp[0], COLORQUAD_FROM_RGB(2,2,2));
    can = t.loadImage("b");
    a.checkNonNull("03. load b", can.get());
    can->getPixels(gfx::Point(0, 0), tmp);
    a.checkEqual("04. color b", tmp[0], COLORQUAD_FROM_RGB(4,4,4));

    // Remove series 'b' (second parameter)
    t.removeProvidersByKey("b");
    can = t.loadImage("a");
    a.checkNonNull("11. load a", can.get());
    can->getPixels(gfx::Point(0, 0), tmp);
    a.checkEqual("12. color a", tmp[0], COLORQUAD_FROM_RGB(1,1,1));
    can = t.loadImage("b");
    a.checkNonNull("13. load a", can.get());
    can->getPixels(gfx::Point(0, 0), tmp);
    a.checkEqual("14. color a", tmp[0], COLORQUAD_FROM_RGB(4,4,4));
}
