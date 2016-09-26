/**
  *  \file u/t_ui_res_provider.cpp
  *  \brief Test for ui::res::Provider
  */

#include "ui/res/provider.hpp"

#include "t_ui_res.hpp"

/** Interface test. */
void
TestUiResProvider::testIt()
{
    class Tester : public ui::res::Provider {
     public:
        virtual afl::base::Ptr<gfx::Canvas> loadImage(String_t /*name*/, ui::res::Manager& /*mgr*/)
            { return 0; }
    };
    Tester t;
}

