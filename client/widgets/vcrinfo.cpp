/**
  *  \file client/widgets/vcrinfo.cpp
  *  \brief Class client::widgets::VcrInfo
  */

#include "client/widgets/vcrinfo.hpp"
#include "afl/string/format.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "util/string.hpp"

namespace {
    /* Number of units in classic combat */
    const size_t NUM_CLASSIC_UNITS = 2;

    /* Check for last ship in a group */
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

client::widgets::VcrInfo::VcrInfo(ui::Root& root, afl::string::Translator& tx)
    : m_root(root),
      m_translator(tx),
      m_leftButton("L", 'l', root),
      m_rightButton("R", 'r', root),
      m_tabButton("Tab", util::Key_Tab, root),
      m_menuButton("#", '#', root),
      m_showMapButton("F4", util::Key_F4, root),
      m_data(),
      m_adjectiveNames(),
      m_teamSettings()
{
    // ex FlakVcrSelector::FlakVcrSelector
    // Do not add m_rightButton yet
    addChild(m_leftButton, 0);
    addChild(m_tabButton, 0);
    addChild(m_menuButton, 0);
    addChild(m_showMapButton, 0);

    m_leftButton.sig_fire.add(this, &VcrInfo::onLeft);
    m_rightButton.sig_fire.add(this, &VcrInfo::onRight);
    m_tabButton.sig_fire.add(this, &VcrInfo::onTab);
    m_menuButton.sig_fire.add(this, &VcrInfo::onMenu);
    m_showMapButton.sig_fire.add(this, &VcrInfo::onMap);

    updateButtonState();
}

client::widgets::VcrInfo::~VcrInfo()
{ }

void
client::widgets::VcrInfo::draw(gfx::Canvas& can)
{
    // ex FlakVcrSelector::drawContent etc.

    // Prepare coordinates
    const gfx::Rectangle& r = getExtent();
    const int x = r.getLeftX();
    const int w = r.getWidth();
    int y = r.getTopY();

    // Prepare fonts
    afl::base::Ref<gfx::Font> largeFont  = getLargeFont();
    afl::base::Ref<gfx::Font> normalFont = getNormalFont();
    afl::base::Ref<gfx::Font> boldFont   = getBoldFont();
    afl::base::Ref<gfx::Font> smallFont  = m_root.provider().getFont("-");

    const int largeHeight  = largeFont->getCellSize().getY();
    const int normalHeight = normalFont->getCellSize().getY();
    const int boldHeight   = boldFont->getCellSize().getY();
    const int indent       = normalHeight/2;

    // Prepare context
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    drawBackground(ctx, r);
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
    if (const int32_t* seed = m_data.seed.get()) {
        ctx.useFont(*smallFont);
        ctx.setColor(util::SkinColor::Faded);
        outText(ctx, gfx::Point(x+w, y+normalHeight), afl::string::Format("#%d", *seed));
    }
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);

    y += largeHeight;
    y += normalHeight/2;

    // Content-dependant layout
    const int nships  = static_cast<int>(m_data.units.size());
    const int nfleets = static_cast<int>(m_data.groups.size());
    if (isClassic()) {
        // Two warriors
        for (size_t i = 0; i < NUM_CLASSIC_UNITS && i < m_data.units.size(); ++i) {
            const game::vcr::ObjectInfo& info = m_data.units[i];

            ctx.useFont(*boldFont);
            ctx.setColor(info.color[0]);
            outTextF(ctx, gfx::Point(x, y), w, info.text[0]);
            y += boldHeight;

            for (size_t j = 1; j < game::vcr::NUM_LINES_PER_UNIT; ++j) {
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
    } else if (normalHeight > 0 && nships > 0 && nfleets > 0) {
        // Only draw content if there is any (to fill the temporary state where m_data has just a heading, no content)
        // This is a convenient place to fend off zero-size fonts which would give a division error.
        ctx.useFont(*boldFont);
        ctx.setColor(util::SkinColor::Static);
        outTextF(ctx, gfx::Point(x, y), w, afl::string::Format(m_translator("%d unit%!1{s%} in %d group%!1{s%}:"), nships, nfleets));
        y += normalHeight;

        int n = (r.getBottomY() - y) / normalHeight;
        ctx.useFont(*normalFont);
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
client::widgets::VcrInfo::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::VcrInfo::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::widgets::VcrInfo::handleChildAdded(Widget& /*child*/)
{
    requestRedraw();
}

void
client::widgets::VcrInfo::handleChildRemove(Widget& /*child*/)
{
    requestRedraw();
}

void
client::widgets::VcrInfo::handlePositionChange()
{
    setChildPositions();
}

void
client::widgets::VcrInfo::handleChildPositionChange(Widget& /*child*/, const gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::widgets::VcrInfo::getLayoutInfo() const
{
    // Font sizes
    gfx::Point normalCell = getNormalFont()->getCellSize();
    gfx::Point largeCell  = getLargeFont()->getCellSize();

    // Classic dialog 8.5x normal + 2x bold + 1x large, where normal+bold essentially is 10.5x normal.
    // The FLAK version uses 13x normal, so this is enough space for both layouts.
    gfx::Point size = largeCell.scaledBy(20, 1)
        .extendBelow(normalCell.scaledBy(40, 13));
    size.addY(normalCell.getY() / 2);

    return ui::layout::Info(size, ui::layout::Info::GrowBoth);
}

bool
client::widgets::VcrInfo::handleKey(util::Key_t key, int prefix)
{
    switch (key) {
     case 's':
        sig_action.raise(ShowScoreSummary);
        return true;
     case 'o':
        sig_action.raise(EditOptions);
        return true;
     default:
        return defaultHandleKey(key, prefix);
    }
}

bool
client::widgets::VcrInfo::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::widgets::VcrInfo::setPlayerNames(const game::PlayerArray<String_t>& adjNames)
{
    m_adjectiveNames = adjNames;
    requestRedraw();
}

void
client::widgets::VcrInfo::setTeams(const game::TeamSettings& teams)
{
    m_teamSettings.copyFrom(teams);
    requestRedraw();
}

void
client::widgets::VcrInfo::setData(const Data_t& data)
{
    m_data = data;
    requestRedraw();
    setChildPositions();           // First, so it sets the position of 'R'...
    updateButtonState();           // ...if that is shown by this.
}

void
client::widgets::VcrInfo::setTabAvailable(bool flag)
{
    m_tabButton.setState(DisabledState, !flag);
}

afl::base::Ref<gfx::Font>
client::widgets::VcrInfo::getLargeFont() const
{
    return m_root.provider().getFont(gfx::FontRequest().addSize(1));
}

afl::base::Ref<gfx::Font>
client::widgets::VcrInfo::getNormalFont() const
{
    return m_root.provider().getFont(gfx::FontRequest());
}

afl::base::Ref<gfx::Font>
client::widgets::VcrInfo::getBoldFont() const
{
    return m_root.provider().getFont(gfx::FontRequest().addWeight(1));
}

bool
client::widgets::VcrInfo::isClassic() const
{
    return m_data.units.size() == NUM_CLASSIC_UNITS
        && m_data.groups.size() == NUM_CLASSIC_UNITS;
}

void
client::widgets::VcrInfo::setChildPositions()
{
    // Metrics
    const int largeHeight  = getLargeFont()->getCellSize().getY();
    const int normalHeight = getNormalFont()->getCellSize().getY();
    const int boldHeight   = getBoldFont()->getCellSize().getY();
    const int buttonSize   = largeHeight * 9/8;
    const int tabSize      = buttonSize * 8/5;
    const int pad          = 5;

    gfx::Rectangle area = getExtent();
    gfx::Rectangle lastRow = area.splitBottomY(buttonSize);

    if (isClassic()) {
        // Classic layout with L/R buttons
        const int rightX = area.getRightX();
        const int leftTop = area.getTopY() + largeHeight + normalHeight/2;
        m_leftButton.setExtent(gfx::Rectangle(rightX - buttonSize, leftTop, buttonSize, buttonSize));

        const int rightTop = leftTop + boldHeight + normalHeight*7/2;
        m_rightButton.setExtent(gfx::Rectangle(rightX - buttonSize, rightTop, buttonSize, buttonSize));

        m_tabButton.setExtent(lastRow.splitRightX(tabSize));
        lastRow.consumeRightX(pad);

        m_menuButton.setExtent(lastRow.splitRightX(buttonSize));
        lastRow.consumeRightX(pad);

        m_showMapButton.setExtent(lastRow.splitRightX(m_showMapButton.getLayoutInfo().getPreferredSize().getX()));
    } else {
        // Fleet-combat layout with just L button
        m_tabButton.setExtent(lastRow.splitRightX(tabSize));
        lastRow.consumeRightX(pad);

        m_menuButton.setExtent(lastRow.splitRightX(buttonSize));
        lastRow.consumeRightX(pad);

        m_leftButton.setExtent(lastRow.splitRightX(buttonSize));
        lastRow.consumeRightX(pad);

        m_showMapButton.setExtent(lastRow.splitRightX(m_showMapButton.getLayoutInfo().getPreferredSize().getX()));
    }
}

void
client::widgets::VcrInfo::updateButtonState()
{
    // Enable map button if position is available
    m_showMapButton.setState(DisabledState, !m_data.position.isValid());

    // Show or hide 'R' button
    if (isClassic()) {
        if (m_rightButton.getParent() == 0) {
            addChild(m_rightButton, 0);
        }
    } else {
        if (m_rightButton.getParent() != 0) {
            removeChild(m_rightButton);
        }
    }
}

void
client::widgets::VcrInfo::onLeft()
{
    sig_info.raise(0);
}

void
client::widgets::VcrInfo::onRight()
{
    sig_info.raise(1);
}

void
client::widgets::VcrInfo::onTab()
{
    sig_action.raise(ShowCombatDiagram);
}

void
client::widgets::VcrInfo::onMenu()
{
    ui::widgets::StringListbox box(m_root.provider(), m_root.colorScheme());
    box.addItem(ShowScoreSummary, m_translator("Scores [S]"));
    box.addItem(ExportBattles,    m_translator("Export all battles"));
    box.addItem(ExportUnits,      m_translator("Export units in this battle"));
    box.addItem(SaveAllBattles,   m_translator("Save all battles"));
    box.addItem(SaveThisBattle,   m_translator("Save this battle"));
    box.addItem(EditOptions,      m_translator("VCR options [O]"));

    gfx::Point anchor = m_menuButton.getExtent().getBottomLeft();
    ui::EventLoop loop(m_root);
    if (ui::widgets::MenuFrame(ui::layout::HBox::instance0, m_root, loop).doMenu(box, anchor)) {
        int32_t id = 0;
        if (box.getCurrentKey().get(id)) {
            sig_action.raise(Action(id));
        }
    }
}

void
client::widgets::VcrInfo::onMap()
{
    if (const game::map::Point* pt = m_data.position.get()) {
        sig_showMap.raise(*pt);
    }
}
