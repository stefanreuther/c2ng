/**
  *  \file u/t_ui_res_imageloader.cpp
  *  \brief Test for ui::res::ImageLoader
  */

#include "ui/res/imageloader.hpp"

#include "t_ui_res.hpp"

/** Interface test. */
void
TestUiResImageLoader::testIt()
{
    class Tester : public ui::res::ImageLoader {
     public:
        virtual afl::base::Ptr<gfx::Canvas> loadImage(afl::io::Stream& /*in*/)
            { return 0; }
    };
    Tester t;
}

