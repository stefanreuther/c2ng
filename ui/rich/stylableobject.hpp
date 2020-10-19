/**
  *  \file ui/rich/stylableobject.hpp
  */
#ifndef C2NG_UI_RICH_STYLABLEOBJECT_HPP
#define C2NG_UI_RICH_STYLABLEOBJECT_HPP

#include <memory>
#include "ui/rich/blockobject.hpp"
#include "afl/base/optional.hpp"
#include "gfx/types.hpp"
#include "ui/draw.hpp"

namespace ui { namespace rich {

    class StylableObject : public BlockObject {
     public:
        StylableObject(std::auto_ptr<BlockObject> content, ColorScheme& colors);
        ~StylableObject();

        void setPaddingBefore(gfx::Point p);
        void setPaddingAfter(gfx::Point p);
        void setMarginBefore(gfx::Point p);
        void setMarginAfter(gfx::Point p);
        void setBackgroundColor(gfx::Color_t color);
        void setFrameWidth(int width);
        void setFrameType(FrameType type);

        virtual gfx::Point getSize();
        virtual void draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area);

     private:
        std::auto_ptr<BlockObject> m_content;
        ColorScheme& m_colors;
        gfx::Point m_paddingBefore;
        gfx::Point m_paddingAfter;
        gfx::Point m_marginBefore;
        gfx::Point m_marginAfter;
        afl::base::Optional<gfx::Color_t> m_backgroundColor;
        FrameType m_frameType;
        int m_frameWidth;
    };

} }

#endif
