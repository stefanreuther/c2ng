/**
  *  \file test/ui/invisiblewidgettest.cpp
  *  \brief Test for ui::InvisibleWidget
  */

#include "ui/invisiblewidget.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("ui.InvisibleWidget")
{
    class MyWidget : public ui::InvisibleWidget {
     public:
        virtual bool handleKey(util::Key_t /*key*/, int /*prefix*/)
            { return false; }
    };
    MyWidget t;
}
