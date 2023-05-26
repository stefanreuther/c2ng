/**
  *  \file client/widgets/filterdisplay.cpp
  */

#include "client/widgets/filterdisplay.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "util/skincolor.hpp"
#include "util/unicodechars.hpp"

namespace {
    namespace gsi = game::spec::info;

    /*
     *  Icons
     */

    gfx::Point scale(const gfx::Rectangle& r, int x, int y)
    {
        return gfx::Point(r.getLeftX() + r.getWidth() * x / 16,
                          r.getTopY() + r.getHeight() * y / 16);
    }

    /* Edit icon (pencil) */
    void drawEdit(gfx::BaseContext& ctx, const gfx::Rectangle& r)
    {
        ctx.setCursor(scale(r, 3, 10));
        drawLineTo(ctx, scale(r, 3, 13));
        drawLineTo(ctx, scale(r, 6, 13));
        drawLineTo(ctx, scale(r, 14, 5));
        drawLineTo(ctx, scale(r, 11, 2));
        drawLineTo(ctx, scale(r, 3, 10));
        drawLineTo(ctx, scale(r, 6, 13));
        drawLine(ctx, scale(r, 9, 4), scale(r, 12, 7));
        drawLine(ctx, scale(r, 10, 5), scale(r, 4, 11));
        drawLine(ctx, scale(r, 11, 6), scale(r, 5, 12));
    }

    /* Switch in "off" position */
    void drawSwitchOff(gfx::BaseContext& ctx, const gfx::Rectangle& r)
    {
        ctx.setCursor(scale(r, 6, 10));
        drawLineTo(ctx, scale(r, 4, 10));
        drawLineTo(ctx, scale(r, 4, 4));
        drawLineTo(ctx, scale(r, 8, 4));
        drawLineTo(ctx, scale(r, 8, 6));
        drawLineTo(ctx, scale(r, 12, 10));
        drawLineTo(ctx, scale(r, 12, 12));
        drawLineTo(ctx, scale(r, 8, 12));
        drawLineTo(ctx, scale(r, 8, 10));
        drawLineTo(ctx, scale(r, 4, 6));
        drawLineTo(ctx, scale(r, 8, 6));
        drawLine(ctx, scale(r, 8, 10), scale(r, 12, 10));
        drawLine(ctx, scale(r, 4, 8), scale(r, 8, 12));
    }

    /* Switch in "on" position */
    void drawSwitchOn(gfx::BaseContext& ctx, const gfx::Rectangle& r)
    {
        ctx.setCursor(scale(r, 6, 14-10));
        drawLineTo(ctx, scale(r, 4, 14-10));
        drawLineTo(ctx, scale(r, 4, 14-4));
        drawLineTo(ctx, scale(r, 8, 14-4));
        drawLineTo(ctx, scale(r, 8, 14-6));
        drawLineTo(ctx, scale(r, 12, 14-10));
        drawLineTo(ctx, scale(r, 12, 14-12));
        drawLineTo(ctx, scale(r, 8, 14-12));
        drawLineTo(ctx, scale(r, 8, 14-10));
        drawLineTo(ctx, scale(r, 4, 14-6));
        drawLineTo(ctx, scale(r, 8, 14-6));
        drawLine(ctx, scale(r, 8, 14-10), scale(r, 12, 14-10));
        drawLine(ctx, scale(r, 4, 14-8), scale(r, 8, 14-12));
    }

    /* Add filter (plus, funnel) */
    void drawFilterAdd(gfx::BaseContext& ctx, const gfx::Rectangle& r)
    {
        ctx.setCursor(scale(r, 3, 2));
        drawLineTo(ctx, scale(r, 13, 2));
        drawLineTo(ctx, scale(r, 13, 3));
        drawLineTo(ctx, scale(r, 9, 7));
        drawLineTo(ctx, scale(r, 9, 12));
        drawLineTo(ctx, scale(r, 7, 10));
        drawLineTo(ctx, scale(r, 7, 7));
        drawLineTo(ctx, scale(r, 3, 3));
        drawLineTo(ctx, scale(r, 3, 2));

        drawLine(ctx, scale(r, 1, 11), scale(r, 5, 11));
        drawLine(ctx, scale(r, 3, 9), scale(r, 3, 13));
    }

    /* Sort (arrow, lines) */
    // void drawSort(gfx::BaseContext& ctx, const gfx::Rectangle& r)
    // {
    //     ctx.setCursor(scale(r, 2, 10));
    //     drawLineTo(ctx, scale(r, 4, 12));
    //     drawLineTo(ctx, scale(r, 6, 10));
    //     drawLine(ctx, scale(r, 4, 3), scale(r, 4, 12));

    //     for (int i = 0; i < 5; ++i) {
    //         drawLine(ctx, scale(r, 7, 3 + 2*i), scale(r, 9+i, 3 + 2*i));
    //     }
    // }

    /*
     *  Color Scheme
     */

    struct Colors {
        uint8_t faded;
        uint8_t fixed;
        uint8_t variable;
        uint8_t background;
        uint8_t highlight;
    };
    const Colors COLORS[] = {
        // Not focused
        { ui::Color_Grayscale + 7, ui::Color_Black, ui::Color_GreenBlack, ui::Color_Gray,  ui::Color_BlueGray },
        // Focused
        { ui::Color_Dark,          ui::Color_White, ui::Color_Green,      ui::Color_Black, ui::Color_BlueGray },
    };
}


client::widgets::FilterDisplay::FilterDisplay(ui::Root& root, afl::string::Translator& tx)
    : m_root(root),
      m_translator(tx),
      m_content(),
      m_highlight(None),
      m_highlightIndex(0),
      m_focus(fAdd),
      m_focusIndex(0),
      m_mouseDown(false),
      m_filterAvailable(true),
      m_sortLabel(),
      m_sortActive(false)
{ }

void
client::widgets::FilterDisplay::setContent(const game::spec::info::FilterInfos_t& infos)
{
    m_content = infos;

    // Cancel highlight if line becomes invalid
    if ((m_highlight == Edit || m_highlight == Delete) && m_highlightIndex >= infos.size()) {
        m_highlight = None;
        m_highlightIndex = 0;
    }

    // Cancel focus if line becomes invalid
    if (m_focus == fEdit && m_focusIndex >= infos.size()) {
        m_focus = fAdd;
        m_focusIndex = 0;
    }
    requestRedraw();
}

void
client::widgets::FilterDisplay::setSort(String_t label, bool active)
{
    m_sortLabel = label;
    m_sortActive = active;
    requestRedraw();
}

void
client::widgets::FilterDisplay::setFilterAvailable(bool flag)
{
    if (flag != m_filterAvailable) {
        m_filterAvailable = flag;
        requestRedraw();
    }
}

gfx::Point
client::widgets::FilterDisplay::getFilterAnchor() const
{
    gfx::Rectangle area = getExtent();
    area.grow(-1, -1);
    area.consumeY(int(m_content.size()) * getFilterHeight());

    gfx::Rectangle thisArea = area.splitY(getMenuHeight());
    gfx::Rectangle filterArea = thisArea.splitRightX(getFilterButtonWidth());

    return filterArea.getBottomLeft();
}

gfx::Point
client::widgets::FilterDisplay::getSortAnchor() const
{
    gfx::Rectangle area = getExtent();
    area.grow(-1, -1);
    area.consumeY(int(m_content.size()) * getFilterHeight());

    gfx::Rectangle thisArea = area.splitY(getMenuHeight());
    return thisArea.getBottomLeft();
}

void
client::widgets::FilterDisplay::draw(gfx::Canvas& can)
{
    const int CELL_SIZE = getFilterHeight();

    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());
    gfx::Rectangle area = getExtent();
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    gfx::Context<util::SkinColor::Color> ctx2(can, getColorScheme());
    ctx.useFont(*font);

    ui::drawFrameDown(ctx, area);
    area.grow(-1, -1);

    for (size_t i = 0; i < m_content.size(); ++i) {
        bool focused = (hasState(FocusedState) && m_focus == fEdit && m_focusIndex == i);
        const Colors& c = COLORS[focused];

        // Background
        gfx::Rectangle thisArea = area.splitY(getFilterHeight());
        drawSolidBar(ctx, thisArea, c.background);

        // Icons
        bool active = m_content[i].active;
        ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
        ctx.setColor(m_highlight == Delete && i == m_highlightIndex ? c.highlight : c.fixed);
        outTextF(ctx, thisArea.splitRightX(CELL_SIZE), UTF_BALLOT_CROSS);
        if (m_content[i].mode != gsi::NotEditable) {
            ctx.setColor(!active ? c.faded : m_highlight == Edit && i == m_highlightIndex ? c.highlight : c.fixed);

            gfx::Rectangle pos = gfx::Rectangle(0, 0, 16, 16).centerWithin(thisArea.splitRightX(CELL_SIZE));
            if (m_content[i].mode == gsi::SetValueRange) {
                if (m_content[i].elem.range == gsi::IntRange_t::fromValue(0)) {
                    drawSwitchOff(ctx, pos);
                } else {
                    drawSwitchOn(ctx, pos);
                }
            } else {
                drawEdit(ctx, pos);
            }
        }

        // Pre-text pad
        int d = font->getLineHeight()/4;
        ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
        thisArea.consumeX(d);

        // Text
        String_t label = m_content[i].name + ": ";
        int w = font->getTextWidth(label);
        ctx.setColor(active ? c.fixed : c.faded);
        outTextF(ctx, thisArea.splitX(w), label);

        ctx.setColor(active ? c.variable : c.faded);
        outTextF(ctx, thisArea, m_content[i].value);
    }

    // Menu
    gfx::Rectangle thisArea = area.splitY(getMenuHeight());

    // Filter button
    {
        bool focused = (hasState(FocusedState) && m_focus == fAdd);
        const Colors& c = COLORS[focused];

        gfx::Rectangle filterArea = thisArea.splitRightX(getFilterButtonWidth());
        drawSolidBar(ctx, filterArea, c.background);

        ctx.setColor(!m_filterAvailable ? c.faded : m_highlight == Add ? c.highlight : c.fixed);
        ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);

        gfx::Rectangle iconPos = gfx::Rectangle(0, 0, 16, 16).centerWithin(filterArea.splitRightX(CELL_SIZE));
        drawFilterAdd(ctx, iconPos);

        outTextF(ctx, filterArea, m_translator("Filter"));
    }

    // Sort button
    {
        bool focused = (hasState(FocusedState) && m_focus == fSort);
        const Colors& c = COLORS[focused];

        gfx::Rectangle sortArea = thisArea;
        drawSolidBar(ctx, sortArea, c.background);

        int d = font->getLineHeight()/4;
        ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
        sortArea.consumeX(d);

        ctx.setColor(m_highlight == Sort ? c.highlight : c.fixed);
        // For now, don't draw the sort icon; doesn't look good.
        // gfx::Rectangle iconPos = gfx::Rectangle(0, 0, 16, 16).centerWithin(sortArea.splitX(CELL_SIZE));
        // drawSort(ctx, iconPos);

        String_t label = m_translator("Sort") + ": ";
        int w = font->getTextWidth(label);
        outTextF(ctx, sortArea.splitX(w), label);

        ctx.setColor(m_sortActive ? c.variable : c.faded);
        outTextF(ctx, sortArea, m_sortLabel);
    }

    drawSolidBar(ctx, area, COLORS[false].background);
}

void
client::widgets::FilterDisplay::handleStateChange(State st, bool enable)
{
    if (st == ActiveState && !enable) {
        if (m_highlight != None) {
            m_highlight = None;
            requestRedraw();
        }
        m_mouseDown = false;
    }
    if (st == FocusedState) {
        requestRedraw();
    }
}

void
client::widgets::FilterDisplay::handlePositionChange()
{
    requestRedraw();
}

ui::layout::Info
client::widgets::FilterDisplay::getLayoutInfo() const
{
    int w = m_root.provider().getFont(gfx::FontRequest())->getEmWidth() * 20;
    int h = getFilterHeight() * int(m_content.size()) + getMenuHeight();
    return ui::layout::Info(gfx::Point(w, h), ui::layout::Info::GrowHorizontal);
}

bool
client::widgets::FilterDisplay::handleKey(util::Key_t key, int prefix)
{
    if (getFocusState() == PrimaryFocus) {
        switch (key) {
         case util::Key_Up:
            if (m_focus == fEdit && m_focusIndex > 0) {
                setFocus(fEdit, m_focusIndex-1);
            } else if ((m_focus == fSort || m_focus == fAdd) && !m_content.empty()) {
                setFocus(fEdit, m_content.size()-1);
            } else {
            }
            return true;

         case util::Key_Down:
            if (m_focus == fEdit) {
                if (m_focusIndex+1 < m_content.size()) {
                    setFocus(fEdit, m_focusIndex+1);
                } else {
                    setFocus(fSort, 0);
                }
            }
            return true;

         case util::Key_Right:
            if (m_focus == fSort) {
                setFocus(fAdd, 0);
            }
            return true;

         case util::Key_Left:
            if (m_focus == fAdd) {
                setFocus(fSort, 0);
            }
            return true;

         case util::Key_Insert:
            if (m_filterAvailable) {
                sig_add.raise();
            }
            return true;

         case util::Key_Delete:
            if (m_focus == fEdit) {
                sig_delete.raise(m_focusIndex);
            }
            return true;

         case ' ':
            if (m_focus == fEdit) {
                sig_edit.raise(m_focusIndex);
            } else if (m_focus == fAdd && m_filterAvailable) {
                sig_add.raise();
            } else if (m_focus == fSort) {
                sig_sort.raise();
            } else {
            }
            return true;
        }
    }
    return defaultHandleKey(key, prefix);
}

bool
client::widgets::FilterDisplay::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    bool down = !pressedButtons.empty();
    bool click = (m_mouseDown && !down);
    m_mouseDown = down;

    gfx::Rectangle area = getExtent();
    const int CELL_SIZE = getFilterHeight();
    area.grow(-1, -1);
    for (size_t i = 0; i < m_content.size(); ++i) {
        gfx::Rectangle thisArea = area.splitY(CELL_SIZE);

        if (thisArea.splitRightX(CELL_SIZE).contains(pt)) {
            setHighlight(Delete, i);
            if (click) {
                sig_delete.raise(i);
            }
            return true;
        }

        if (m_content[i].mode != gsi::NotEditable && thisArea.splitRightX(CELL_SIZE).contains(pt) && m_content[i].active) {
            setHighlight(Edit, i);
            if (click) {
                setFocus(fEdit, i);
                sig_edit.raise(i);
            }
            return true;
        }

        if (thisArea.contains(pt) && click) {
            setFocus(fEdit, i);
            return true;
        }
    }

    gfx::Rectangle thisArea = area.splitY(getMenuHeight());
    if (thisArea.splitRightX(getFilterButtonWidth()).contains(pt)) {
        setHighlight(Add, 0);
        if (click && m_filterAvailable) {
            setFocus(fAdd, 0);
            sig_add.raise();
        }
        return true;
    }

    if (thisArea.contains(pt)) {
        setHighlight(Sort, 0);
        if (click) {
            setFocus(fSort, 0);
            sig_sort.raise();
        }
        return true;
    }

    setHighlight(None, 0);
    return defaultHandleMouse(pt, pressedButtons);
}

int
client::widgets::FilterDisplay::getFilterHeight() const
{
    int h = m_root.provider().getFont(gfx::FontRequest())->getLineHeight();
    return h + (2*(h/6));
}

inline int
client::widgets::FilterDisplay::getMenuHeight() const
{
    return getFilterHeight();
}

int
client::widgets::FilterDisplay::getFilterButtonWidth() const
{
    return getFilterHeight()*3/2 + m_root.provider().getFont(gfx::FontRequest())->getTextWidth(m_translator("Filter"));
}

void
client::widgets::FilterDisplay::setHighlight(Highlight h, size_t index)
{
    if (h != m_highlight || index != m_highlightIndex) {
        m_highlight = h;
        m_highlightIndex = index;
        requestRedraw();
        if (h != None) {
            requestActive();
        }
    }
}

void
client::widgets::FilterDisplay::setFocus(Focus f, size_t index)
{
    if (f != m_focus || index != m_focusIndex) {
        m_focus = f;
        m_focusIndex = index;
        requestRedraw();
        requestFocus();
    }
}
