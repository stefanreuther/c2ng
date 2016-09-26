/**
  *  \file u/t_gfx_resourceprovider.cpp
  *  \brief Test for gfx::ResourceProvider
  */

#include "gfx/resourceprovider.hpp"

#include "t_gfx.hpp"

/** Interface test. */
void
TestGfxResourceProvider::testIt()
{
    class Tester : public gfx::ResourceProvider {
     public:
        virtual afl::base::Ptr<gfx::Canvas> getImage(String_t /*name*/, bool* /*status*/)
            { return 0; }
        virtual afl::base::Ptr<gfx::Font> getFont(gfx::FontRequest /*req*/)
            { return 0; }
    };
    Tester t;
}
