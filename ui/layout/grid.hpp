/**
  *  \file ui/layout/grid.hpp
  *  \brief Class ui::layout::Grid
  */
#ifndef C2NG_UI_LAYOUT_GRID_HPP
#define C2NG_UI_LAYOUT_GRID_HPP

#include "afl/base/optional.hpp"
#include "ui/layout/manager.hpp"

namespace ui { namespace layout {

    /** Grid layout.
        This layout manager arranges widgets in a grid.
        The container will be divided with a grid, and each cell contains a widget.
        The cell widths and heights are computed automatically.
        You can force widths and/or heights to a common value if you need to. */
    class Grid : public Manager {
     public:
        /** Construct new grid.
            \param numColumns Number of columns
            \param space      Space (pixels) between widgets, for X and Y direction
            \param outer      Padding (pixels) at all borders */
        Grid(size_t numColumns, int space = 5, int outer = 0);

        // Manager:
        virtual void doLayout(Widget& container, gfx::Rectangle area) const;
        virtual Info getLayoutInfo(const Widget& container) const;

        /** Set forced cell size.
            You can force cells to have a particular size, with no respect to the contained widgets' wishes.
            By default, no requirements are given, thus the layout manager respects all widgets' layout requests.
            \param forcedCellWidth  required width for cells; Nothing to accept any size.
            \param forcedCellHeight required height for cells; Nothing to accept any size. */
        void setForcedCellSize(afl::base::Optional<int> forcedCellWidth, afl::base::Optional<int> forcedCellHeight);

        /** Get forced cell width.
            \return value set using setForcedCellSize(): required width for cells; Nothing to accept any size */
        afl::base::Optional<int> getForcedCellWidth() const
            { return m_forcedCellWidth; }

        /** Get forced cell height.
            \return value set using setForcedCellSize(): required height for cells; Nothing to accept any size */
        afl::base::Optional<int> getForcedCellHeight() const
            { return m_forcedCellHeight; }

     private:
        size_t m_numColumns;
        int m_space;
        int m_outer;
        afl::base::Optional<int> m_forcedCellWidth;
        afl::base::Optional<int> m_forcedCellHeight;
    };

} }

#endif
