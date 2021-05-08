/**
  *  \file client/widgets/classicvcrinfo.cpp
  *
  *  FIXME: this is a very specific widget.
  *  Can we make it more general?
  */

#include "client/widgets/classicvcrinfo.hpp"
#include "util/string.hpp"

namespace {
    const size_t NUM_UNITS = 2;
    const size_t NUM_LINES_PER_UNIT = game::vcr::NUM_LINES_PER_UNIT;
}


/*
 * Layout:
 *     [large]      Heading    [normal] type
 *     [normal/2]   -blank-
 *     [bold]       Left Unit            'L'
 *     [normal]       Info1
 *     [normal]       Info2
 *     [normal]       Info3
 *     [normal/2]   -blank-
 *     [bold]       Right Unit           'R'
 *     [normal]       Info1
 *     [normal]       Info2
 *     [normal]       Info3
 *     [normal/2]   -blank-
 *     [normal]     Result             'Tab'
 */

client::widgets::ClassicVcrInfo::ClassicVcrInfo(ui::Root& root)
    : Widget(),
      m_root(root),
      m_leftButton("L", 'l', root),
      m_rightButton("R", 'r', root),
      m_tabButton("Tab", util::Key_Tab, root)
{
    // ex WVcrSelector::WVcrSelector [part]
    addChild(m_leftButton, 0);
    addChild(m_rightButton, 0);
    addChild(m_tabButton, 0);

    m_leftButton.sig_fire.add(&sig_left, &afl::base::Signal<void(int)>::raise);
    m_rightButton.sig_fire.add(&sig_right, &afl::base::Signal<void(int)>::raise);
    m_tabButton.sig_fire.add(&sig_tab, &afl::base::Signal<void(int)>::raise);
}

client::widgets::ClassicVcrInfo::~ClassicVcrInfo()
{ }

void
client::widgets::ClassicVcrInfo::draw(gfx::Canvas& can)
{
    // Prepare coordinates
    const gfx::Rectangle& r = getExtent();
    const int x = r.getLeftX();
    const int w = r.getWidth();
    int y = r.getTopY();

    // Prepare fonts
    afl::base::Ref<gfx::Font> largeFont  = getLargeFont();
    afl::base::Ref<gfx::Font> normalFont = getNormalFont();
    afl::base::Ref<gfx::Font> boldFont   = getBoldFont();

    const int largeHeight  = largeFont->getCellSize().getY();
    const int normalHeight = normalFont->getCellSize().getY();
    const int boldHeight   = boldFont->getCellSize().getY();
    const int indent       = normalHeight/2;

    // Prepare context
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.setSolidBackground();
    ctx.setColor(util::SkinColor::Static);

    // First line
    ctx.useFont(*largeFont);
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    outTextF(ctx, gfx::Point(x, y), w, m_data.heading);

    ctx.useFont(*normalFont);
    ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
    {
        String_t text = m_data.algorithmName;
        util::addListItem(text, ", ", m_data.position);
        outText(ctx, gfx::Point(x+w, y), text);
    }
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);

    y += largeHeight;
    y += normalHeight/2;

    // Two warriors
    for (size_t i = 0; i < NUM_UNITS && i < m_data.units.size(); ++i) {
        const game::vcr::ObjectInfo& info = m_data.units[i];

        ctx.useFont(*boldFont);
        ctx.setColor(info.color[0]);
        outTextF(ctx, gfx::Point(x, y), w, info.text[0]);
        y += boldHeight;

        for (size_t j = 1; j < NUM_LINES_PER_UNIT; ++j) {
            ctx.useFont(*normalFont);
            ctx.setColor(info.color[j]);
            outTextF(ctx, gfx::Point(x + indent, y), w - indent, info.text[j]);
            y += normalHeight;
        }

        y += normalHeight/2;
    }

    // Result
    ctx.useFont(*normalFont);
    ctx.setColor(util::SkinColor::Static);
    outTextF(ctx, gfx::Point(x, y), w, m_data.resultSummary);

    defaultDrawChildren(can);
}

void
client::widgets::ClassicVcrInfo::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::ClassicVcrInfo::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::widgets::ClassicVcrInfo::handleChildAdded(Widget& /*child*/)
{
    requestRedraw();
}

void
client::widgets::ClassicVcrInfo::handleChildRemove(Widget& /*child*/)
{
    requestRedraw();
}

void
client::widgets::ClassicVcrInfo::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    setChildPositions();
}

void
client::widgets::ClassicVcrInfo::handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::widgets::ClassicVcrInfo::getLayoutInfo() const
{
    // Font sizes
    gfx::Point normalCell = getNormalFont()->getCellSize();
    gfx::Point largeCell  = getLargeFont()->getCellSize();
    gfx::Point boldCell   = getBoldFont()->getCellSize();

    // Build result: 8.5x normal, 2x bold, 1x large
    return gfx::Point(normalCell.getX() * 40, normalCell.getY() * 17/2 + boldCell.getY() * 2 + largeCell.getY());
}

bool
client::widgets::ClassicVcrInfo::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::widgets::ClassicVcrInfo::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::widgets::ClassicVcrInfo::setData(const Data_t& data)
{
    m_data = data;
    requestRedraw();
}

afl::base::Ref<gfx::Font>
client::widgets::ClassicVcrInfo::getLargeFont() const
{
    return m_root.provider().getFont(gfx::FontRequest().addSize(1));
}

afl::base::Ref<gfx::Font>
client::widgets::ClassicVcrInfo::getNormalFont() const
{
    return m_root.provider().getFont(gfx::FontRequest());
}

afl::base::Ref<gfx::Font>
client::widgets::ClassicVcrInfo::getBoldFont() const
{
    return m_root.provider().getFont(gfx::FontRequest().addWeight(1));
}

void
client::widgets::ClassicVcrInfo::setChildPositions()
{
    // Metrics
    const int largeHeight  = getLargeFont()->getCellSize().getY();
    const int normalHeight = getNormalFont()->getCellSize().getY();
    const int boldHeight   = getBoldFont()->getCellSize().getY();

    // Build coordinates
    const int rightX = getExtent().getRightX();
    const int buttonSize = largeHeight * 9/8;
    const int leftTop = getExtent().getTopY() + largeHeight + normalHeight/2;
    m_leftButton.setExtent(gfx::Rectangle(rightX - buttonSize, leftTop, buttonSize, buttonSize));

    const int rightTop = leftTop + boldHeight + normalHeight*7/2;
    m_rightButton.setExtent(gfx::Rectangle(rightX - buttonSize, rightTop, buttonSize, buttonSize));

    const int bottomY = getExtent().getBottomY();
    const int tabSize = buttonSize * 8/5;
    m_tabButton.setExtent(gfx::Rectangle(rightX - tabSize, bottomY - buttonSize, tabSize, buttonSize));
}
