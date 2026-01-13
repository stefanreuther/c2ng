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

/*
 *  PaletteHandler
 */

namespace {
    class PHImpl : public ui::Root::PaletteHandler {
     public:
        PHImpl(String_t& acc, String_t name)
            : m_acc(acc), m_name(name)
            { }
        void loadPalette(ui::Root::PaletteLoader&)
            { m_acc += "load " + m_name + "."; }
        void unloadPalette()
            { m_acc += "unload " + m_name + "."; }
     private:
        String_t& m_acc;
        String_t m_name;
    };
}

/* Simple sequence */
AFL_TEST("ui.Root:PaletteHandler:simple", a)
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Add
    String_t acc;
    PHImpl ph(acc, "a");
    root.addPaletteHandler(ph);
    a.checkEqual("after add", acc, "load a.");

    // Remove
    acc.clear();
    root.removePaletteHandler(ph);
    a.checkEqual("after remove", acc, "");
}

/* Multiple PHs, symmetric */
AFL_TEST("ui.Root:PaletteHandler:symmetric", a)
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Add
    String_t acc;
    PHImpl ph1(acc, "a");
    PHImpl ph2(acc, "b");
    PHImpl ph3(acc, "c");
    root.addPaletteHandler(ph1);
    root.addPaletteHandler(ph2);
    root.addPaletteHandler(ph3);
    a.checkEqual("after add", acc, "load a.load b.unload a.load c.unload a.unload b.");

    // Update
    acc.clear();
    root.updatePalette(ph3);
    a.checkEqual("after update 3", acc, "load c.unload a.unload b.");

    acc.clear();
    root.updatePalette(ph1);
    a.checkEqual("after update 1", acc, "");

    // Remove
    acc.clear();
    root.removePaletteHandler(ph3);
    root.removePaletteHandler(ph2);
    root.removePaletteHandler(ph1);
    a.checkEqual("after remove", acc, "load b.unload a.load a.");
}

/* Multiple PHs, asymmetric */
AFL_TEST("ui.Root:PaletteHandler:asymmetric", a)
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Add and remove a PaletteHandler
    String_t acc;
    PHImpl ph1(acc, "a");
    PHImpl ph2(acc, "b");
    root.addPaletteHandler(ph1);
    root.addPaletteHandler(ph2);
    a.checkEqual("after add", acc, "load a.load b.unload a.");

    // Removing ph1 does not trigger a callback because it is invisible; removing ph2 does not trigger because there is none left.
    acc.clear();
    root.removePaletteHandler(ph1);
    root.removePaletteHandler(ph2);
    a.checkEqual("after remove", acc, "");
}

/* Palette setting */
AFL_TEST("ui.Root:PaletteHandler:loadPalette", a)
{
    class LocalPHImpl : public ui::Root::PaletteHandler {
     public:
        void loadPalette(ui::Root::PaletteLoader& ldr)
            {
                const gfx::ColorQuad_t q[] = { COLORQUAD_FROM_RGB(200, 0, 0), COLORQUAD_FROM_RGB(0, 0, 100) };
                ldr.setPalette(210, q);
            }
        void unloadPalette()
            { }
    };

    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Add a PaletteHandler
    LocalPHImpl ph;
    root.addPaletteHandler(ph);

    // Verify that palette was loaded
    a.checkEqual("color 210", root.colorScheme().getColor(210), COLORQUAD_FROM_RGB(200, 0, 0));
    a.checkEqual("color 211", root.colorScheme().getColor(211), COLORQUAD_FROM_RGB(0, 0, 100));
}

/* Palette setting, out of range */
AFL_TEST("ui.Root:PaletteHandler:loadPalette:out-of-range", a)
{
    class LocalPHImpl : public ui::Root::PaletteHandler {
     public:
        void loadPalette(ui::Root::PaletteLoader& ldr)
            {
                const gfx::ColorQuad_t q[] = { COLORQUAD_FROM_RGB(200, 0, 0), COLORQUAD_FROM_RGB(0, 0, 100) };
                ldr.setPalette(ui::Color_Avail-1, q);
            }
        void unloadPalette()
            { }
    };

    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Add a PaletteHandler
    LocalPHImpl ph;
    root.addPaletteHandler(ph);

    // Verify that palette was loaded
    a.checkDifferent("color 159", root.colorScheme().getColor(ui::Color_Avail-1), COLORQUAD_FROM_RGB(200, 0, 0));
    a.checkEqual("color 160", root.colorScheme().getColor(ui::Color_Avail), COLORQUAD_FROM_RGB(0, 0, 100));
}
