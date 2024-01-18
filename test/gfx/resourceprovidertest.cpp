/**
  *  \file test/gfx/resourceprovidertest.cpp
  *  \brief Test for gfx::ResourceProvider
  */

#include "gfx/resourceprovider.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("gfx.ResourceProvider")
{
    class Tester : public gfx::ResourceProvider {
     public:
        virtual afl::base::Ptr<gfx::Canvas> getImage(String_t /*name*/, bool* /*status*/)
            { return 0; }
        virtual afl::base::Ref<gfx::Font> getFont(gfx::FontRequest /*req*/)
            { throw "egal"; }
    };
    Tester t;
}
