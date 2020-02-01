/**
  *  \file ui/widgets/tabbar.cpp
  */

#include "ui/widgets/tabbar.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "util/key.hpp"

struct ui::widgets::TabBar::TabInfo {
    size_t index;
    String_t name;
    util::Key_t key;
    Widget& widget;

    TabInfo(size_t index, const String_t& name, util::Key_t key, Widget& w)
        : index(index), name(name), key(key), widget(w)
        { }
};


ui::widgets::TabBar::TabBar(Root& root, CardGroup& g)
    : m_root(root),
      m_group(g),
      m_tabs(),
      conn_focusChange(g.sig_handleFocusChange.add(this, (void (TabBar::*)()) &TabBar::requestRedraw))
{
    // ex UICardGroupTab::UICardGroupTab
}

ui::widgets::TabBar::~TabBar()
{
    // ex UICardGroupTab::~UICardGroupTab
    conn_focusChange.disconnect();
}

void
ui::widgets::TabBar::addPage(const String_t& name, util::Key_t key, Widget& w)
{
    // ex UICardGroupTab::addTab
    m_tabs.pushBackNew(new TabInfo(m_tabs.size(), name, key, w));
    requestRedraw();
}

void
ui::widgets::TabBar::addPage(const util::KeyString& name, Widget& w)
{
    // ex UICardGroupTab::addTab
    addPage(name.getString(), name.getKey(), w);
}

void
ui::widgets::TabBar::setFocusedPage(size_t index)
{
    if (index < m_tabs.size()) {
        m_tabs[index]->widget.requestFocus();
    }
}

void
ui::widgets::TabBar::draw(gfx::Canvas& can)
{
    // ex UICardGroupTab::drawContent
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addSize(1));

    // Straight-forward port, could be more idiomatic
    int x = getExtent().getLeftX();
    int top = getExtent().getTopY();
    int bot = getExtent().getBottomY();

    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());  // used to draw interior of tabs which is visually inside the group
    ctx.useFont(*font);

    gfx::ColorQuad_t whiteColor = m_root.colorScheme().getColor(ui::Color_White);
    gfx::ColorQuad_t grayColor  = m_root.colorScheme().getColor(ui::Color_Gray);
    gfx::ColorQuad_t blackColor = m_root.colorScheme().getColor(ui::Color_Black);

    for (std::size_t i = 0, n = m_tabs.size(); i < n; ++i) {
        // Status
        const TabInfo& tab = *m_tabs[i];
        const bool isCurrent = tab.widget.hasState(FocusedState);

        // first, draw lines at bottom
        int this_width = font->getTextWidth(tab.name) + 20;
        int lwidth = isCurrent ? 10 : this_width+16;
        can.drawHLine(gfx::Point(x, bot-2), lwidth, whiteColor, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        can.drawHLine(gfx::Point(x, bot-1), lwidth, grayColor,  gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        if (isCurrent) {
            drawBackground(ctx, gfx::Rectangle(x + 10, bot - 2, this_width + 6, 2));
        }

        // left side of tab
        x += 10;
        can.drawVLine(gfx::Point(x-1, top), bot-2-top, whiteColor, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        can.drawVLine(gfx::Point(x, top+1), bot-2-top, grayColor,  gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);

        // top
        can.drawHLine(gfx::Point(x, top),   this_width, whiteColor, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        can.drawHLine(gfx::Point(x, top+1), this_width, grayColor,  gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);

        // content and rhs
        int cx = 0, he = bot - top - 2, lastl = 1, bx = x + this_width;
        for (int l = 1; l < he; ++l) {
            int w = 6 * l / he;
            if (w != cx) {
                can.drawBar(gfx::Rectangle(bx + cx, top + lastl, 2, l - lastl), blackColor, blackColor, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
                if (lastl == 1 && l != 1) { // HACK HACK HACK
                    ++lastl;
                }
                drawBackground(ctx, gfx::Rectangle(x+1, top + lastl, cx+this_width-1, l - lastl));
                lastl = l;
                cx = w;
            }
        }
        drawBackground(ctx, gfx::Rectangle(x+1, top + lastl, cx+this_width-1, he - lastl));
        can.drawBar(gfx::Rectangle(bx + cx, top + lastl, 2,  he - lastl), blackColor, blackColor, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);

        ctx.setColor(isCurrent ? util::SkinColor::Heading : util::SkinColor::Static);
        outText(ctx, gfx::Point(x + 10, top + 1), tab.name);

        x += this_width+6;
    }

    int right = getExtent().getRightX();
    if (x < right) {
        can.drawHLine(gfx::Point(x, bot-2), right-x, whiteColor, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        can.drawHLine(gfx::Point(x, bot-1), right-x, grayColor,  gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
    }
}

void
ui::widgets::TabBar::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::widgets::TabBar::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
ui::widgets::TabBar::handleChildAdded(Widget& /*child*/)
{ }

void
ui::widgets::TabBar::handleChildRemove(Widget& /*child*/)
{ }

void
ui::widgets::TabBar::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

void
ui::widgets::TabBar::handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
ui::widgets::TabBar::getLayoutInfo() const
{
    // ex UICardGroupTab::getLayoutInfo
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addSize(1));

    int min_x = 0;
    for (size_t i = 0; i < m_tabs.size(); ++i) {
        min_x += font->getTextWidth(m_tabs[i]->name) + 36;
    }

    int height = font->getLineHeight() + 3;

    return ui::layout::Info(gfx::Point(min_x, height),
                            gfx::Point(min_x, height),
                            ui::layout::Info::GrowHorizontal);
}

bool
ui::widgets::TabBar::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex UICardGroupTab::handleEvent (part)
    // Tab: next page
    if (key == util::Key_Tab) {
        if (const TabInfo* pTab = getCurrentTab()) {
            size_t index = pTab->index + 1;
            if (index >= m_tabs.size()) {
                index = 0;
            }
            setFocusedPage(index);
            return true;
        }
    }

    // Shift-Tab: previous page
    if (key == util::Key_Tab + util::KeyMod_Shift) {
        if (const TabInfo* pTab = getCurrentTab()) {
            size_t index = pTab->index;
            if (index == 0) {
                index = m_tabs.size();
            }
            setFocusedPage(index - 1);
            return true;
        }
    }

    // Per-page keys
    for (size_t i = 0, n = m_tabs.size(); i < n; ++i) {
        TabInfo* pTab = m_tabs[i];
        if (pTab->key == key || pTab->key == (key & ~util::KeyMod_Alt)) {
            setFocusedPage(i);
            return true;
        }
    }

    return false;
}

bool
ui::widgets::TabBar::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // ex UICardGroupTab::handleEvent (part)
    if (getExtent().contains(pt) && !pressedButtons.empty()) {
        afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addSize(1));

        int x = getExtent().getLeftX();
        for (size_t i = 0; i < m_tabs.size(); ++i) {
            const TabInfo& tab = *m_tabs[i];
            int tw = font->getTextWidth(tab.name) + 20;
            x += 10;
            if (x > pt.getX()) {
                break;
            }
            if (pt.getX() < x + tw) {
                if (!tab.widget.hasState(FocusedState)) {
                    tab.widget.requestFocus();
                    return true;
                }
                break;
            }
            x += tw+6;
        }
    }
    return false;
}

const ui::widgets::TabBar::TabInfo*
ui::widgets::TabBar::getCurrentTab() const
{
    for (size_t i = 0, n = m_tabs.size(); i < n; ++i) {
        TabInfo* pTab = m_tabs[i];
        if (pTab->widget.hasState(FocusedState)) {
            return pTab;
        }
    }
    return 0;
}
