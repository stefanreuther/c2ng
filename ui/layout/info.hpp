/**
  *  \file ui/layout/info.hpp
  *  \brief Class ui::layout::Info
  */
#ifndef C2NG_UI_LAYOUT_INFO_HPP
#define C2NG_UI_LAYOUT_INFO_HPP

#include "gfx/point.hpp"

namespace ui { namespace layout {

    /** Layout information.
        Widgets report their layout wishes using this class.

        Widgets can opt out of layouting (NoLayout).
        Widgets that take part in layouting have a minimum and preferred size:
        - if possible, the widget is given its preferred size
        - if less room is available, the widget is given its minimum size
        - if more room is available, and the widget allows growing in that direction,
          it is given more room

        Note that if too little room is available, or layout constraints conflict,
        widgets may still be given more or less room than they request.
        For example, a widget A may be made wider even if it is not marked GrowHorizontal
        if it is put in a VBox together with a wide widget B (VBox(A,B)).
        This problem is normally countered by putting the no-grow widget into a HBox
        together with a spacer (VBox(HBox(A,Spacer),B)).

        \see ui::Widget::getLayoutInfo() */
    class Info {
     public:
        /** Layout/Growth behaviour. */
        enum Growth {
            NoLayout,             ///< Widget does not take part in layout/is invisible.
            Fixed,                ///< Widget has a fixed size.
            GrowHorizontal,       ///< Widget has a fixed height but can grow horizontally.
            GrowVertical,         ///< Widget has a fixed width but can grow vertically.
            GrowBoth              ///< Widget can grow in any direction.
        };

        /** General constructor.
            \param minSize Minimum size
            \param prefSize Preferred size
            \param growth Growth behaviour */
        Info(gfx::Point minSize, gfx::Point prefSize, Growth growth) throw();

        /** Fixed-size constructor.
            \param fixedSize Fixed size (used as preferred and minimum size).
            \post getGrowthBehaviour() == Fixed */
        Info(gfx::Point fixedSize) throw();

        /** No-layout/invisible constructor.
            \post getGrowthBehaviour() == NoLayout */
        Info() throw();

        /** Get minimum size.
            \result size */
        gfx::Point getMinSize() const;

        /** Get preferred size.
            \result size */
        gfx::Point getPreferredSize() const;

        /** Get growth behaviour.
            \return growth behaviour */
        Growth getGrowthBehaviour() const;

        /** Check for horizontal growth.
            \return true if the widget can grow horizontally. */
        bool isGrowHorizontal() const;

        /** Check for vertical growth.
            \return true if the widget can grow vertically. */
        bool isGrowVertical() const;

        /** Check whether widget shall be ignored in layout.
            \return true if the widget shall be ignored in layout */
        bool isIgnored() const;

        /** Check for horizontal growth.
            \param g Growth behaviour value
            \return true if the value allows horizontal growth. */
        static bool isGrowHorizontal(Growth g);

        /** Check for vertical growth.
            \param g Growth behaviour value
            \return true if the value allows vertical growth. */
        static bool isGrowVertical(Growth g);

        /** Check for ignored widget.
            \param g Growth behaviour value
            \return true if the value requests the widget to be ignored. */
        static bool isIgnored(Growth g);

        /** Make growth behaviour from parameters.
            \param h Desired horizontal growth
            \param v Desired vertical growth
            \param ignore Desired ignorance */
        static Growth makeGrowthBehaviour(bool h, bool v, bool ignore);

        /** Combine two growth behaviours with "And".
            The "And" combination is used for a widget that needs to fulfill two layout constraints at the same time.
            For example, GrowHorizontal AND GrowBoth yields GrowHorizontal.
            \param a First value
            \param b Second value
            \return Combined value */
        static Growth andGrowthBehaviour(Growth a, Growth b);

     private:
        gfx::Point m_minSize;
        gfx::Point m_preferredSize;
        Growth m_growth;
    };

} }

// Get minimum size.
inline gfx::Point
ui::layout::Info::getMinSize() const
{
    return m_minSize;
}

// Get preferred size.
inline gfx::Point
ui::layout::Info::getPreferredSize() const
{
    return m_preferredSize;
}

// Get growth behaviour.
inline ui::layout::Info::Growth
ui::layout::Info::getGrowthBehaviour() const
{
    return m_growth;
}

// Check for horizontal growth.
inline bool
ui::layout::Info::isGrowHorizontal() const
{
    return isGrowHorizontal(m_growth);
}

// Check for vertical growth.
inline bool
ui::layout::Info::isGrowVertical() const
{
    return isGrowVertical(m_growth);
}

// Check whether widget shall be ignored in layout.
inline bool
ui::layout::Info::isIgnored() const
{
    return isIgnored(m_growth);
}

#endif
