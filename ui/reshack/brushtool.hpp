/**
  *  \file ui/reshack/brushtool.hpp
  *  \brief Class ui::reshack::BrushTool
  */
#ifndef C2NG_UI_RESHACK_BRUSHTOOL_HPP
#define C2NG_UI_RESHACK_BRUSHTOOL_HPP

#include "ui/reshack/tool.hpp"
#include "afl/string/translator.hpp"

namespace ui { namespace reshack {

    /** Brush tool.
        Draws a big blot of color for every movement. */
    class BrushTool : public Tool {
     public:
        /** Constructor.
            @param tx Translator (for generating name) */
        explicit BrushTool(afl::string::Translator& tx);

        // Tool:
        void click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t bg);
        void drag(gfx::BaseContext& c, gfx::Point pt);
        void release(gfx::BaseContext& c, gfx::Point pt);
        bool isUsable();
    };

} }

#endif
