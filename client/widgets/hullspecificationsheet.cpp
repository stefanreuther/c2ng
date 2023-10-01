/**
  *  \file client/widgets/hullspecificationsheet.cpp
  */

#include "client/widgets/hullspecificationsheet.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/specbrowserdialog.hpp"
#include "client/widgets/playerlist.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/res/resid.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/framegroup.hpp"

using afl::string::Format;

namespace {
    const int PAD = 5;

    const size_t NUM_HULLFUNC_LINES = 7;

    void initBaseTable(ui::widgets::SimpleTable& tab, int em, afl::string::Translator& tx)
    {
        // ex WSpecBaseInfo::drawContent (part)
        tab.column(0).setColor(ui::Color_Gray);
        tab.column(1).setColor(ui::Color_Green).setTextAlign(gfx::RightAlign, gfx::TopAlign);
        tab.column(2).setColor(ui::Color_Green);
        tab.setColumnPadding(1, PAD);
        tab.setColumnPadding(2, PAD);
        tab.setColumnWidth(1, 5*em);
        tab.cell(0, 0).setText(tx("Mass:"));
        tab.cell(0, 1).setText(tx("Engines:"));
        tab.cell(0, 2).setText(tx("Tech:"));
        tab.cell(0, 3).setText(tx("Crew:"));
        tab.cell(0, 4).setText(tx("Cargo:"));
        tab.cell(0, 5).setText(tx("Fuel:"));

        tab.cell(2, 0).setText(tx("kt"));
        tab.cell(2, 4).setText(tx("kt"));
        tab.cell(2, 5).setText(tx("kt"));
    }

    void setBaseTable(ui::widgets::SimpleTable& tab, const game::proxy::HullSpecificationProxy::HullSpecification& data, const util::NumberFormatter& fmt)
    {
        // ex WSpecBaseInfo::drawContent (part)
        tab.cell(1, 0).setText(fmt.formatNumber(data.mass));
        tab.cell(1, 1).setText(fmt.formatNumber(data.numEngines));
        tab.cell(1, 2).setText(fmt.formatNumber(data.techLevel));
        tab.cell(1, 3).setText(fmt.formatNumber(data.maxCrew));
        tab.cell(1, 4).setText(fmt.formatNumber(data.maxCargo));
        tab.cell(1, 5).setText(fmt.formatNumber(data.maxFuel));
    }

    void initBuildTable(ui::widgets::SimpleTable& tab, int em, afl::string::Translator& tx)
    {
        // WSpecBuildInfo::drawContent (part)
        tab.column(0).setColor(ui::Color_Gray);
        tab.column(1).setColor(ui::Color_Green).setTextAlign(gfx::RightAlign, gfx::TopAlign);
        tab.column(2).setColor(ui::Color_Green);
        tab.column(3).setColor(ui::Color_Gray);
        tab.column(4).setColor(ui::Color_Green).setTextAlign(gfx::RightAlign, gfx::TopAlign);

        tab.row(0).setColor(ui::Color_White);

        tab.setColumnPadding(0, PAD);
        tab.setColumnPadding(1, PAD);
        tab.setColumnPadding(2, em);
        tab.setColumnPadding(3, PAD);

        tab.setColumnWidth(1, 4*em);
        tab.setColumnWidth(4, 4*em);

        tab.cell(0, 0).setText(tx("Resources Needed")).setExtraColumns(2);
        tab.cell(0, 1).setText(tx("Money:"));
        tab.cell(0, 2).setText(tx("Tritanium:"));
        tab.cell(0, 3).setText(tx("Duranium:"));
        tab.cell(0, 4).setText(tx("Molybdenum:"));

        tab.cell(2, 1).setText(tx("mc"));
        tab.cell(2, 2).setText(tx("kt"));
        tab.cell(2, 3).setText(tx("kt"));
        tab.cell(2, 4).setText(tx("kt"));

        tab.cell(3, 0).setText(tx("Build Points")).setExtraColumns(1);      // <- FIXME: conditional
        tab.cell(3, 1).setText(tx("Build:"));
        tab.cell(3, 2).setText(tx("Kill:"));
        tab.cell(3, 3).setText(tx("Scrap:"));
        tab.cell(3, 4).setText(tx("You have:"));

        // Reset column widths to force recomputation.
        // Required as of 20180831 because each action causes an immediate re-layout, and auto-columns never shrink.
        tab.clearColumnWidth(0);
        tab.clearColumnWidth(2);
        tab.clearColumnWidth(3);
    }

    void setBuildTable(ui::widgets::SimpleTable& tab, const game::proxy::HullSpecificationProxy::HullSpecification& data, const util::NumberFormatter& fmt)
    {
        // ex WSpecBuildInfo::drawContent (part)
        tab.cell(1, 1).setText(fmt.formatNumber(data.cost.get(game::spec::Cost::Money)));
        tab.cell(1, 2).setText(fmt.formatNumber(data.cost.get(game::spec::Cost::Tritanium)));
        tab.cell(1, 3).setText(fmt.formatNumber(data.cost.get(game::spec::Cost::Duranium)));
        tab.cell(1, 4).setText(fmt.formatNumber(data.cost.get(game::spec::Cost::Molybdenum)));

        tab.cell(4, 1).setText(fmt.formatNumber(data.pointsToBuild));
        tab.cell(4, 2).setText(fmt.formatNumber(data.pointsForKilling));
        tab.cell(4, 3).setText(fmt.formatNumber(data.pointsForScrapping));
        tab.cell(4, 4).setText(fmt.formatNumber(data.pointsAvailable));
    }

    void renderAttribute(ui::rich::Document& doc, String_t label, String_t value)
    {
        doc.add(label);
        doc.add(": ");
        doc.add(util::rich::Text(util::SkinColor::Green, value));
        doc.addNewline();
    }

    void setHullFunctions(ui::rich::DocumentView& docView, ui::Root& root, const game::proxy::HullSpecificationProxy::HullSpecification& data, const util::NumberFormatter& fmt, bool useIcons, afl::string::Translator& tx)
    {
        // Extra attributes
        ui::rich::Document& doc = docView.getDocument();
        doc.clear();
        size_t numLines = NUM_HULLFUNC_LINES;

        // - Weapons
        String_t w;
        if (data.maxBeams > 0) {
            w += Format(tx("%d beam%!1{s%}"), data.maxBeams);
        }
        if (data.maxLaunchers > 0) {
            if (!w.empty()) {
                w += ", ";
            }
            w += Format(tx("%d torpedo launcher%!1{s%}"), data.maxLaunchers);
        }
        if (data.numBays > 0) {
            if (!w.empty()) {
                w += ", ";
            }
            w += Format(tx("%d fighter bay%!1{s%}"), data.numBays);
        }
        if (w.empty()) {
            w = tx("none");
        }
        renderAttribute(doc, tx("Weapons"), w);
        --numLines;

        // - Mine hit damage
        renderAttribute(doc, tx("Mine Hit"), Format(tx("%d%% damage"), data.mineHitDamage));
        --numLines;

        // - Hull Id
        renderAttribute(doc, tx("Hull Id"), fmt.formatNumber(data.hullId));
        --numLines;

        // - Fuel usage
        if (data.fuelBurnPerTurn != 0 || data.fuelBurnPerFight != 0) {
            renderAttribute(doc, "Fuel burn", Format(tx("%d kt/turn, %d kt/fight"), data.fuelBurnPerTurn, data.fuelBurnPerFight));
            --numLines;
        }

        // Hull abilities
        client::dialogs::renderAbilityList(doc, root, data.abilities, useIcons, numLines, tx);
        doc.finish();
        docView.handleDocumentUpdate();
    }
}


client::widgets::HullSpecificationSheet::HullSpecificationSheet(ui::Root& root,
                                                                afl::string::Translator& tx,
                                                                game::PlayerSet_t allPlayers,
                                                                const game::PlayerArray<String_t>& playerNames,
                                                                util::NumberFormatter fmt,
                                                                bool useIcons)
    : Group(ui::layout::VBox::instance5),
      m_deleter(),
      m_root(root),
      m_translator(tx),
      m_formatter(fmt),
      m_useIcons(useIcons),
      m_pTitle(),
      m_pImage(),
      m_pBaseTable(),
      m_pBuildTable(),
      m_pHullFunctions()
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
    if (m_pBaseTable != 0) {
        setBaseTable(*m_pBaseTable, data, m_formatter);
    }
    if (m_pBuildTable != 0) {
        setBuildTable(*m_pBuildTable, data, m_formatter);
    }
    if (m_pHullFunctions != 0) {
        setHullFunctions(*m_pHullFunctions, m_root, data, m_formatter, m_useIcons, m_translator);
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
    const gfx::Point cellSize = m_root.provider().getFont(gfx::FontRequest())->getCellSize();
    const int em = cellSize.getX();

    // Title
    m_pTitle = &m_deleter.addNew(new ui::widgets::StaticText(String_t(), util::SkinColor::Heading, gfx::FontRequest().addSize(1), m_root.provider()));
    m_pTitle->setIsFlexible(true);
    add(*m_pTitle);

    // Image || First Table
    m_pImage = &m_deleter.addNew(new ui::widgets::ImageButton(String_t(), 0, m_root, gfx::Point(105, 95)));
    m_pImage->setBackgroundColor(ui::Color_Black);
    m_pBaseTable = &m_deleter.addNew(new ui::widgets::SimpleTable(m_root, 3, 6));
    initBaseTable(*m_pBaseTable, em, m_translator);

    Group& g1 = m_deleter.addNew(new Group(ui::layout::HBox::instance5));
    g1.add(ui::widgets::FrameGroup::wrapWidget(m_deleter, m_root.colorScheme(), ui::LoweredFrame, *m_pImage));
    g1.add(*m_pBaseTable);
    g1.add(m_deleter.addNew(new ui::Spacer()));
    add(g1);

    // Hull functions (+more)
    m_pHullFunctions = &m_deleter.addNew(new ui::rich::DocumentView(cellSize.scaledBy(30, int(NUM_HULLFUNC_LINES)), 0, m_root.provider()));
    add(*m_pHullFunctions);

    // Third Table
    m_pBuildTable = &m_deleter.addNew(new ui::widgets::SimpleTable(m_root, 5, 5));
    add(*m_pBuildTable);
    initBuildTable(*m_pBuildTable, em, m_translator);
}

void
client::widgets::HullSpecificationSheet::initPlayerLists(game::PlayerSet_t allPlayers, const game::PlayerArray<String_t>& playerNames)
{
    Group& outer = m_deleter.addNew(new Group(ui::layout::VBox::instance0));

    // Player lists
    outer.add(m_deleter.addNew(new ui::widgets::StaticText(m_translator("Players"), util::SkinColor::Heading, gfx::FontRequest(), m_root.provider())));

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
        m_pPlayerLists[i] = &m_deleter.addNew(new PlayerList(m_root, PlayerList::VerticalLayout, PlayerList::ShowNames, PlayerList::SameColors, 100, allPlayers.take(numLines)));
        m_pPlayerLists[i]->setNames(playerNames);
        m_pPlayerLists[i]->sig_playerClick.add(&sig_playerClick, &afl::base::Signal<void(int)>::raise);
        inner.add(*m_pPlayerLists[i]);
    }
    outer.add(inner);
    add(outer);
}
