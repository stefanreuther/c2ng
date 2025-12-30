/**
  *  \file ui/reshack/rectangletool.hpp
  *  \brief Class ui::reshack::RectangleTool
  */
#ifndef C2NG_UI_RESHACK_RECTANGLETOOL_HPP
#define C2NG_UI_RESHACK_RECTANGLETOOL_HPP

#include "afl/string/translator.hpp"
#include "gfx/rectangle.hpp"
#include "ui/reshack/tool.hpp"

namespace ui { namespace reshack {

    /** Rectangle tool.
        Draws a solid or hollow rectangle from initial click to mouse release point. */
    class RectangleTool : public Tool {
     public:
        /** Constructor.
            @param tx     Translator (for generating name)
            @param solid  true to draw solid rectangles, false for hollow */
        explicit RectangleTool(afl::string::Translator& tx, bool solid);

        // Tool:
        void click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t bg);
        void drag(gfx::BaseContext& c, gfx::Point pt);
        void release(gfx::BaseContext& c, gfx::Point pt);
        bool isUsable();

        /** Make rectangle covering two points.
            @param a First point
            @param b Second point
            @return rectangle */
        static gfx::Rectangle makeRectangle(gfx::Point a, gfx::Point b);

     private:
        const bool m_solid;
        gfx::Point m_prev;
    };

} }

#endif
