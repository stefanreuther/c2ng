/**
  *  \file ui/reshack/penciltool.hpp
  *  \brief Class ui::reshack::PencilTool
  */
#ifndef C2NG_UI_RESHACK_PENCILTOOL_HPP
#define C2NG_UI_RESHACK_PENCILTOOL_HPP

#include "afl/string/translator.hpp"
#include "ui/reshack/tool.hpp"

namespace ui { namespace reshack {

    /** Pencil tool.
        Draws a line following the mouse drag. */
    class PencilTool : public Tool {
     public:
        /** Constructor.
            @param tx Translator (for generating name) */
        explicit PencilTool(afl::string::Translator& tx);

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
