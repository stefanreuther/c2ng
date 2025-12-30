/**
  *  \file ui/reshack/tool.hpp
  *  \brief Class ui::reshack::Tool
  */
#ifndef C2NG_UI_RESHACK_TOOL_HPP
#define C2NG_UI_RESHACK_TOOL_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/point.hpp"

namespace ui { namespace reshack {

    /** Tool for use in Painter.
        Each use of the tool is translated into a sequence
        - click()
        - drag() (0..n times)
        - release()

        The tool receives a configured BaseContext containing the canvas to draw on
        and the foreground color to use; it can update it. */
    class Tool : public afl::base::Deletable {
     public:
        /** Constructor.
            @param needsPreview true for tools for which the drag() operation is a preview
                                and the subject image is restored before each subsequent call;
                                false if each drag() draws persistently.
            @param name         Name of tool */
        Tool(bool needsPreview, String_t name);

        /** Destructor. */
        ~Tool();

        /** Click (=start operation).
            @param ctx  Context
            @param pt   Point (image coordinates)
            @param bg   Background color */
        virtual void click(gfx::BaseContext& ctx, gfx::Point pt, gfx::Color_t bg) = 0;

        /** Drag (=continue operation).
            @param ctx  Context
            @param pt   Point (image coordinates) */
        virtual void drag(gfx::BaseContext& ctx, gfx::Point pt) = 0;

        /** Release (=end operation).
            @param ctx  Context
            @param pt   Point (image coordinates) */
        virtual void release(gfx::BaseContext& ctx, gfx::Point pt) = 0;

        /** Check whether tool is usable.
            A tool can be temporarily unusable if some preconditions are not valid.
            @return status */
        virtual bool isUsable() = 0;

        /** Get needsPreview flag.
            @return flag; see constructor */
        bool needsPreview() const;

        /** Get name of tool.
            @return name; see constructor */
        const String_t& getName() const;

     private:
        const bool m_needsPreview;
        const String_t m_name;
    };

} }

#endif
