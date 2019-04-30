/**
  *  \file client/widgets/hullspecificationsheet.cpp
  */

#include "client/widgets/hullspecificationsheet.hpp"
#include "afl/string/format.hpp"
#include "client/widgets/playerlist.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/framegroup.hpp"
#include "util/translation.hpp"
#include "ui/res/resid.hpp"

using afl::string::Format;

namespace {
    const int PAD = 5;

    game::PlayerSet_t takePlayers(game::PlayerSet_t& set, int count)
    {
        game::PlayerSet_t result;
        for (int i = 1; i <= game::MAX_PLAYERS && count > 0; ++i) {
            if (set.contains(i)) {
                set -= i;
                result += i;
                --count;
            }
        }
        return result;
    }

    void initFirstTable(ui::widgets::SimpleTable& tab, int em)
    {
        // ex WSpecBaseInfo::drawContent (part)
        tab.column(0).setColor(ui::Color_Gray);
        tab.column(1).setColor(ui::Color_Green).setTextAlign(2, 0);
        tab.column(2).setColor(ui::Color_Green);
        tab.setColumnPadding(1, PAD);
        tab.setColumnPadding(2, PAD);
        tab.setColumnWidth(1, 5*em);
        tab.cell(0, 0).setText(_("Mass:"));
        tab.cell(0, 1).setText(_("Engines:"));
        tab.cell(0, 2).setText(_("Tech:"));
        tab.cell(0, 3).setText(_("Crew:"));
        tab.cell(0, 4).setText(_("Cargo:"));
        tab.cell(0, 5).setText(_("Fuel:"));

        tab.cell(2, 0).setText(_("kt"));
        tab.cell(2, 4).setText(_("kt"));
        tab.cell(2, 5).setText(_("kt"));
    }

    void setFirstTable(ui::widgets::SimpleTable& tab, const client::proxy::HullSpecificationProxy::HullSpecification& data, const util::NumberFormatter& fmt)
    {
        // ex WSpecBaseInfo::drawContent (part)
        tab.cell(1, 0).setText(fmt.formatNumber(data.mass));
        tab.cell(1, 1).setText(fmt.formatNumber(data.numEngines));
        tab.cell(1, 2).setText(fmt.formatNumber(data.techLevel));
        tab.cell(1, 3).setText(fmt.formatNumber(data.maxCrew));
        tab.cell(1, 4).setText(fmt.formatNumber(data.maxCargo));
        tab.cell(1, 5).setText(fmt.formatNumber(data.maxFuel));
    }

    void initSecondTable(ui::widgets::SimpleTable& tab)
    {
        // ex WSpecMainInfo::drawContent (part)
        tab.column(0).setColor(ui::Color_Gray);
        tab.column(1).setColor(ui::Color_Green);
        tab.setColumnPadding(0, PAD);
        tab.cell(0, 0).setText(_("Weapons:"));
        tab.cell(0, 1).setText(_("Mine Hit:"));
        tab.cell(0, 2).setText(_("Hull Id:"));
        // FIXME -> tab.cell(0, 3).setText(_("..."));
    }

    void setSecondTable(ui::widgets::SimpleTable& tab, const client::proxy::HullSpecificationProxy::HullSpecification& data, const util::NumberFormatter& fmt)
    {
        // ex WSpecMainInfo::drawContent
        String_t w;
        if (data.maxBeams > 0) {
            w += Format(_("%d beam%!1{s%}").c_str(), data.maxBeams);
        }
        if (data.maxLaunchers > 0) {
            if (!w.empty()) {
                w += ", ";
            }
            w += Format(_("%d torpedo launcher%!1{s%}").c_str(), data.maxLaunchers);
        }
        if (data.numBays > 0) {
            if (!w.empty()) {
                w += ", ";
            }
            w += Format(_("%d fighter bay%!1{s%}").c_str(), data.numBays);
        }
        if (w.empty()) {
            w = _("none");
        }
        tab.cell(1, 0).setText(w);
        tab.cell(1, 1).setText(Format(_("%d%% damage").c_str(), data.mineHitDamage));
        tab.cell(1, 2).setText(fmt.formatNumber(data.hullId));

        // FIXME: hullfuncs
    }

    void initThirdTable(ui::widgets::SimpleTable& tab, int em)
    {
        // WSpecBuildInfo::drawContent (part)
        tab.column(0).setColor(ui::Color_Gray);
        tab.column(1).setColor(ui::Color_Green).setTextAlign(2, 0);
        tab.column(2).setColor(ui::Color_Green);
        tab.column(3).setColor(ui::Color_Gray);
        tab.column(4).setColor(ui::Color_Green).setTextAlign(2, 0);

        tab.row(0).setColor(ui::Color_White);

        tab.setColumnPadding(0, PAD);
        tab.setColumnPadding(1, PAD);
        tab.setColumnPadding(2, em);
        tab.setColumnPadding(3, PAD);

        tab.setColumnWidth(1, 4*em);
        tab.setColumnWidth(4, 4*em);

        tab.cell(0, 0).setText(_("Resources Needed")).setExtraColumns(2);
        tab.cell(0, 1).setText(_("Money:"));
        tab.cell(0, 2).setText(_("Tritanium:"));
        tab.cell(0, 3).setText(_("Duranium:"));
        tab.cell(0, 4).setText(_("Molybdenum:"));

        tab.cell(2, 1).setText(_("mc"));
        tab.cell(2, 2).setText(_("kt"));
        tab.cell(2, 3).setText(_("kt"));
        tab.cell(2, 4).setText(_("kt"));

        tab.cell(3, 0).setText(_("Build Points")).setExtraColumns(1);      // <- FIXME: conditional
        tab.cell(3, 1).setText(_("Build:"));
        tab.cell(3, 2).setText(_("Kill:"));
        tab.cell(3, 3).setText(_("Scrap:"));
        tab.cell(3, 4).setText(_("You have:"));

        // Reset column widths to force recomputation.
        // Required as of 20180831 because each action causes an immediate re-layout, and auto-columns never shrink.
        tab.clearColumnWidth(0);
        tab.clearColumnWidth(2);
        tab.clearColumnWidth(3);
    }

    void setThirdTable(ui::widgets::SimpleTable& tab, const client::proxy::HullSpecificationProxy::HullSpecification& data, const util::NumberFormatter& fmt)
    {
        // ex WSpecBuildInfo::drawContent (part)
        tab.cell(1, 1).setText(fmt.formatNumber(data.cost.get(game::spec::Cost::Money)));
        tab.cell(1, 2).setText(fmt.formatNumber(data.cost.get(game::spec::Cost::Tritanium)));
        tab.cell(1, 3).setText(fmt.formatNumber(data.cost.get(game::spec::Cost::Duranium)));
        tab.cell(1, 4).setText(fmt.formatNumber(data.cost.get(game::spec::Cost::Molybdenum)));

        tab.cell(4, 1).setText(fmt.formatNumber(data.pointsToBuild));
        tab.cell(4, 2).setText(fmt.formatNumber(data.pointsForKilling));
        tab.cell(4, 3).setText(fmt.formatNumber(data.pointsForScrapping));
    }
}


client::widgets::HullSpecificationSheet::HullSpecificationSheet(ui::Root& root,
                                                                bool hasPerTurnCosts,
                                                                game::PlayerSet_t allPlayers,
                                                                const game::PlayerArray<String_t>& playerNames,
                                                                util::NumberFormatter fmt)
    : Group(ui::layout::VBox::instance5),
      m_deleter(),
      m_root(root),
      m_hasPerTurnCosts(hasPerTurnCosts),
      m_formatter(fmt)
{
    init();
    initPlayerLists(allPlayers, playerNames);
}

void
client::widgets::HullSpecificationSheet::setContent(const HullSpecification_t& data)
{
    if (m_pTitle != 0) {
        m_pTitle->setText(data.name);
    }
    if (m_pImage != 0) {
        m_pImage->setImage(data.image.empty() ? RESOURCE_ID("nvc") : data.image);
    }
    if (m_pTables[0] != 0) {
        setFirstTable(*m_pTables[0], data, m_formatter);
    }
    if (m_pTables[1] != 0) {
        setSecondTable(*m_pTables[1], data, m_formatter);
    }
    if (m_pTables[2] != 0) {
        setThirdTable(*m_pTables[2], data, m_formatter);
    }
    for (int i = 0; i < 3; ++i) {
        if (m_pPlayerLists[i] != 0) {
            m_pPlayerLists[i]->setHighlightedPlayers(data.players);
        }
    }
}

void
client::widgets::HullSpecificationSheet::init()
{
    // ex WSpecView::init
    const int em = m_root.provider().getFont(gfx::FontRequest())->getCellSize().getX();

    // Title
    m_pTitle = &m_deleter.addNew(new ui::widgets::StaticText(String_t(), util::SkinColor::Heading, gfx::FontRequest().addSize(1), m_root.provider()));
    m_pTitle->setIsFlexible(true);
    add(*m_pTitle);

    // Image || First Table
    m_pImage = &m_deleter.addNew(new ui::widgets::ImageButton(String_t(), 0, m_root, gfx::Point(105, 95)));
    m_pTables[0] = &m_deleter.addNew(new ui::widgets::SimpleTable(m_root, 3, 6));
    initFirstTable(*m_pTables[0], em);

    Group& g1 = m_deleter.addNew(new Group(ui::layout::HBox::instance5));
    g1.add(ui::widgets::FrameGroup::wrapWidget(m_deleter, m_root.colorScheme(), ui::widgets::FrameGroup::LoweredFrame, *m_pImage));
    g1.add(*m_pTables[0]);
    g1.add(m_deleter.addNew(new ui::Spacer()));
    add(g1);

    // Second Table
    m_pTables[1] = &m_deleter.addNew(new ui::widgets::SimpleTable(m_root, 2, 7));
    add(*m_pTables[1]);
    initSecondTable(*m_pTables[1]);

    // Third Table
    m_pTables[2] = &m_deleter.addNew(new ui::widgets::SimpleTable(m_root, 5, 5));
    add(*m_pTables[2]);
    initThirdTable(*m_pTables[2], em);

    // FIXME: make it possible for a table to contain multi-column text
}

void
client::widgets::HullSpecificationSheet::initPlayerLists(game::PlayerSet_t allPlayers, const game::PlayerArray<String_t>& playerNames)
{
    Group& outer = m_deleter.addNew(new Group(ui::layout::VBox::instance0));

    // Player lists
    outer.add(m_deleter.addNew(new ui::widgets::StaticText(_("Players"), util::SkinColor::Heading, gfx::FontRequest(), m_root.provider())));

    // - Count the players
    int numPlayers = 0;
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        if (allPlayers.contains(i)) {
            ++numPlayers;
        }
    }
    int numLines = (numPlayers == 0 ? 1 : (numPlayers + 2) / 3);

    Group& inner = m_deleter.addNew(new Group(ui::layout::HBox::instance5));
    for (int i = 0; i < 3; ++i) {
        m_pPlayerLists[i] = &m_deleter.addNew(new PlayerList(m_root, PlayerList::VerticalLayout, PlayerList::ShowNames, PlayerList::SameColors, 100, takePlayers(allPlayers, numLines)));
        m_pPlayerLists[i]->setNames(playerNames);
        m_pPlayerLists[i]->sig_playerClick.add(&sig_playerClick, &afl::base::Signal<void(int)>::raise);
        inner.add(*m_pPlayerLists[i]);
    }
    outer.add(inner);
    add(outer);
}
