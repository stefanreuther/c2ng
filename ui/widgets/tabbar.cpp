/**
  *  \file ui/widgets/tabbar.cpp
  *  \brief Class ui::widgets::TabBar
  */

#include "ui/widgets/tabbar.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "util/key.hpp"

struct ui::widgets::TabBar::TabInfo {
    size_t id;
    String_t name;
    util::Key_t key;

    TabInfo(size_t id, const String_t& name, util::Key_t key)
        : id(id), name(name), key(key)
        { }
};


ui::widgets::TabBar::TabBar(Root& root)
    : Widget(),
      sig_tabClick(),
      m_root(root),
      m_tabs(),
      m_currentTabId(0),
      m_font(gfx::FontRequest().addSize(1)),
      m_keys(Tab | CtrlTab)
{
    // ex UICardGroupTab::UICardGroupTab
}

ui::widgets::TabBar::~TabBar()
{
    // ex UICardGroupTab::~UICardGroupTab
}

void
ui::widgets::TabBar::addPage(size_t id, const String_t& name, util::Key_t key)
{
    // ex UICardGroupTab::addTab
    m_tabs.pushBackNew(new TabInfo(id, name, key));
    requestRedraw();
}

void
ui::widgets::TabBar::addPage(size_t id, const util::KeyString& name)
{
    // ex UICardGroupTab::addTab
    addPage(id, name.getString(), name.getKey());
}

void
ui::widgets::TabBar::setFocusedTab(size_t id)
{
    if (id != m_currentTabId) {
        m_currentTabId = id;
        requestRedraw();
        sig_tabClick.raise(m_currentTabId);
    }
}

void
ui::widgets::TabBar::setFont(gfx::FontRequest font)
{
    m_font = font;
}

void
ui::widgets::TabBar::setKeys(int keys)
{
    m_keys = keys;
}

void
ui::widgets::TabBar::draw(gfx::Canvas& can)
{
    // ex UICardGroupTab::drawContent
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_font);

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
        const bool isCurrent = (tab.id == m_currentTabId);

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
ui::widgets::TabBar::handlePositionChange()
{ }

void
ui::widgets::TabBar::handleChildPositionChange(Widget& /*child*/, const gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
ui::widgets::TabBar::getLayoutInfo() const
{
    // ex UICardGroupTab::getLayoutInfo
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_font);

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
    bool handleTab     = (m_keys & Tab) != 0;
    bool handleCtrlTab = (m_keys & CtrlTab) != 0;
    bool handleF6      = (m_keys & F6) != 0;
    bool handleArrows  = (m_keys & Arrows) != 0;

    // Tab: next page
    if ((handleTab && key == util::Key_Tab)
        || (handleCtrlTab && key == util::Key_Tab + util::KeyMod_Ctrl)
        || (handleF6 && key == util::Key_F6)
        || (handleArrows && key == util::Key_Right))
    {
        size_t index = getCurrentIndex() + 1;
        if (index >= m_tabs.size()) {
            index = 0;
        }
        setCurrentIndex(index);
        return true;
    }

    // Shift-Tab: previous page
    if ((handleTab && key == util::Key_Tab + util::KeyMod_Shift)
        || (handleCtrlTab && key == util::Key_Tab + util::KeyMod_Ctrl + util::KeyMod_Shift)
        || (handleF6 && key == util::Key_F6 + util::KeyMod_Shift)
        || (handleArrows && key == util::Key_Left))
    {
        size_t index = getCurrentIndex();
        if (index == 0) {
            index = m_tabs.size();
        }
        setCurrentIndex(index - 1);
        return true;
    }

    // Per-page keys
    for (size_t i = 0, n = m_tabs.size(); i < n; ++i) {
        TabInfo* pTab = m_tabs[i];
        // Do not consume the key if it refers to the active page.
        // This is required for the ship build screen, where the "S"tarship Hull page has a "S"pecification button.
        if ((pTab->key == key || pTab->key == (key & ~util::KeyMod_Alt)) && i != getCurrentIndex()) {
            setCurrentIndex(i);
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
        afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_font);

        int x = getExtent().getLeftX();
        for (size_t i = 0; i < m_tabs.size(); ++i) {
            const TabInfo& tab = *m_tabs[i];
            int tw = font->getTextWidth(tab.name) + 20;
            x += 10;
            if (x > pt.getX()) {
                break;
            }
            if (pt.getX() < x + tw) {
                if (tab.id != m_currentTabId) {
                    setFocusedTab(tab.id);
                    return true;
                }
                break;
            }
            x += tw+6;
        }
    }
    return false;
}

size_t
ui::widgets::TabBar::getCurrentIndex() const
{
    for (size_t i = 0, n = m_tabs.size(); i < n; ++i) {
        TabInfo* pTab = m_tabs[i];
        if (pTab->id == m_currentTabId) {
            return i;
        }
    }
    return 0;
}

void
ui::widgets::TabBar::setCurrentIndex(size_t index)
{
    if (index < m_tabs.size()) {
        setFocusedTab(m_tabs[index]->id);
    }
}
