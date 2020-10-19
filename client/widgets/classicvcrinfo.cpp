/**
  *  \file client/widgets/classicvcrinfo.cpp
  *
  *  FIXME: this is a very specific widget.
  *  Can we make it more general?
  */

#include "client/widgets/classicvcrinfo.hpp"
#include "util/string.hpp"

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

client::widgets::ClassicVcrInfo::Data::Data()
{
    for (size_t i = 0; i < NUM_UNITS; ++i) {
        for (size_t j = 0; j < NUM_LINES_PER_UNIT; ++j) {
            color[i][j] = util::SkinColor::Static;
        }
    }
}

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
    ctx.setTextAlign(0, 0);
    outTextF(ctx, gfx::Point(x, y), w, m_data.text[Heading]);

    ctx.useFont(*normalFont);
    ctx.setTextAlign(2, 0);
    {
        String_t text = m_data.text[Type];
        util::addListItem(text, ", ", m_data.text[Position]);
        outText(ctx, gfx::Point(x+w, y), text);
    }
    ctx.setTextAlign(0, 0);

    y += largeHeight;
    y += normalHeight/2;

    // Two warriors
    for (size_t i = 0; i < NUM_UNITS; ++i) {
        ctx.useFont(*boldFont);
        ctx.setColor(m_data.color[i][0]);
        outTextF(ctx, gfx::Point(x, y), w, m_data.info[i][0]);
        y += boldHeight;

        for (size_t j = 1; j < NUM_LINES_PER_UNIT; ++j) {
            ctx.useFont(*normalFont);
            ctx.setColor(m_data.color[i][j]);
            outTextF(ctx, gfx::Point(x + indent, y), w - indent, m_data.info[i][j]);
            y += normalHeight;
        }

        y += normalHeight/2;
    }

    // Result
    ctx.useFont(*normalFont);
    ctx.setColor(util::SkinColor::Static);
    outTextF(ctx, gfx::Point(x, y), w, m_data.text[Result]);

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
client::widgets::ClassicVcrInfo::setData(const Data& data)
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
