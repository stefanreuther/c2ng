/**
  *  \file ui/reshack/pipettetool.hpp
  *  \brief Class ui::reshack::PipetteTool
  */
#ifndef C2NG_UI_RESHACK_PIPETTETOOL_HPP
#define C2NG_UI_RESHACK_PIPETTETOOL_HPP

#include "afl/string/translator.hpp"
#include "ui/reshack/tool.hpp"

namespace ui { namespace reshack {

    class Painter;

    /** Pipette tool.
        This tool updates a painter's color on click. */
    class PipetteTool : public Tool {
     public:
        /** Constructor.
            @param tx Translator (for generating name)
            @param p  Painter whose color to control */
        explicit PipetteTool(afl::string::Translator& tx, Painter& p);

        // Tool:
        void click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t bg);
        void drag(gfx::BaseContext& c, gfx::Point pt);
        void release(gfx::BaseContext& c, gfx::Point pt);
        bool isUsable();

     private:
        Painter& m_painter;
    };

} }

#endif
