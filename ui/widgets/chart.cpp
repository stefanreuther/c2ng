/**
  *  \file ui/widgets/chart.cpp
  *  \brief Class ui::widgets::Chart
  */

#include <algorithm>
#include "ui/widgets/chart.hpp"
#include "gfx/clipfilter.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/icons/skintext.hpp"
#include "util/string.hpp"
#include "util/updater.hpp"
#include "ui/icons/hbox.hpp"
#include "ui/icons/colortile.hpp"
#include "util/layout.hpp"

using util::Updater;
using ui::widgets::Chart;

namespace {
    int convertX(const gfx::Rectangle& area, int maxWorldX, int x)
    {
        return area.getLeftX() + (area.getWidth() * (2*x+1)) / (2*maxWorldX);
    }

    int convertY(const gfx::Rectangle& area, const util::Range<int32_t> worldYRange, int y)
    {
        int32_t worldHeight = worldYRange.max() - worldYRange.min() + 1;
        return area.getBottomY()-1
            - (area.getHeight() * (y - worldYRange.min())) / worldHeight;
    }


    void drawChart(gfx::BaseContext& ctx, const gfx::Rectangle& area, int maxWorldX, util::Range<int32_t> worldYRange, const util::DataTable::Row& data,
                   uint8_t lineMode, Chart::PointIcon pointIcon)
    {
        // ex WScoreChart::drawPlayerChart
        gfx::Point prevPoint;
        enum { Never, On, Off } drawState = Never;

        for (int i = 0, n = data.getNumColumns(); i < n; ++i) {
            int32_t thisValue;
            if (data.get(i).get(thisValue)) {
                gfx::Point thisPoint(convertX(area, maxWorldX, i), convertY(area, worldYRange, thisValue));
                if (drawState == On) {
                    // I'm already drawing, just proceed
                    drawLine(ctx, prevPoint, thisPoint);
                } else if (drawState == Never && (lineMode & Chart::Line_ExtendLeft) != 0) {
                    // I'm starting to draw, and line needs to be extended to the left
                    if (thisPoint.getX() > area.getLeftX()) {
                        drawLine(ctx, gfx::Point(area.getLeftX()+1, thisPoint.getY()), thisPoint);
                    }
                } else {
                    // FIXME: do not draw this pixel if there is an incoming or outgoing line: this messes up patterned lines
                    // (For now, this is mitigated mostly by the affected lines using Line_ExtendLeft.)
                    drawPixel(ctx, thisPoint);
                }
                switch (pointIcon) {
                 case Chart::NoIcon:
                    break;
                 case Chart::DotIcon:
                    drawPixel(ctx, thisPoint + gfx::Point(-1, 0));
                    drawPixel(ctx, thisPoint + gfx::Point(+1, 0));
                    drawPixel(ctx, thisPoint + gfx::Point(0, -1));
                    drawPixel(ctx, thisPoint + gfx::Point(0, +1));
                    break;
                }
                prevPoint = thisPoint;
                drawState = On;
            } else {
                if (drawState == On && (lineMode & Chart::Line_SkipGaps) != 0) {
                    drawState = Off;
                }
            }
        }
        if ((lineMode & Chart::Line_ExtendRight) != 0 && drawState != Never && prevPoint.getX() < area.getRightX()-1) {
            drawLine(ctx, prevPoint, gfx::Point(area.getRightX()-1, prevPoint.getY()));
        }
    }
}


struct ui::widgets::Chart::CompareZOrder {
    bool operator()(const std::pair<const util::DataTable::Row*, const Style*>& a,
                    const std::pair<const util::DataTable::Row*, const Style*>& b) const
        {
            if (a.second->z != b.second->z) {
                return a.second->z < b.second->z;
            } else {
                return a.first->getId() < b.first->getId();
            }
        }
};

struct ui::widgets::Chart::Layout {
    int maxWorldX;
    util::Range<int32_t> worldYRange;

    gfx::Rectangle area;
    gfx::Rectangle bottom;
    gfx::Rectangle left;
};


const ui::widgets::Chart::Style ui::widgets::Chart::DEFAULT_STYLE = {
    0,                          // id
    gfx::SOLID_LINE,            // linePattern
    gfx::OPAQUE_ALPHA,          // alpha
    1,                          // lineThickness
    0,                          // color
    0,                          // lineMode
    DotIcon,                    // pointIcon
    0                           // z
};


/*
 *  StyleRef
 */

ui::widgets::Chart::StyleRef&
ui::widgets::Chart::StyleRef::setLineThickness(int n)
{
    if (Updater().set(m_parent.m_style[m_index].lineThickness, static_cast<uint8_t>(n))) {
        m_parent.requestRedraw();
    }
    return *this;
}

ui::widgets::Chart::StyleRef&
ui::widgets::Chart::StyleRef::setLinePattern(gfx::LinePattern_t pattern)
{
    if (Updater().set(m_parent.m_style[m_index].linePattern, pattern)) {
        m_parent.requestRedraw();
    }
    return *this;
}

ui::widgets::Chart::StyleRef&
ui::widgets::Chart::StyleRef::setColor(uint8_t color)
{
    if (Updater().set(m_parent.m_style[m_index].color, color)) {
        m_parent.requestRedraw();
    }
    return *this;
}

ui::widgets::Chart::StyleRef&
ui::widgets::Chart::StyleRef::setAlpha(gfx::Alpha_t alpha)
{
    if (Updater().set(m_parent.m_style[m_index].alpha, alpha)) {
        m_parent.requestRedraw();
    }
    return *this;
}

ui::widgets::Chart::StyleRef&
ui::widgets::Chart::StyleRef::setLineMode(int mode)
{
    if (Updater().set(m_parent.m_style[m_index].lineMode, static_cast<uint8_t>(mode))) {
        m_parent.requestRedraw();
    }
    return *this;
}

ui::widgets::Chart::StyleRef&
ui::widgets::Chart::StyleRef::setPointIcon(PointIcon icon)
{
    if (Updater().set(m_parent.m_style[m_index].pointIcon, icon)) {
        m_parent.requestRedraw();
    }
    return *this;
}

ui::widgets::Chart::StyleRef&
ui::widgets::Chart::StyleRef::setZOrder(int z)
{
    if (Updater().set(m_parent.m_style[m_index].z, z)) {
        m_parent.requestRedraw();
    }
    return *this;
}

inline
ui::widgets::Chart::StyleRef::StyleRef(Chart& parent, size_t index)
    : m_parent(parent), m_index(index)
{ }


/*
 *  Chart
 */

ui::widgets::Chart::Chart(Root& root, gfx::Point size, util::NumberFormatter fmt)
    : SimpleWidget(),
      m_root(root), m_size(size),
      m_style(), m_data(), m_auxData(), m_formatter(fmt), m_layout(), m_tooltip(root), m_icons()
{
    m_style.push_back(DEFAULT_STYLE);
    m_tooltip.sig_hover.add(this, &Chart::onTooltipHover);
}

ui::widgets::Chart::~Chart()
{ }

void
ui::widgets::Chart::setContent(std::auto_ptr<util::DataTable> data)
{
    m_data = data;
    m_layout.reset();
    requestRedraw();
}

void
ui::widgets::Chart::setAuxContent(std::auto_ptr<util::DataTable> data)
{
    m_auxData = data;
    m_layout.reset();
    requestRedraw();
}

const util::DataTable*
ui::widgets::Chart::getContent() const
{
    return m_data.get();
}

ui::widgets::Chart::StyleRef
ui::widgets::Chart::defaultStyle()
{
    return StyleRef(*this, 0);
}

ui::widgets::Chart::StyleRef
ui::widgets::Chart::style(int id)
{
    // Find existing element
    for (size_t i = 1; i < m_style.size(); ++i) {
        if (m_style[i].id == id) {
            return StyleRef(*this, i);
        }
    }

    // Create new element
    Style newStyle = m_style.back();
    newStyle.id = id;
    m_style.push_back(newStyle);
    return StyleRef(*this, m_style.size()-1);
}

void
ui::widgets::Chart::addNewIcon(int id, gfx::Point pos, ui::icons::Icon* pIcon)
{
    if (pIcon == 0) {
        removeIcon(id);
    } else {
        std::auto_ptr<ui::icons::Icon> p(pIcon);
        bool found = false;
        for (size_t i = 0, n = m_icons.size(); i < n; ++i) {
            if (m_icons[i]->id == id) {
                m_icons[i]->icon = p;
                found = true;
                break;
            }
        }
        if (!found) {
            Icon& pp = *m_icons.pushBackNew(new Icon());
            pp.id = id;
            pp.pos = pos;
            pp.icon = p;
        }
        requestRedraw();
    }
}

void
ui::widgets::Chart::removeIcon(int id)
{
    for (size_t i = 0, n = m_icons.size(); i < n; ++i) {
        if (m_icons[i]->id == id) {
            m_icons.erase(m_icons.begin() + i);
            requestRedraw();
            break;
        }
    }
}

void
ui::widgets::Chart::draw(gfx::Canvas& can)
{
    // Clear canvas. If we don't have data, that's all.
    getColorScheme().drawBackground(can, getExtent());
    if (!m_data.get()) {
        return;
    }

    // Determine layout
    const Layout& lay = getLayout();

    // Determine order of charts to draw
    std::vector<std::pair<const util::DataTable::Row*, const Style*> > plan;
    for (size_t i = 0, n = m_data->getNumRows(); i < n; ++i) {
        if (const util::DataTable::Row* c = m_data->getRow(i)) {
            plan.push_back(std::make_pair(c, &getStyleForId(c->getId())));
        }
    }
    if (m_auxData.get() != 0) {
        for (size_t i = 0, n = m_auxData->getNumRows(); i < n; ++i) {
            if (const util::DataTable::Row* c = m_auxData->getRow(i)) {
                plan.push_back(std::make_pair(c, &getStyleForId(c->getId())));
            }
        }
    }
    std::sort(plan.begin(), plan.end(), CompareZOrder());

    // Draw labels on vertical axis, using layout
    const afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());
    const int lineHeight = font->getLineHeight();
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.useFont(*font);

    {
        std::vector<int32_t> labelValues;      // Values
        std::vector<uint8_t> labelColors;      // Colors
        util::Labels_t labelPositions;         // Layout parameters; uses index into labelValues/labelColors as Id

        // - Maximum label
        labelPositions.push_back(util::Label(int(labelValues.size()), lay.left.getTopY(), lineHeight));
        labelValues.push_back(lay.worldYRange.max());
        labelColors.push_back(Color_Black);

        // - Minimum label
        labelPositions.push_back(util::Label(int(labelValues.size()), lay.left.getBottomY(), lineHeight));
        labelValues.push_back(lay.worldYRange.min());
        labelColors.push_back(Color_Black);

        // - Line labels
        for (size_t i = 0, n = plan.size(); i < n; ++i) {
            const Style& style = *plan[i].second;
            int32_t value;
            if ((style.lineMode & Line_LabelLeft) != 0 && style.lineThickness != 0 && plan[i].first->get(0).get(value)) {
                labelPositions.push_back(util::Label(int(labelValues.size()), convertY(lay.area, lay.worldYRange, value) - lineHeight/2, lineHeight));
                labelValues.push_back(value);
                labelColors.push_back(style.color);
            }
        }

        // - Layout algorithm
        util::computeLabelPositions(labelPositions, lay.left.getTopY(), lay.left.getBottomY());

        // - Draw them
        ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
        for (size_t i = 0; i < labelPositions.size(); ++i) {
            const util::Label& me = labelPositions[i];
            ctx.setColor(labelColors[me.id]);
            outText(ctx, gfx::Point(lay.left.getRightX(), me.pos), m_formatter.formatNumber(labelValues[me.id]));
        }
    }

    // Draw labels on horizontal axis
    ctx.setColor(Color_Black);
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    outTextF(ctx, lay.bottom, m_data->getColumnName(0));
    ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
    outTextF(ctx, lay.bottom, m_data->getColumnName(lay.maxWorldX-1));

    // Draw axes
    drawVLine(ctx, lay.area.getLeftX(), lay.area.getTopY(), lay.area.getBottomY()-1);
    if (lay.worldYRange.contains(1)) {
        drawHLine(ctx, lay.area.getLeftX()-1, lay.area.getTopY()+1, lay.area.getLeftX()+1);
        drawHLine(ctx, lay.area.getLeftX()-2, lay.area.getTopY()+2, lay.area.getLeftX()+2);
    }
    if (lay.worldYRange.contains(-1)) {
        drawHLine(ctx, lay.area.getLeftX()-1, lay.area.getBottomY()-2, lay.area.getLeftX()+1);
        drawHLine(ctx, lay.area.getLeftX()-2, lay.area.getBottomY()-3, lay.area.getLeftX()+2);
    }

    int zeroY = convertY(lay.area, lay.worldYRange, 0);
    drawHLine(ctx, lay.area.getLeftX(), zeroY, lay.area.getRightX()-1);
    drawVLine(ctx, lay.area.getRightX()-2, zeroY-1, zeroY+1);
    drawVLine(ctx, lay.area.getRightX()-3, zeroY-2, zeroY+2);

    // Draw
    gfx::ClipFilter filter(can, getExtent());
    for (size_t i = 0, n = plan.size(); i < n; ++i) {
        // Configure context
        const Style& style = *plan[i].second;
        if (style.lineThickness != 0) {
            gfx::Context<uint8_t> lineContext(filter, m_root.colorScheme());
            lineContext.setLinePattern(style.linePattern);
            lineContext.setAlpha(style.alpha);
            lineContext.setLineThickness(style.lineThickness);
            lineContext.setColor(style.color);
            drawChart(lineContext, lay.area, lay.maxWorldX, lay.worldYRange, *plan[i].first, style.lineMode, style.pointIcon);
        }
    }

    // Draw icons
    for (size_t i = 0; i < m_icons.size(); ++i) {
        Icon& pp = *m_icons[i];
        gfx::Context<util::SkinColor::Color> iconContext(can, getColorScheme());
        pp.icon->draw(iconContext, gfx::Rectangle(lay.area.getTopLeft() + pp.pos, pp.icon->getSize()), ButtonFlags_t());
    }
}

void
ui::widgets::Chart::handleStateChange(State st, bool enable)
{
    m_tooltip.handleStateChange(st, enable);
}

void
ui::widgets::Chart::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    m_layout.reset();
    requestRedraw();
}

ui::layout::Info
ui::widgets::Chart::getLayoutInfo() const
{
    return m_size;
}

bool
ui::widgets::Chart::handleKey(util::Key_t key, int prefix)
{
    m_tooltip.handleKey(key, prefix);
    return false;
}

bool
ui::widgets::Chart::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    bool inside = getExtent().contains(pt);
    if (inside) {
        requestActive();
    }
    m_tooltip.handleMouse(pt, pressedButtons, inside);
    return false;
}

const ui::widgets::Chart::Layout&
ui::widgets::Chart::getLayout()
{
    assert(m_data.get() != 0);
    if (m_layout.get() == 0) {
        m_layout.reset(new Layout());

        // Area
        m_layout->area = getExtent();

        // World ranges
        m_layout->maxWorldX = std::max(1, m_data->getNumColumns());
        m_layout->worldYRange = m_data->getValueRange();
        m_layout->worldYRange.include(0);
        if (m_layout->worldYRange.isUnit()) {
            m_layout->worldYRange.include(1);
        }

        // Allocate space at bottom
        const afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());
        const int lineHeight = font->getLineHeight();
        m_layout->bottom = m_layout->area.splitBottomY(lineHeight);

        // Allocate space for axis lables
        String_t maxLabel = m_formatter.formatNumber(m_layout->worldYRange.max());
        String_t minLabel = m_formatter.formatNumber(m_layout->worldYRange.min());
        m_layout->left = m_layout->area.splitX(std::max(font->getTextWidth(maxLabel),
                                                        font->getTextWidth(minLabel)));
        m_layout->area.consumeX(5);

        // Extra room for thick lines
        m_layout->area.consumeY(1);
        m_layout->area.consumeBottomY(1);
        m_layout->area.consumeRightX(1);
    }
    return *m_layout;
}

const ui::widgets::Chart::Style&
ui::widgets::Chart::getStyleForId(int id) const
{
    for (size_t i = 1; i < m_style.size(); ++i) {
        if (m_style[i].id == id) {
            return m_style[i];
        }
    }
    return m_style.front();
}

void
ui::widgets::Chart::onTooltipHover(gfx::Point pos)
{
    // Data available?
    if (!m_data.get()) {
        return;
    }

    // Check layout: must be inside area
    const Layout& lay = getLayout();
    if (!lay.area.contains(pos)) {
        return;
    }

    // Find closest point
    const int MAX_DIST = 30;
    const int estWorldX = (pos.getX() - lay.area.getLeftX()) * lay.maxWorldX / lay.area.getWidth();
    const int fuzz = 1 + MAX_DIST * lay.maxWorldX / lay.area.getWidth();

    const util::DataTable::Row* foundRow = 0;
    int foundColumn = 0;
    int32_t foundValue = 0;
    int32_t foundDist = MAX_DIST * MAX_DIST;
    for (size_t rowIndex = 0, numRows = m_data->getNumRows(); rowIndex < numRows; ++rowIndex) {
        if (const util::DataTable::Row* row = m_data->getRow(rowIndex)) {
            if (getStyleForId(row->getId()).lineThickness != 0) {
                for (int i = std::max(0, estWorldX - fuzz); i <= estWorldX + fuzz; ++i) {
                    int32_t value;
                    if (row->get(i).get(value)) {
                        const int screenDX = convertX(lay.area, lay.maxWorldX, i)       - pos.getX();
                        const int screenDY = convertY(lay.area, lay.worldYRange, value) - pos.getY();
                        const int screenDist = screenDX*screenDX + screenDY*screenDY;
                        if (screenDist < foundDist) {
                            foundRow = row;
                            foundColumn = i;
                            foundValue = value;
                            foundDist = screenDist;
                        }
                    }
                }
            }
        }
    }

    if (foundRow != 0) {
        String_t text;
        util::addListItem(text, ", ", foundRow->getName());
        util::addListItem(text, ", ", m_data->getColumnName(foundColumn));
        util::addListItem(text, ": ", m_formatter.formatNumber(foundValue));

        ui::icons::SkinText t(text, m_root);
#if 0
        m_tooltip.showPopup(gfx::Point(convertX(lay.area, lay.maxWorldX, foundColumn), convertY(lay.area, lay.worldYRange, foundValue)), t);
#else
        int size = m_root.provider().getFont(gfx::FontRequest())->getLineHeight() / 2;
        ui::icons::ColorTile c(m_root, gfx::Point(size, size), getStyleForId(foundRow->getId()).color);
        c.setFrameWidth(0);

        ui::icons::HBox box;
        box.add(c);
        box.add(t);
        box.setPad(5);
        m_tooltip.showPopup(gfx::Point(convertX(lay.area, lay.maxWorldX, foundColumn), convertY(lay.area, lay.worldYRange, foundValue)), box);
#endif
    }
}
