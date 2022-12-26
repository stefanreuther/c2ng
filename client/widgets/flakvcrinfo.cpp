/**
  *  \file client/widgets/flakvcrinfo.cpp
  *  \brief Class client::widgets::FlakVcrInfo
  */

#include "client/widgets/flakvcrinfo.hpp"
#include "afl/string/format.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "util/string.hpp"

namespace {
    bool isLastShipInGroup(size_t shipIndex, const game::vcr::BattleInfo& data)
    {
        for (size_t i = 0, n = data.groups.size(); i < n; ++i) {
            if (shipIndex == data.groups[i].firstObject + data.groups[i].numObjects-1) {
                return true;
            }
        }
        return false;
    }
}

client::widgets::FlakVcrInfo::FlakVcrInfo(ui::Root& root, afl::string::Translator& tx)
    : m_root(root),
      m_translator(tx),
      m_listButton("L", 'l', root),
      m_tabButton("Tab", util::Key_Tab, root),
      m_scoreButton("S", 's', root),
      m_showMapButton("F4", util::Key_F4, root),
      m_data(),
      m_adjectiveNames(),
      m_teamSettings()
{
    // ex FlakVcrSelector::FlakVcrSelector
    addChild(m_listButton, 0);
    addChild(m_tabButton, 0);
    addChild(m_scoreButton, 0);
    addChild(m_showMapButton, 0);

    m_listButton.sig_fire.add(&sig_list, &afl::base::Signal<void(int)>::raise);
    m_tabButton.sig_fire.add(&sig_tab, &afl::base::Signal<void(int)>::raise);
    m_scoreButton.sig_fire.add(&sig_score, &afl::base::Signal<void(int)>::raise);
    m_showMapButton.sig_fire.add(this, &FlakVcrInfo::onMap);

    updateButtonState();
}

client::widgets::FlakVcrInfo::~FlakVcrInfo()
{ }

void
client::widgets::FlakVcrInfo::draw(gfx::Canvas& can)
{
    // ex FlakVcrSelector::drawContent
    // Prepare coordinates
    const gfx::Rectangle& r = getExtent();
    const int x = r.getLeftX();
    const int w = r.getWidth();
    int y = r.getTopY();

    // Prepare fonts
    afl::base::Ref<gfx::Font> largeFont  = getLargeFont();
    afl::base::Ref<gfx::Font> normalFont = getNormalFont();

    const int largeHeight  = largeFont->getCellSize().getY();
    const int normalHeight = normalFont->getCellSize().getY();
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
        if (const game::map::Point* pt = m_data.position.get()) {
            util::addListItem(text, ", ", pt->toString());
        }
        outText(ctx, gfx::Point(x+w, y), text);
    }
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);

    y += largeHeight;
    y += normalHeight/2;

    // Only draw content if there is any (to fill the temporary state where m_data has just a heading, no content)
    // This is a convenient place to fend off zero-size fonts which would give a division error.
    int nships  = static_cast<int>(m_data.units.size());
    int nfleets = static_cast<int>(m_data.groups.size());
    if (normalHeight > 0 && nships > 0 && nfleets > 0) {
        // Content
        outTextF(ctx, gfx::Point(x, y), w, afl::string::Format(m_translator("%d unit%!1{s%} in %d group%!1{s%}:"), nships, nfleets));
        y += normalHeight;

        int n = (r.getBottomY() - y) / normalHeight;
        if (nships <= n) {
            // Enough room for ships
            ctx.setTransparentBackground();
            for (size_t i = 0; i < m_data.units.size(); ++i) {
                // Clear text
                drawBackground(ctx, gfx::Rectangle(x+indent, y, w-indent, normalHeight));

                // If this is the last ship in its group (and this is not the last group), draw a divider.
                // This is O(n^2), but n is small and this makes us independant of the order of groups.
                if (i+1 != m_data.units.size() && isLastShipInGroup(i, m_data)) {
                    ctx.setColor(util::SkinColor::Faded);
                    drawHLine(ctx, x+indent, y + normalHeight-1, x+w-1);
                }

                // Unit name
                ctx.setColor(m_data.units[i].color[0]);
                outTextF(ctx, gfx::Point(x+indent, y), w-indent, m_data.units[i].text[0]);
                y += normalHeight;
            }
        } else if (nfleets <= n) {
            // Enough room for fleets
            ctx.setTransparentBackground();
            for (size_t i = 0; i < m_data.groups.size(); ++i) {
                // Clear text and draw divider (for consistency with single-ship version)
                if (i+1 != m_data.groups.size()) {
                    drawBackground(ctx, gfx::Rectangle(x+indent, y, w-indent, normalHeight));
                    ctx.setColor(util::SkinColor::Faded);
                    drawHLine(ctx, x+indent, y + normalHeight-1, x+w-1);
                }

                // Unit/fleet name
                const game::vcr::GroupInfo& f = m_data.groups[i];
                ctx.setColor(m_teamSettings.getPlayerColor(f.owner));
                String_t text;
                if (f.numObjects == 1 && f.firstObject < m_data.units.size()) {
                    text = m_data.units[f.firstObject].text[0];
                } else {
                    text = afl::string::Format(m_translator("%d %s unit%0$!d%!1{s%}"), f.numObjects, m_adjectiveNames.get(f.owner));
                }
                outTextF(ctx, gfx::Point(x+indent, y), w-indent, text);
                y += normalHeight;
            }
        } else {
            // Race summary
            game::PlayerArray<int> sum;
            sum.setAll(0);
            for (size_t i = 0; i < m_data.groups.size(); ++i) {
                int owner = m_data.groups[i].owner;
                sum.set(owner, sum.get(owner) + 1);
            }
            for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
                if (int numPlayerGroups = sum.get(i)) {
                    ctx.setColor(m_teamSettings.getPlayerColor(i));
                    outTextF(ctx, gfx::Point(x+indent, y), w-indent, afl::string::Format(m_translator("%d %s group%0$!d%!1{s%}"), numPlayerGroups, m_adjectiveNames.get(i)));
                    y += normalHeight;
                }
            }
        }
    }

    // Buttons
    defaultDrawChildren(can);
}

void
client::widgets::FlakVcrInfo::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::FlakVcrInfo::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::widgets::FlakVcrInfo::handleChildAdded(Widget& /*child*/)
{
    requestRedraw();
}

void
client::widgets::FlakVcrInfo::handleChildRemove(Widget& /*child*/)
{
    requestRedraw();
}

void
client::widgets::FlakVcrInfo::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    setChildPositions();
}

void
client::widgets::FlakVcrInfo::handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::widgets::FlakVcrInfo::getLayoutInfo() const
{
    // ex FlakVcrSelector::getLayoutInfo
    gfx::Point normalCell = getNormalFont()->getCellSize();
    gfx::Point largeCell  = getLargeFont()->getCellSize();

    gfx::Point size = largeCell.scaledBy(20, 1)
        .extendBelow(normalCell.scaledBy(40, 13));
    size.addY(normalCell.getY() / 2);

    return ui::layout::Info(size, size, ui::layout::Info::GrowBoth);
}

bool
client::widgets::FlakVcrInfo::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::widgets::FlakVcrInfo::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::widgets::FlakVcrInfo::setPlayerNames(const game::PlayerArray<String_t>& adjNames)
{
    m_adjectiveNames = adjNames;
    requestRedraw();
}

void
client::widgets::FlakVcrInfo::setTeams(const game::TeamSettings& teams)
{
    m_teamSettings.copyFrom(teams);
    requestRedraw();
}

void
client::widgets::FlakVcrInfo::setData(const Data_t& data)
{
    m_data = data;
    requestRedraw();
    updateButtonState();
}

void
client::widgets::FlakVcrInfo::setTabAvailable(bool flag)
{
    m_tabButton.setState(DisabledState, !flag);
}

afl::base::Ref<gfx::Font>
client::widgets::FlakVcrInfo::getLargeFont() const
{
    return m_root.provider().getFont(gfx::FontRequest().addSize(1));
}

afl::base::Ref<gfx::Font>
client::widgets::FlakVcrInfo::getNormalFont() const
{
    return m_root.provider().getFont(gfx::FontRequest());
}

afl::base::Ref<gfx::Font>
client::widgets::FlakVcrInfo::getBoldFont() const
{
    return m_root.provider().getFont(gfx::FontRequest().addWeight(1));
}

void
client::widgets::FlakVcrInfo::setChildPositions()
{
    // Dimensions
    const int largeHeight  = getLargeFont()->getCellSize().getY();
    const int buttonSize = largeHeight * 9/8;
    const int tabSize = buttonSize * 8/5;
    const int pad = 5;

    gfx::Rectangle area = getExtent();
    gfx::Rectangle lastRow = area.splitBottomY(buttonSize);

    m_tabButton.setExtent(lastRow.splitRightX(tabSize));
    lastRow.consumeRightX(pad);

    m_scoreButton.setExtent(lastRow.splitRightX(buttonSize));
    lastRow.consumeRightX(pad);

    m_listButton.setExtent(lastRow.splitRightX(buttonSize));
    lastRow.consumeRightX(pad);

    m_showMapButton.setExtent(lastRow.splitRightX(m_showMapButton.getLayoutInfo().getMinSize().getX()));
}

void
client::widgets::FlakVcrInfo::updateButtonState()
{
    m_showMapButton.setState(DisabledState, !m_data.position.isValid());
}

void
client::widgets::FlakVcrInfo::onMap()
{
    if (const game::map::Point* pt = m_data.position.get()) {
        sig_showMap.raise(*pt);
    }
}
