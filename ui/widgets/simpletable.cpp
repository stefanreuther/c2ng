/**
  *  \file ui/widgets/simpletable.cpp
  */

#include <cassert>
#include "ui/widgets/simpletable.hpp"
#include "gfx/context.hpp"
#include "util/updater.hpp"

ui::widgets::SimpleTable::Range&
ui::widgets::SimpleTable::Range::setText(const String_t& text)
{
    util::Updater up;
    for (size_t i = 0, pos = m_start; i < m_count; ++i, pos += m_stride) {
        assert(pos < m_table.m_cells.size());
        up.set(m_table.m_cells[pos].text, text);
    }
    if (up) {
        m_table.requestUpdateMetrics();
        m_table.requestRedraw();
    }
    return *this;
}

ui::widgets::SimpleTable::Range&
ui::widgets::SimpleTable::Range::setFont(gfx::FontRequest& font)
{
    util::Updater up;
    for (size_t i = 0, pos = m_start; i < m_count; ++i, pos += m_stride) {
        assert(pos < m_table.m_cells.size());
        up.set(m_table.m_cells[pos].font, font);
    }
    if (up) {
        m_table.requestUpdateMetrics();
        m_table.requestRedraw();
    }
    return *this;
}

ui::widgets::SimpleTable::Range&
ui::widgets::SimpleTable::Range::setTextAlign(int x, int y)
{
    util::Updater up;
    for (size_t i = 0, pos = m_start; i < m_count; ++i, pos += m_stride) {
        assert(pos < m_table.m_cells.size());
        up.set(m_table.m_cells[pos].alignX, x);
        up.set(m_table.m_cells[pos].alignY, y);
    }
    if (up) {
        m_table.requestRedraw();
    }
    return *this;
}

ui::widgets::SimpleTable::Range&
ui::widgets::SimpleTable::Range::setColor(uint8_t color)
{
    util::Updater up;
    for (size_t i = 0, pos = m_start; i < m_count; ++i, pos += m_stride) {
        assert(pos < m_table.m_cells.size());
        up.set(m_table.m_cells[pos].color, color);
    }
    if (up) {
        m_table.requestRedraw();
    }
    return *this;
}

ui::widgets::SimpleTable::Range
ui::widgets::SimpleTable::Range::subrange(size_t start, size_t count)
{
    size_t effStart = std::min(start, m_count);
    size_t effCount = std::min(count, m_count - effStart);
    return Range(m_table, m_start + m_stride*effStart, m_stride, effCount);
}


inline
ui::widgets::SimpleTable::Range::Range(SimpleTable& table, size_t start, size_t stride, size_t count)
    : m_table(table), m_start(start), m_stride(stride), m_count(count)
{ }


ui::widgets::SimpleTable::SimpleTable(Root& root, size_t numColumns, size_t numRows)
    : m_root(root),
      m_cells(),
      m_rowMetrics(),
      m_columnMetrics(),
      m_numRows(numRows),
      m_numColumns(numColumns)
{
    m_cells.resize(numColumns * numRows);
    m_rowMetrics.resize(numRows);
    m_columnMetrics.resize(numColumns);
}

ui::widgets::SimpleTable::~SimpleTable()
{ }

ui::widgets::SimpleTable::Range
ui::widgets::SimpleTable::cell(size_t x, size_t y)
{
    if (x < m_numColumns && y < m_numRows) {
        return Range(*this, x + y*m_numColumns, 0, 1);
    } else {
        return Range(*this, 0, 0, 0);
    }
}

ui::widgets::SimpleTable::Range
ui::widgets::SimpleTable::row(size_t y)
{
    if (y < m_numRows) {
        return Range(*this, y * m_numColumns, 1, m_numColumns);
    } else {
        return Range(*this, 0, 0, 0);
    }
}

ui::widgets::SimpleTable::Range
ui::widgets::SimpleTable::column(size_t x)
{
    if (x < m_numColumns) {
        return Range(*this, x, m_numColumns, m_numRows);
    } else {
        return Range(*this, 0, 0, 0);
    }
}

void
ui::widgets::SimpleTable::setRowHeight(size_t row, int height)
{
    if (Metric* p = getMetricPtr(m_rowMetrics, row)) {
        p->isAuto = false;
        p->size = height;
    }
    requestRedraw();
}

void
ui::widgets::SimpleTable::clearRowHeight(size_t row)
{
    if (Metric* p = getMetricPtr(m_rowMetrics, row)) {
        p->isAuto = true;
        p->size = 0;
    }
    requestUpdateMetrics();
    requestRedraw();
}

void
ui::widgets::SimpleTable::setRowPadding(size_t row, int height)
{
    if (Metric* p = getMetricPtr(m_rowMetrics, row)) {
        p->padAfter = height;
    }
    requestRedraw();
}

void
ui::widgets::SimpleTable::setColumnWidth(size_t column, int width)
{
    if (Metric* p = getMetricPtr(m_columnMetrics, column)) {
        p->isAuto = false;
        p->size = width;
    }
    requestRedraw();
}

void
ui::widgets::SimpleTable::clearColumnWidth(size_t column)
{
    if (Metric* p = getMetricPtr(m_columnMetrics, column)) {
        p->isAuto = true;
        p->size = 0;
    }
    requestUpdateMetrics();
    requestRedraw();
}

void
ui::widgets::SimpleTable::setColumnPadding(size_t column, int width)
{
    if (Metric* p = getMetricPtr(m_columnMetrics, column)) {
        p->padAfter = width;
    }
    requestRedraw();
}

void
ui::widgets::SimpleTable::draw(gfx::Canvas& can)
{
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    gfx::Rectangle area(getExtent());
    gfx::Rectangle rowArea;

    size_t row = 0;
    size_t column = 0;
    for (size_t i = 0, n = m_cells.size(); i < n; ++i) {
        // On first column, determine row area
        if (column == 0) {
            Metric rowMetric = getMetric(m_rowMetrics, row);
            rowArea = area.splitY(rowMetric.size);
            area.consumeY(rowMetric.padAfter);
        }

        // Determine cell area
        Metric columnMetric = getMetric(m_columnMetrics, column);
        gfx::Rectangle cellArea = rowArea.splitX(columnMetric.size);
        rowArea.consumeX(columnMetric.padAfter);

        // Render cell
        const Cell& c = m_cells[i];
        ctx.useFont(*m_root.provider().getFont(c.font));
        ctx.setTextAlign(c.alignX, c.alignY);
        ctx.setColor(c.color);
        outTextF(ctx, cellArea, c.text);

        // Next cell
        ++column;
        if (column == m_numColumns) {
            column = 0;
            ++row;
        }
    }
}

void
ui::widgets::SimpleTable::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::widgets::SimpleTable::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
ui::widgets::SimpleTable::getLayoutInfo() const
{
    return gfx::Point(sumMetric(m_columnMetrics), sumMetric(m_rowMetrics));
}

bool
ui::widgets::SimpleTable::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return false;
}

bool
ui::widgets::SimpleTable::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}

void
ui::widgets::SimpleTable::requestUpdateMetrics()
{
    // Placeholder for future deferred metrics update
    updateMetrics();
}

void
ui::widgets::SimpleTable::updateMetrics()
{
    size_t row = 0;
    size_t column = 0;
    for (size_t i = 0, n = m_cells.size(); i < n; ++i) {
        const Cell& c = m_cells[i];
        afl::base::Ref<gfx::Font> font = m_root.provider().getFont(c.font);
        updateAutoMetric(m_rowMetrics, row, font->getTextHeight(c.text));
        updateAutoMetric(m_columnMetrics, column, font->getTextWidth(c.text));

        ++column;
        if (column == m_numColumns) {
            column = 0;
            ++row;
        }
    }
}

void
ui::widgets::SimpleTable::resetMetric(std::vector<Metric>& m)
{
    for (size_t i = 0, n = m.size(); i < n; ++i) {
        if (m[i].isAuto) {
            m[i].size = 0;
        }
    }
}

void
ui::widgets::SimpleTable::updateAutoMetric(std::vector<Metric>& m, size_t index, int value)
{
    if (index < m.size()) {
        if (m[index].isAuto) {
            if (value > m[index].size) {
                m[index].size = value;
            }
        }
    }
}

int
ui::widgets::SimpleTable::sumMetric(const std::vector<Metric>& m)
{
    int sum = 0;
    for (size_t i = 0, n = m.size(); i < n; ++i) {
        sum += m[i].size;
        sum += m[i].padAfter;
    }
    return sum;
}

ui::widgets::SimpleTable::Metric
ui::widgets::SimpleTable::getMetric(const std::vector<Metric>& m, size_t index)
{
    if (index < m.size()) {
        return m[index];
    } else {
        return Metric();
    }
}

ui::widgets::SimpleTable::Metric*
ui::widgets::SimpleTable::getMetricPtr(std::vector<Metric>& m, size_t index)
{
    if (index < m.size()) {
        return &m[index];
    } else {
        return 0;
    }
}
