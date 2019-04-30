/**
  *  \file ui/widgets/simpletable.hpp
  */
#ifndef C2NG_UI_WIDGETS_SIMPLETABLE_HPP
#define C2NG_UI_WIDGETS_SIMPLETABLE_HPP

#include <vector>
#include "gfx/fontrequest.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"

namespace ui { namespace widgets {

    /** Simple (non-interactive) Table Widget.
        Caller can define properties and values to be displayed in cells.
        Cells can be sized automatically or manually.
        Use cell(), row() or column() to manipulate slices of the table. */
    class SimpleTable : public SimpleWidget {
     public:
        /** Range of cells.
            Represents either a single cell, (part of) a row, or (part of) a column. */
        class Range {
         public:
            /** Set text content of all cells in range.
                \param text New text
                \return *this */
            Range& setText(const String_t& text);

            /** Set font of all cells in range.
                \param font New font
                \return *this */
            Range& setFont(gfx::FontRequest& font);

            /** Set text alignment of all cells in range.
                \param x X alignment (0=left, 1=center, 2=right)
                \param y Y alignment (0=top, 1=middle, 2=bottom)
                \return *this */
            Range& setTextAlign(int x, int y);

            /** Set color of all cells in range.
                \param color Color (ui::Color_xxx)
                \return *this */
            Range& setColor(uint8_t color);

            /** Set number of extra columns to allocate for this cell value.
                Zero means just this cell, one means this cell and the next one, and so on.
                The skipped cells' values and attributes will be ignored.
                (This would be HTML's "colspan" attribute, minus one.)
                \param n Number of columns
                \return *this */
            Range& setExtraColumns(int n);

            /** Set underlining for all cells in range.
                This will underline the whole cell.
                \param flag true to underline
                \return *this */
            Range& setUnderline(bool flag);

            /** Get subrange.
                \param start 0-based index
                \param count Number of cells
                \return new range. If the parameters are out of range, a reduced range will be returned. */
            Range subrange(size_t start, size_t count);

         private:
            friend class SimpleTable;
            Range(SimpleTable& table, size_t start, size_t stride, size_t count);

            SimpleTable& m_table;
            size_t m_start;
            size_t m_stride;
            size_t m_count;
        };

        /** Constructor.
            \param root Root
            \param numColumns Number of columns
            \param numRows Number of rows */
        SimpleTable(Root& root, size_t numColumns, size_t numRows);

        /** Destructor. */
        ~SimpleTable();

        /** Get handle to a cell.
            \param x Column (0-based)
            \param y Row (0-based)
            \return Handle to requested cell; empty range if parameters are out of range */
        Range cell(size_t x, size_t y);

        /** Get handle to a row.
            \param y Row (0-based)
            \return Handle to all cells of the requested row; empty range if parameters are out of range */
        Range row(size_t y);

        /** Get handle to a column.
            \param x Column (0-based)
            \return Handle to all cells of the requested column; empty range if parameters are out of range */
        Range column(size_t x);

        /** Set height of a row.
            This row will have this exact height in pixels.
            \param row Row (0-based)
            \param height Desired height */
        void setRowHeight(size_t row, int height);

        /** Clear fixed height of a row.
            This row height will be determined from its content.
            \param row Row (0-based) */
        void clearRowHeight(size_t row);

        /** Set post-padding of a row.
            This row will be followed by a gap of this height.
            \param row Row (0-based)
            \param height Gap height */
        void setRowPadding(size_t row, int height);

        /** Set width of a column.
            This column will have this exact width in pixels.
            \param column Column (0-based)
            \param width Desired width */
        void setColumnWidth(size_t column, int width);

        /** Clear fixed width of a column.
            This column width will be determined from its content.
            \param column Column (0-based) */
        void clearColumnWidth(size_t column);

        /** Set post-padding of a column.
            This column will be followed by a gap of this width.
            \param column Column (0-based)
            \param width Gap width */
        void setColumnPadding(size_t column, int width);

        /** Set number of rows.
            This can be used to make the table larger or shorter.
            New cells will not yet have any content or format.
            \param numRows new number */
        void setNumRows(size_t numRows);

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        struct Cell {
            String_t text;
            gfx::FontRequest font;
            int alignX;
            int alignY;
            int extraColumns;
            uint8_t color;
            bool underlined;
            Cell()
                : text(), font(), alignX(0), alignY(0), extraColumns(0), color(ui::Color_White), underlined(false)
                { }
        };
        struct Metric {
            bool isAuto;
            int size;
            int padAfter;
            Metric()
                : isAuto(true), size(0), padAfter(0)
                { }
        };

        Root& m_root;

        std::vector<Cell> m_cells;
        std::vector<Metric> m_rowMetrics;
        std::vector<Metric> m_columnMetrics;
        size_t m_numRows;
        size_t m_numColumns;

        void requestUpdateMetrics();
        void updateMetrics();

        static void resetMetric(std::vector<Metric>& m);
        static void updateAutoMetric(std::vector<Metric>& m, size_t index, int value);
        static int sumMetric(const std::vector<Metric>& m);
        static Metric getMetric(const std::vector<Metric>& m, size_t index);
        static Metric* getMetricPtr(std::vector<Metric>& m, size_t index);
    };

} }

#endif
