/**
  *  \file ui/reshack/circletool.hpp
  *  \brief Class ui::reshack::CircleTool
  */
#ifndef C2NG_UI_RESHACK_CIRCLETOOL_HPP
#define C2NG_UI_RESHACK_CIRCLETOOL_HPP

#include "afl/string/translator.hpp"
#include "ui/reshack/tool.hpp"

namespace ui { namespace reshack {

    /** Circle tool.
        Draws a circle with the radius given by the distance between initial click and release. */
    class CircleTool : public Tool {
     public:
        /** Constructor.
            @param tx Translator (for generating name) */
        explicit CircleTool(afl::string::Translator& tx);

        // Tool:
        void click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t bg);
        void drag(gfx::BaseContext& c, gfx::Point pt);
        void release(gfx::BaseContext& c, gfx::Point pt);
        bool isUsable();

     private:
        gfx::Point m_prev;
    };

} }

#endif
