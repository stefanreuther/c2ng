/**
  *  \file u/t_ui_rich_blockobject.cpp
  *  \brief Test for ui::rich::BlockObject
  */

#include "ui/rich/blockobject.hpp"

#include "t_ui_rich.hpp"

void
TestUiRichBlockObject::testIt()
{
    /* Interface test */
    class Tester : public ui::rich::BlockObject {
     public:
        virtual gfx::Point getSize()
            { return gfx::Point(); }
        virtual void draw(gfx::Context<util::SkinColor::Color>& /*ctx*/, gfx::Rectangle /*area*/)
            { }
    };
    Tester t;
}
