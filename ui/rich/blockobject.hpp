/**
  *  \file ui/rich/blockobject.hpp
  */
#ifndef C2NG_UI_RICH_BLOCKOBJECT_HPP
#define C2NG_UI_RICH_BLOCKOBJECT_HPP

#include "afl/base/deletable.hpp"
#include "gfx/point.hpp"
#include "gfx/context.hpp"
#include "gfx/rectangle.hpp"

namespace ui { namespace rich {

    /** Rich Document Block Object.
        This is a block object, such as an image, which is displayed in a Document. */
    class BlockObject : public afl::base::Deletable {
     public:
        virtual gfx::Point getSize() = 0;
        virtual void draw(gfx::Context& ctx, gfx::Rectangle area) = 0;
    };

} }

#endif
