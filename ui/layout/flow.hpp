/**
  *  \file ui/layout/flow.hpp
  */
#ifndef C2NG_UI_LAYOUT_FLOW_HPP
#define C2NG_UI_LAYOUT_FLOW_HPP

#include "ui/layout/manager.hpp"

namespace ui { namespace layout {

    /** Flow layout.
        This layout manager positions widgets in a row, with line breaks if needed, much like a typesetter positions words.
        It can allocate from top-left to bottom-right, or vice versa.

        Unlike the Java FlowLayout, you have to specify the number of lines you expect to use
        (so getLayoutInfo() can report the correct size).
        Like the Java FlowLayout, however, this might exceed the allocated size anyway. */
    class Flow : public Manager {
     public:
        /** Constructor.
            \param numLines number of lines to allocate space for
            \param rightJust allocate from bottom-right, not top-left
            \param horizontalGap distance between widgets in X direction
            \param verticalGap distance between lines in Y direction */
        Flow(int numLines, bool rightJust, int horizontalGap = 5, int verticalGap = 5)
            : m_numLines(numLines), m_rightJustified(rightJust), m_horizontalGap(horizontalGap), m_verticalGap(verticalGap)
            { }

        virtual void doLayout(Widget& container, gfx::Rectangle area) const;
        virtual Info getLayoutInfo(const Widget& container) const;

     private:
        int  m_numLines;
        bool m_rightJustified;
        int  m_horizontalGap;
        int  m_verticalGap;
    };

} }

#endif
