/**
  *  \file test/ui/res/imageloadertest.cpp
  *  \brief Test for ui::res::ImageLoader
  */

#include "ui/res/imageloader.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("ui.res.ImageLoader")
{
    class Tester : public ui::res::ImageLoader {
     public:
        virtual afl::base::Ptr<gfx::Canvas> loadImage(afl::io::Stream& /*in*/)
            { return 0; }
    };
    Tester t;
}
