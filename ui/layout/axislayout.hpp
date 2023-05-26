/**
  *  \file ui/layout/axislayout.hpp
  *  \brief Class ui::layout::AxisLayout
  */
#ifndef C2NG_UI_LAYOUT_AXISLAYOUT_HPP
#define C2NG_UI_LAYOUT_AXISLAYOUT_HPP

#include <vector>
#include "afl/base/types.hpp"

namespace ui { namespace layout {

    /** Helper for computing widget layouts.
        Accepts layout constraints along one axis (e.g. X axis coordinates)
        and computes widget positions along that axis.

        To use,
        - add layout constraints using add(), update();
        - use empty(), size(), etc. to inquire;
        - use computeLayout() to build a layout. */
    class AxisLayout {
     public:
        /** Position result. */
        struct Position {
            int position;                ///< Relative position. You need to add origin coordinate.
            int size;                    ///< Size.
            Position(int position, int size)
                : position(position), size(size)
                { }
        };

        /** Default constructor.
            Create an empty layout.
            @post size() == 0 */
        AxisLayout();

        /** Add layout constraint.
            Adds a new slot that can contain a widget.
            @param prefSize     Preferred size of widget
            @param isFlexible   true if widget agrees to grow (or, if needed, shrink)
            @param isIgnored    true if widget shall be ignored for layout
            @post size() > 0 */
        void add(int prefSize, bool isFlexible, bool isIgnored);

        /** Update layout constraint.
            Updates a slot with data for an additional widget.
            This function has no isIgnored parameter; do not call it for ignored widgets.
            @param index        Index, [0, size())
            @param prefSize     Preferred size of new widget. Preferred slot size is maximum of all widgets in it.
            @param isFlexible   true if new widget agrees to grow. Slot agrees to grow only if all widgets are flexible. */
        void update(size_t index, int prefSize, bool isFlexible);

        /** Check emptiness.
            @return true if AxisLayout is empty (no slots added) */
        bool empty() const;

        /** Get number of slots.
            @return number of slots (=number of add() calls)) */
        size_t size() const;

        /** Get total size of all widgets/slots.
            @return sum of all prefSize parameters */
        int getTotalSize() const;

        /** Check whether layout is flexible.
            @return true if layout includes at least one flexible widget */
        bool isFlexible() const;

        /** Check whether a slot is ignored.
            @param index  Index, [0, size())
            @return true if add() was called with isIgnored=true for this slot */
        bool isIgnored(size_t index) const;

        /** Compute layout.
            @param space          Desired inter-widget space
            @param outer          Desired outer padding
            @param availableSize  Available space
            @return computed positions. One element for each added constraint.
            @post result.size() == size() */
        std::vector<Position> computeLayout(int space, int outer, int availableSize) const;

     private:
        struct Info {
            int prefSize;
            bool isFlexible;
            bool isIgnored;

            Info(int prefSize, bool isFlexible, bool isIgnored)
                : prefSize(prefSize), isFlexible(isFlexible), isIgnored(isIgnored)
                { }
        };

        std::vector<Info> m_info;
    };

} }


inline bool
ui::layout::AxisLayout::empty() const
{
    return m_info.empty();
}

inline size_t
ui::layout::AxisLayout::size() const
{
    return m_info.size();
}

#endif
