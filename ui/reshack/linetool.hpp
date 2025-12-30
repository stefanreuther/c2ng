/**
  *  \file ui/reshack/linetool.hpp
  *  \brief Class ui::reshack::LineTool
  */
#ifndef C2NG_UI_RESHACK_LINETOOL_HPP
#define C2NG_UI_RESHACK_LINETOOL_HPP

#include "afl/string/translator.hpp"
#include "ui/reshack/tool.hpp"

namespace ui { namespace reshack {

    /** Line tool.
        Draws a line from initial click to mouse release point. */
    class LineTool : public Tool {
     public:
        /** Constructor.
            @param tx Translator (for generating name) */
        explicit LineTool(afl::string::Translator& tx);

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
