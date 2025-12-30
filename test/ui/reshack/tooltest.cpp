/**
  *  \file test/ui/reshack/tooltest.cpp
  *  \brief Test for ui::reshack::Tool
  */

#include "ui/reshack/tool.hpp"

#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST("ui.reshack.Tool", a)
{
    class Tester : public ui::reshack::Tool {
     public:
        Tester()
            : Tool(true, "n")
            { }
        virtual void click(gfx::BaseContext& /*ctx*/, gfx::Point /*pt*/, gfx::Color_t /*bg*/)
            { }
        virtual void drag(gfx::BaseContext& /*ctx*/, gfx::Point /*pt*/)
            { }
        virtual void release(gfx::BaseContext& /*ctx*/, gfx::Point /*pt*/)
            { }
        virtual bool isUsable()
            { return false; }
    };
    Tester t;

    a.checkEqual("needsPreview", t.needsPreview(), true);
    a.checkEqual("name", t.getName(), "n");
}

