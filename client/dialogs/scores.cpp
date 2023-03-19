/**
  *  \file client/dialogs/scores.cpp
  *  \brief Score Dialog
  */

#include <vector>
#include <algorithm>
#include "client/dialogs/scores.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/playerlist.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/scoreproxy.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/cardgroup.hpp"
#include "ui/group.hpp"
#include "ui/icons/skintext.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/chart.hpp"
#include "ui/widgets/iconbox.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/simpletable.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/numberformatter.hpp"
#include "util/string.hpp"
#include "util/unicodechars.hpp"

using game::proxy::ScoreProxy;
using ui::Group;
using ui::widgets::Button;
using ui::widgets::SimpleTable;
using ui::widgets::StaticText;
using util::DataTable;

namespace {
    /*
     *  Widget Parameters
     */
    const size_t COLUMNS_PER_TABLE = 5;

    const int CHART_WIDTH = 500;
    const int CHART_HEIGHT = 300;

    // Line layer Z's
    const int NORMAL_Z = 10;
    const int CURRENT_PLAYER_Z = 11;
    const int GRID_Z = 0;

    // Line Ids (share namespace with player Ids)
    const int DECAY_LINE_EVEN_ID = 1000;
    const int DECAY_LINE_ODD_ID = 1001;
    const int GRID_LINE_ID = 1002;
    const int WINLIMIT_LINE_ID = 1003;

    // Icon Ids (arbitrary)
    const int CHART_MODE_ICON_ID = 40;

    const uint8_t COLOR_NORMAL   = ui::Color_Black;
    const uint8_t COLOR_GOOD     = ui::Color_GreenBlack;
    const uint8_t COLOR_BAD      = ui::Color_Red;
    const uint8_t COLOR_FADED    = ui::Color_Dark;
    const uint8_t COLOR_SELECTED = ui::Color_Blue;
    const uint8_t COLOR_GRID     = ui::Color_Dark;
    const uint8_t COLOR_WINLIMIT = ui::Color_Black;


    /*
     *  Utilities
     */

    /* Comparison predicate to sort a score table's rows */
    class CompareRowsByColumn : public afl::functional::BinaryFunction<const DataTable::Row&, const DataTable::Row&, bool> {
     public:
        CompareRowsByColumn(int row)
            : m_row(row)
            { }
        virtual bool get(const DataTable::Row& a, const DataTable::Row& b) const
            {
                if (m_row >= 0) {
                    int32_t aval, bval;
                    bool aok = a.get(m_row).get(aval);
                    bool bok = b.get(m_row).get(bval);
                    if (!aok || !bok) {
                        return aok > bok;
                    }
                    if (aval != bval) {
                        return aval > bval;
                    }
                }
                return a.getId() < b.getId();
            }
     private:
        int m_row;
    };

    /* Utility for finding a list item, given a key.
       If multiple items match one key, picks them in sequence (with wraparound). */
    template<typename Index>
    class Matcher {
     public:
        Matcher()
            : m_foundActive(false), m_foundAny(false), m_foundIndex(0)
            { }
        bool operator()(Index index, bool isActive)
            {
                if (isActive) {
                    // Found active item; keep searching.
                    m_foundActive = true;
                    return false;
                } else if (m_foundActive) {
                    // We previously found the active item, followed by another one: accept this.
                    m_foundAny = true;
                    m_foundIndex = index;
                    return true;
                } else if (!m_foundAny) {
                    // Didn't find anything so far, so remember this one.
                    m_foundAny = true;
                    m_foundIndex = index;
                    return false;
                } else {
                    return false;
                }
            }
        bool isValid() const
            { return m_foundAny; }
        Index getIndex() const
            { return m_foundIndex; }
     private:
        bool m_foundActive;
        bool m_foundAny;
        Index m_foundIndex;
    };

    /* Given a DataTable, finds currentPlayer's row, and than goes forward or backward one.
       If allowZero is set, leaving the table selects player 0, otherwise wraps. */
    int findNextPlayerInTable(const DataTable& tab, int currentPlayer, bool forward, bool allowZero)
    {
        const size_t limit = tab.getNumRows();
        const DataTable::Row* currentRow = tab.findRowById(currentPlayer);

        size_t newLine;
        if (currentRow == 0) {
            // Nothing selected: start at top/bottom
            newLine = forward ? 0 : limit-1;
        } else {
            // Pick next line. With allowZero set, allow exceeding the range; the 'getRow' check below will then return 0.
            size_t current = currentRow->getIndex();
            if (forward) {
                newLine = allowZero || current < limit-1 ? current+1 : 0;
            } else {
                newLine = allowZero || current > 0 ? current-1 : limit-1;
            }
        }

        if (const DataTable::Row* r = tab.getRow(newLine)) {
            return r->getId();
        } else {
            return 0;
        }
    }


    /* Kind of score tab. */
    enum ScoreKind {
        Table,
        Chart
    };

    /* Description of a score tab. */
    struct ScoreTab {
        ScoreKind kind;         ///< Kind of page.
        String_t  name;         ///< Name of page. Shown on button and in heading.
        util::Key_t key;
        size_t    index;        ///< Table: first column; Chart: index
        int       decay;        ///< Table: decay rate (from ScoreBuilderBase::Variant)
        int32_t   winLimit;     ///< Table: win limit (from ScoreBuilderBase::Variant)

        ScoreTab(ScoreKind kind, String_t name, size_t index, int decay, int32_t winLimit)
            : kind(kind), name(name), key(0), index(index), decay(decay), winLimit(winLimit)
            { }
        ScoreTab()
            : kind(), name(), key(), index(), decay(), winLimit()
            { }
    };

    /* List of score tabs */
    typedef std::vector<ScoreTab> ScoreTabs_t;  // ex ScoreTabVector

    /* Assign keys to all score tabs.
       Tries to pick word starters (i.e. "Planets" -> "P"), avoiding hardcoded letters,
       but will assign duplicated if it cannot be done otherwise. */
    void assignKeys(ScoreTabs_t& tabs)
    {
        const uint32_t USED_LETTERS
            = (1 << ('d'-'a'))                      // display mode menu
            | (1 << ('y'-'a'))                      // team toggle
            | (1 << ('x'-'a'))                      // cumulative toggle
            | (1 << ('u'-'a'))                      // with Ctrl, unsorted
            | (1 << ('h'-'a'));                     // help

        uint32_t used_letters = USED_LETTERS;
        for (ScoreTabs_t::iterator i = tabs.begin(); i != tabs.end(); ++i) {
            // First check whether any word starter can be a key
            char c = 0;
            bool look_at_this_char = true;
            for (String_t::size_type si = 0; si < i->name.size(); ++si) {
                char this_char = afl::string::charToLower(i->name[si]);
                if (look_at_this_char && this_char >= 'a' && this_char <= 'z'
                    && (used_letters & (1 << (this_char-'a'))) == 0)
                {
                    /* ok */
                    c = this_char;
                    break;
                }

                look_at_this_char = (this_char == ' ');
            }

            /* If we didn't find one, take any free one */
            if (c == 0) {
                for (char ci = 'a'; ci <= 'z'; ++ci) {
                    if ((used_letters & (1 << (ci-'a'))) == 0) {
                        c = ci;
                        break;
                    }
                }
            }

            /* Still didn't find one? This means letters used up. Start anew. */
            if (c == 0) {
                used_letters = USED_LETTERS;
                c = 'a';
            }

            /* Found one, assign it */
            i->key = c;
            used_letters |= 1 << (c - 'a');
        }
    }

    /* Get total number of ships, given a 'Totals' row. */
    afl::base::Optional<int32_t> getTotalShips(const DataTable::Row& totals, const std::vector<int> columns)
    {
        afl::base::Optional<int32_t> result;
        for (size_t i = 0; i < columns.size(); ++i) {
            int32_t v;
            if (totals.get(columns[i]).get(v)) {
                result = result.orElse(0) + v;
            }
        }
        return result;
    }

    /* Table modes */
    enum TableMode {
        Normal,
        DifferenceToPrevious,
        DifferenceToSpecific,
        Percentages,
        RatioOfTotal
    };
    const int NUM_TABLE_MODES = RatioOfTotal+1;

    /* Format a TableMode into a string.
       For DifferenceToSpecific, we need the turn we're comparing against, which is passed as an index into a turn list. */
    String_t toString(TableMode mode, size_t otherIndex, const util::StringList& turnList, afl::string::Translator& tx)
    {
        switch (mode) {
         case Normal:                return tx("Normal scoreboard");
         case DifferenceToPrevious:  return tx("Differences to previous turn");
         case DifferenceToSpecific:  {
            int32_t key;
            String_t s;
            if (turnList.get(otherIndex, key, s)) {
                return afl::string::Format(tx("Differences to turn %d"), key);
            }
            return String_t();
         }
         case Percentages:           return tx("Percentages");
         case RatioOfTotal:          return tx("Ratio of total");
        }
        return String_t();
    }

    bool isDifferenceMode(TableMode mode)
    {
        return mode == DifferenceToPrevious || mode == DifferenceToSpecific;
    }

    /*
     *  ScoreIconBox widget: display a ScoreTabs_t as an IconBox
     */

    class ScoreIconBox : public ui::widgets::IconBox {
     public:
        ScoreIconBox(ui::Root& root, ScoreTabs_t& tabs, afl::string::Translator& tx);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual int getItemWidth(size_t nr) const;
        virtual size_t getNumItems() const;
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);
     private:
        const ScoreTab* getTab(size_t nr) const;
        afl::base::Ref<gfx::Font> getTitleFont() const;
        afl::base::Ref<gfx::Font> getSubtitleFont() const;
        String_t getSubtitle(const ScoreTab& tab) const;

        ui::Root& m_root;
        ScoreTabs_t& m_tabs;
        afl::string::Translator& m_translator;
    };

    /*
     *  ScoreDialog: main entry point
     */

    class ScoreDialog : public gfx::KeyEventConsumer {
     public:
        ScoreDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

        bool init();
        void run();

        // Major entry points
        void openTab(size_t tab);
        bool handleKey(util::Key_t key, int prefix);

        // Event handlers
        void onTableCellClick(size_t column, size_t row);
        void onTableUpdate(std::auto_ptr<DataTable>& tab);
        void onChartUpdate(std::auto_ptr<DataTable>& data);
        void onTableMode();

     private:
        // Links/constants
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;
        ScoreProxy m_proxy;
        client::Downlink m_link;
        util::StringList m_turnList;
        ScoreTabs_t m_tabs;
        ScoreProxy::Info m_overview;
        util::NumberFormatter m_formatter;
        std::vector<int> m_tableShipColumns;
        std::vector<int> m_tableSortKeys;

        // Widgets
        ScoreIconBox m_tabIcons;
        StaticText m_titleText;
        StaticText m_timestampText;
        StaticText m_modeText;
        Group m_tablePage;
        Group m_chartPage;
        std::auto_ptr<SimpleTable> m_pTable;
        std::auto_ptr<ui::widgets::Chart> m_pChart;
        client::widgets::PlayerList m_chartPlayerList;
        Button m_tableModeButton;

        // Dialog status
        int m_highlightedPlayer;        // ...or team
        size_t m_currentTab;
        size_t m_tableTurnIndex;
        size_t m_tableTurnOtherIndex;   // for DifferenceToSpecific mode
        int m_tableSortColumn;          // -1 for none
        TableMode m_tableMode;
        bool m_byTeam;
        bool m_cumulativeMode;

        std::auto_ptr<DataTable> m_tableData;

        // Initialisation
        void generateScoreTabs();

        const ScoreTab* getTab(size_t nr) const
            { return nr < m_tabs.size() ? &m_tabs[nr] : 0; }

        // Data access
        void requestTableData();
        void sortTableData();

        // User actions
        void nextTurn();
        void previousTurn();
        void setTableTurnIndex(size_t index);
        void setTableSortColumn(int column);
        void setTableMode(TableMode mode);
        void changeHighlighedPlayer(bool forward);
        void setHighlightedPlayer(int playerNr);
        void toggleTeams();
        void setByTeam(bool flag);
        void toggleCumulativeMode();
        void setCumulativeMode(bool flag);

        // Rendering
        void renderChartPlayer();
        void renderChartAuxContent(const DataTable& data, DataTable& aux, const ScoreTab& tab);
        void renderMode();
        void renderTableData();
        void renderTableRow(SimpleTable::Range row, size_t startingIndex, TableMode mode, const DataTable::Row& data, const DataTable::Row* compareData) const;

    };
}

/*
 *  ScoreIconBox
 */

ScoreIconBox::ScoreIconBox(ui::Root& root, ScoreTabs_t& tabs, afl::string::Translator& tx)
    : IconBox(root), m_root(root), m_tabs(tabs), m_translator(tx)
{ }

ui::layout::Info
ScoreIconBox::getLayoutInfo() const
{
    // Original is 400px x (title + subtitle + 10)
    gfx::Point titleSize = getTitleFont()->getCellSize();
    gfx::Point subtitleSize = getSubtitleFont()->getCellSize();

    gfx::Point mySize(30 * titleSize.getX(),
                      titleSize.getY() + subtitleSize.getY() + 10);

    return ui::layout::Info(mySize, mySize, ui::layout::Info::GrowHorizontal);
}

int
ScoreIconBox::getItemWidth(size_t nr) const
{
    // ex WScoreIconBox::getItemWidth
    if (const ScoreTab* t = getTab(nr)) {
        return std::max(getTitleFont()->getTextWidth(t->name),
                        getSubtitleFont()->getTextWidth(getSubtitle(*t))) + 10;
    } else {
        return 0;
    }
}

size_t
ScoreIconBox::getNumItems() const
{
    // ex WScoreIconBox::getNumberOfItems
    return m_tabs.size();
}

void
ScoreIconBox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    if (state == Normal) {
        drawBackground(ctx, area);
        ctx.setColor(util::SkinColor::Static);
    } else {
        drawBackground(ctx, gfx::Rectangle(area.getLeftX(),    area.getTopY(),      1, 1));
        drawBackground(ctx, gfx::Rectangle(area.getRightX()-1, area.getTopY(),      1, 1));
        drawBackground(ctx, gfx::Rectangle(area.getLeftX(),    area.getBottomY()-1, 1, 1));
        drawBackground(ctx, gfx::Rectangle(area.getRightX()-1, area.getBottomY()-1, 1, 1));

        ctx.setRawColor(m_root.colorScheme().getColor(state == Hover ? ui::Color_Grayscale+6 : ui::Color_Black));
        drawBar(ctx, gfx::Rectangle(area.getLeftX(),   area.getTopY()+1,    area.getWidth(),   area.getHeight()-2));
        drawBar(ctx, gfx::Rectangle(area.getLeftX()+1, area.getTopY(),      area.getWidth()-2, 1));
        drawBar(ctx, gfx::Rectangle(area.getLeftX()+1, area.getBottomY()-1, area.getWidth()-2, 1));

        ctx.setRawColor(m_root.colorScheme().getColor(state == Hover ? ui::Color_Black : ui::Color_White));
    }
    if (const ScoreTab* tab = getTab(item)) {
        ctx.useFont(*getTitleFont());
        area.consumeX(5);
        area.consumeY(5);
        outTextF(ctx, area.splitY(ctx.getFont()->getLineHeight()), tab->name);
        ctx.useFont(*getSubtitleFont());
        outTextF(ctx, area, getSubtitle(*tab));
    }
}

const ScoreTab*
ScoreIconBox::getTab(size_t nr) const
{
    return nr < m_tabs.size() ? &m_tabs[nr] : 0;
}

afl::base::Ref<gfx::Font>
ScoreIconBox::getTitleFont() const
{
    return m_root.provider().getFont("+");
}

afl::base::Ref<gfx::Font>
ScoreIconBox::getSubtitleFont() const
{
    return m_root.provider().getFont("");
}

String_t
ScoreIconBox::getSubtitle(const ScoreTab& tab) const
{
    // ex WScoreIconBox::getSubTitle
    String_t subtitle = tab.kind == Table ? m_translator("Table") : m_translator("Graph");
    if (tab.key != 0) {
        subtitle += " [";
        subtitle += char(tab.key);
        subtitle += "]";
    }
    return subtitle;
}


/*
 *  ScoreDialog
 */

ScoreDialog::ScoreDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
    : m_root(root),
      m_gameSender(gameSender),
      m_translator(tx),
      m_proxy(root.engine().dispatcher(), gameSender),
      m_link(root, tx),
      m_turnList(),
      m_tabs(),
      m_overview(),
      m_formatter(false, false),
      m_tabIcons(root, m_tabs, tx),
      m_titleText(String_t(), util::SkinColor::Static, "+", root.provider(), gfx::CenterAlign),
      m_timestampText(String_t(), util::SkinColor::Static, "", root.provider()),
      m_modeText(String_t(), util::SkinColor::Static, "", root.provider()),
      m_tablePage(ui::layout::VBox::instance5),
      m_chartPage(ui::layout::VBox::instance5),
      m_pTable(),
      m_pChart(),
      m_chartPlayerList(root, client::widgets::PlayerList::FlowLayout, client::widgets::PlayerList::ShowNames, client::widgets::PlayerList::PlayerColors, CHART_WIDTH, game::PlayerSet_t()),
      m_tableModeButton("D", 'd', root),
      m_highlightedPlayer(0),
      m_currentTab(static_cast<size_t>(-1)),
      m_tableTurnIndex(0),
      m_tableTurnOtherIndex(0),
      m_tableSortColumn(0),
      m_tableMode(Normal),
      m_byTeam(false),
      m_cumulativeMode(false)
{ }

bool
ScoreDialog::init()
{
    // Retrieve list of turns
    m_proxy.getTurns(m_link, m_turnList);
    if (m_turnList.empty()) {
        return false;
    }
    m_tableTurnIndex = m_turnList.size()-1;

    // Build list of tabs
    generateScoreTabs();
    if (m_tabs.empty()) {
        return false;
    }
    m_tabIcons.handleStructureChange(0);

    // Retrieve score info
    m_proxy.getOverviewInformation(m_link, m_overview);
    if (m_overview.players.empty()) {
        return false;
    }
    m_highlightedPlayer = m_overview.viewpointPlayer;

    // Go to viewpoint turn
    size_t viewpointTurnIndex;
    if (m_overview.viewpointTurn != 0 && m_turnList.find(m_overview.viewpointTurn).get(viewpointTurnIndex)) {
        m_tableTurnIndex = viewpointTurnIndex;
    }

    // Configuration
    m_formatter = game::proxy::ConfigurationProxy(m_gameSender).getNumberFormatter(m_link);

    // Initialize events
    m_proxy.sig_tableUpdate.add(this, &ScoreDialog::onTableUpdate);
    m_proxy.sig_chartUpdate.add(this, &ScoreDialog::onChartUpdate);
    m_tableModeButton.sig_fire.add(this, &ScoreDialog::onTableMode);
    m_chartPlayerList.sig_playerClick.add(this, &ScoreDialog::setHighlightedPlayer);

    // Initialize widgets
    const int em = m_root.provider().getFont("")->getEmWidth();
    m_tabIcons.sig_change.add(this, &ScoreDialog::openTab);
    m_timestampText.setForcedWidth(15*em);
    m_modeText.setForcedWidth(20*em);
    m_chartPlayerList.setMinimumLines(2);

    // Table
    // (ex WScoreTable::WScoreTable, sort-of)
    // - COLUMNS_PER_TABLE+1 columns (label + columns)
    // - number of players + header + totals + ships rows
    const size_t numPlayers = m_overview.players.size();
    const size_t numLines = numPlayers + 3;
    m_pTable.reset(new SimpleTable(m_root, COLUMNS_PER_TABLE+1, numLines));
    m_pTable->row(0).setFont("+");
    m_pTable->setColumnWidth(0, 12*em);
    for (size_t i = 0; i < COLUMNS_PER_TABLE; ++i) {
        m_pTable->setColumnWidth(i+1, 7*em);
        m_pTable->cell(i+1, 0).setTextAlign(gfx::CenterAlign, gfx::BottomAlign);
    }
    for (size_t i = 1; i < numLines-1; ++i) {
        m_pTable->setRowPadding(i, em/4);
        m_pTable->row(i).subrange(1, COLUMNS_PER_TABLE).setTextAlign(gfx::RightAlign, gfx::MiddleAlign);
    }
    m_pTable->all().setColor(COLOR_NORMAL);
    m_pTable->sig_cellClick.add(this, &ScoreDialog::onTableCellClick);

    // Chart
    m_pChart.reset(new ui::widgets::Chart(m_root, gfx::Point(CHART_WIDTH, CHART_HEIGHT), m_formatter));
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        m_pChart->style(i)
            .setColor(client::widgets::PlayerList::getPlayerColor(i))
            .setLineThickness(1)
            .setZOrder(NORMAL_Z);
    }
    m_pChart->style(DECAY_LINE_EVEN_ID)
        .setColor(COLOR_GRID)
        .setPointIcon(ui::widgets::Chart::NoIcon)
        .setLinePattern(0xAA)
        .setZOrder(GRID_Z);
    m_pChart->style(DECAY_LINE_ODD_ID)
        .setColor(COLOR_GRID)
        .setPointIcon(ui::widgets::Chart::NoIcon)
        .setLinePattern(0x55)
        .setZOrder(GRID_Z);
    m_pChart->style(GRID_LINE_ID)
        .setColor(COLOR_GRID)
        .setPointIcon(ui::widgets::Chart::NoIcon)
        .setLinePattern(0xAA)
        .setLineMode(ui::widgets::Chart::Line_ExtendRight | ui::widgets::Chart::Line_ExtendLeft | ui::widgets::Chart::Line_LabelLeft)
        .setZOrder(GRID_Z);
    m_pChart->style(WINLIMIT_LINE_ID)
        .setColor(COLOR_WINLIMIT)
        .setPointIcon(ui::widgets::Chart::NoIcon)
        .setLinePattern(0x33)
        .setLineMode(ui::widgets::Chart::Line_ExtendRight | ui::widgets::Chart::Line_ExtendLeft | ui::widgets::Chart::Line_LabelLeft)
        .setZOrder(GRID_Z);

    return true;
}

void
ScoreDialog::run()
{
    // ex WScoreWindow::init (part)
    // Window [VBox]
    //   StaticText (title)
    //   CardGroup
    //     Group [VBox]
    //       SimpleTable
    //       Group [HBox]
    //         Button "+"
    //         Button "-"
    //         StaticText (timestamp)
    //         Spacer
    //         Button "D"
    //         StaticText (mode)    // or OptionGrid?
    //     Group [VBox]
    //       Chart
    //       PlayerList
    //   ScoreIconBox
    //   Group [HBox]
    //     Button "Help"
    //     Spacer
    //     Button "Close"
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Scores"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    // Title
    win.add(m_titleText);

    // Cards
    ui::CardGroup& cards = del.addNew(new ui::CardGroup());

    // - table
    assert(m_pTable.get() != 0);
    m_tablePage.add(*m_pTable);

    Group& tableControls = del.addNew(new Group(ui::layout::HBox::instance5));
    Button& btnNextTurn = del.addNew(new Button("+", '+', m_root));
    Button& btnPreviousTurn = del.addNew(new Button("-", '-', m_root));
    btnNextTurn.setState(ui::Widget::DisabledState, m_turnList.size() <= 1);
    btnPreviousTurn.setState(ui::Widget::DisabledState, m_turnList.size() <= 1);
    tableControls.add(btnNextTurn);
    tableControls.add(btnPreviousTurn);
    tableControls.add(m_timestampText);
    tableControls.add(del.addNew(new ui::Spacer()));
    tableControls.add(m_tableModeButton);
    tableControls.add(m_modeText);
    m_tablePage.add(tableControls);
    m_tablePage.add(del.addNew(new ui::Spacer()));
    cards.add(m_tablePage);

    // - chart
    assert(m_pChart.get() != 0);
    m_chartPage.add(*m_pChart);
    m_chartPage.add(m_chartPlayerList);
    cards.add(m_chartPage);
    win.add(cards);

    // Icons
    win.add(m_tabIcons);

    // Buttons
    ui::EventLoop loop(m_root);
    Group& buttonGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    Button& btnHelp = del.addNew(new Button(m_translator("Help"), 'h', m_root));
    Button& btnClose = del.addNew(new Button(m_translator("Close"), util::Key_Escape, m_root));
    buttonGroup.add(btnHelp);
    buttonGroup.add(del.addNew(new ui::Spacer()));
    buttonGroup.add(btnClose);
    win.add(buttonGroup);

    client::widgets::HelpWidget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:scores"));
    win.add(help);
    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
    win.add(del.addNew(new ui::widgets::KeyForwarder(*this)));

    btnNextTurn.sig_fire.add(this, &ScoreDialog::nextTurn);
    btnPreviousTurn.sig_fire.add(this, &ScoreDialog::previousTurn);
    btnHelp.dispatchKeyTo(help);
    btnClose.sig_fire.addNewClosure(loop.makeStop(0));

    openTab(0);

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    loop.run();
}

void
ScoreDialog::openTab(size_t tab)
{
    // ex WScoreWindow::openPage, WScoreWindow::changeToPage
    const ScoreTab* oldTab = getTab(m_currentTab);
    const ScoreTab* newTab = getTab(tab);
    if (m_currentTab != tab && newTab != 0) {
        bool isStyleChange = (oldTab != 0 && oldTab->kind != newTab->kind);
        m_titleText.setText(newTab->name);
        switch (newTab->kind) {
         case Table:
            // If we're switching to a table clear previous content to avoid old data flashing.
            // If we're staying on a table, keep it.
            if (isStyleChange) {
                m_pTable->all().setText(String_t());
            }
            requestTableData();
            renderMode();
            m_tablePage.requestFocus();
            m_root.postMouseEvent();
            break;
         case Chart:
            // Clear on change
            if (isStyleChange) {
                m_pChart->setContent(std::auto_ptr<DataTable>());
                m_pChart->setAuxContent(std::auto_ptr<DataTable>());
            }
            m_proxy.setChartIndex(newTab->index);
            renderChartPlayer();
            m_chartPage.requestFocus();
            m_root.postMouseEvent();
            break;
        }

        // Remember
        m_currentTab = tab;
        m_tabIcons.setCurrentItem(tab);
    }
}

bool
ScoreDialog::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex WScoreIconBox::handleEvent, WScoreTable::handleEvent
    const ScoreTab*const tab = getTab(m_currentTab);
    const bool isTable = (tab != 0 && tab->kind == Table);
    const bool isChart = (tab != 0 && tab->kind == Chart);

    // Hard-coded keys
    switch (key) {
     case util::Key_Tab:
     case util::Key_Right:
        // Tab: cycle forward
        m_tabIcons.requestActive();
        if (m_currentTab < m_tabs.size()-1) {
            openTab(m_currentTab+1);
        } else {
            openTab(0);
        }
        return true;

     case util::Key_Tab + util::KeyMod_Shift:
     case util::Key_Left:
        // Shift-Tab: cycle backward
        m_tabIcons.requestActive();
        if (m_currentTab > 0) {
            openTab(m_currentTab-1);
        } else {
            openTab(m_tabs.size()-1);
        }
        return true;

     case util::Key_Up:
     case util::Key_PgUp:
     case util::Key_WheelUp:
        if (isTable) {
            previousTurn();
            return true;
        }
        break;

     case util::Key_Down:
     case util::Key_PgDn:
     case util::Key_WheelDown:
        if (isTable) {
            nextTurn();
            return true;
        }
        break;

     case util::Key_PgDn + util::KeyMod_Ctrl:
     case util::Key_End + util::KeyMod_Ctrl:
        if (isTable) {
            setTableTurnIndex(m_turnList.size()-1);
            return true;
        }
        break;

     case util::Key_PgUp + util::KeyMod_Ctrl:
     case util::Key_Home + util::KeyMod_Ctrl:
        if (isTable) {
            setTableTurnIndex(0);
            return true;
        }
        break;

     case util::Key_Up + util::KeyMod_Ctrl:
     case util::Key_Left + util::KeyMod_Ctrl:
        changeHighlighedPlayer(false);
        return true;

     case util::Key_Down + util::KeyMod_Ctrl:
     case util::Key_Right + util::KeyMod_Ctrl:
        changeHighlighedPlayer(true);
        return true;

     case util::KeyMod_Ctrl + 'u':
        if (isTable) {
            setTableSortColumn(-1);
            return true;
        }
        break;

     case 'y':
        toggleTeams();
        return true;

     case 'x':
        if (isChart) {
            toggleCumulativeMode();
            return true;
        }
        break;

     default:
        break;
    }

    // Hot-keys to select individual pages
    Matcher<size_t> matchTab;
    for (size_t i = 0, n = m_tabs.size(); i < n; ++i) {
        if (m_tabs[i].key == key && matchTab(i, i == m_currentTab)) {
            break;
        }
    }
    if (matchTab.isValid()) {
        m_tabIcons.requestActive();
        openTab(matchTab.getIndex());
        return true;
    }

    // Hot-keys to sort a table
    if (isTable) {
        Matcher<int> matchSort;
        for (int i = 0; i < static_cast<int>(m_tableSortKeys.size()); ++i) {
            if (m_tableSortKeys[i] + util::KeyMod_Ctrl == key && matchSort(i, i == m_tableSortColumn)) {
                break;
            }
        }
        if (matchSort.isValid()) {
            setTableSortColumn(matchSort.getIndex());
            return true;
        }
    }
    return false;
}

void
ScoreDialog::onTableCellClick(size_t column, size_t row)
{
    // ex WScoreTable::handleEvent (part)
    if (m_tableData.get() == 0) {
        // Fail-safe
    } else if (row == 0) {
        // Click on header
        if (column == 0) {
            setTableSortColumn(-1);
        } else {
            const ScoreTab*const tab = getTab(m_currentTab);
            if (tab != 0 && tab->kind == Table) {
                int effectiveColumn = static_cast<int>((column - 1) + tab->index);
                if (!m_tableData->getColumnName(effectiveColumn).empty()) {
                    setTableSortColumn(effectiveColumn);
                }
            }
        }
    } else if (column == 0) {
        // Click on player
        if (const DataTable::Row* r = m_tableData->getRow(row-1)) {
            if (int playerNr = r->getId()) {
                setHighlightedPlayer(playerNr);
            }
        }
    } else {
        // Nothing.
    }
}

void
ScoreDialog::onTableUpdate(std::auto_ptr<DataTable>& data)
{
    if (data.get() != 0) {
        m_tableData = data;
        sortTableData();
        renderTableData();
    }
}

void
ScoreDialog::onChartUpdate(std::auto_ptr<DataTable>& data)
{
    const ScoreTab* tab = getTab(m_currentTab);
    if (data.get() != 0 && m_pChart.get() != 0 && tab != 0 && tab->kind == Chart) {
        // Player list content
        game::PlayerSet_t players;
        for (size_t i = 0, n = data->getNumRows(); i < n; ++i) {
            const DataTable::Row* r = data->getRow(i);
            players += r->getId();
            m_chartPlayerList.setName(r->getId(), r->getName());
        }
        m_chartPlayerList.setVisiblePlayers(players);

        // Aux lines
        std::auto_ptr<DataTable> aux(new DataTable());
        renderChartAuxContent(*data, *aux, *tab);

        // Chart content
        m_pChart->setContent(data);
        m_pChart->setAuxContent(aux);

        if (m_cumulativeMode) {
            m_pChart->addNewIcon(CHART_MODE_ICON_ID, gfx::Point(5, 0), new ui::icons::SkinText(m_translator("Cumulative chart"), m_root));
        } else {
            m_pChart->removeIcon(CHART_MODE_ICON_ID);
        }
    }
}

void
ScoreDialog::onTableMode()
{
    // ex WScoreWindow::handleCommand
    ui::widgets::StringListbox menu(m_root.provider(), m_root.colorScheme());

    // Table modes
    for (int i = 0; i < NUM_TABLE_MODES; ++i) {
        if (!isDifferenceMode(TableMode(i)) || m_turnList.size() > 1) {
            menu.addItem(i, toString(TableMode(i), m_tableTurnIndex, m_turnList, m_translator));
        }
    }

    // Aggregation
    const int BY_PLAYER = 1000, BY_TEAM = 1001;
    if (m_overview.hasTeams) {
        menu.addItem(BY_PLAYER, m_translator("Scores by player"));
        menu.addItem(BY_TEAM,   m_translator("Scores by team"));
    }

    // Operate
    ui::EventLoop loop(m_root);
    if (!ui::widgets::MenuFrame(ui::layout::HBox::instance5, m_root, loop).doMenu(menu, m_tableModeButton.getExtent().getBottomLeft())) {
        return;
    }

    int32_t result;
    if (menu.getCurrentKey(result)) {
        if (result >= 0 && result < NUM_TABLE_MODES) {
            setTableMode(TableMode(result));
        } else if (result == BY_TEAM) {
            setByTeam(true);
        } else if (result == BY_PLAYER) {
            setByTeam(false);
        }
    }
}

/* Initialisation */

void
ScoreDialog::generateScoreTabs()
{
    // ex client/scr-score.cc:generateScoreTabs
    // Overviews
    ScoreProxy::Variants_t tableVariants;
    m_proxy.getTableVariants(m_link, tableVariants);
    for (size_t i = 0; COLUMNS_PER_TABLE*i < tableVariants.size(); ++i) {
        if (i == 0) {
            m_tabs.push_back(ScoreTab(Table, m_translator("Overview"), COLUMNS_PER_TABLE*i, 0, 0));
        } else {
            m_tabs.push_back(ScoreTab(Table, afl::string::Format(m_translator("Overview %d"), i+1), COLUMNS_PER_TABLE*i, 0, 0));
        }
    }

    // Tables
    const size_t numOverviewTabs = m_tabs.size();
    ScoreProxy::Variants_t chartVariants;
    m_proxy.getChartVariants(m_link, chartVariants);
    for (size_t i = 0; i < chartVariants.size(); ++i) {
        m_tabs.push_back(ScoreTab(Chart, chartVariants[i].name, i, chartVariants[i].decay, chartVariants[i].winLimit));
    }
    assignKeys(m_tabs);

    // Process table variants and assign ship columns/sort keys
    for (size_t i = 0; i < tableVariants.size(); ++i) {
        if (tableVariants[i].scoreId == game::score::ScoreId_Capital || tableVariants[i].scoreId == game::score::ScoreId_Freighters) {
            m_tableShipColumns.push_back(static_cast<int>(i));
        }

        int thisKey = 0;
        for (size_t j = 0; j < chartVariants.size(); ++j) {
            if (chartVariants[j].score == tableVariants[i].score) {
                thisKey = m_tabs[j + numOverviewTabs].key;
                break;
            }
        }
        m_tableSortKeys.push_back(thisKey);
    }

    // If we have no turns, drop the 'Table' tabs.
    // We still need to generate them to reliably assign keys for sorting.
    if (m_turnList.size() <= 1) {
        m_tabs.resize(numOverviewTabs);
    }
}

/* Data access */

void
ScoreDialog::requestTableData()
{
    // Request data from game
    switch (m_tableMode) {
     case DifferenceToPrevious:
        if (m_tableTurnIndex > 0) {
            m_proxy.setTableTurnDifferenceIndexes(m_tableTurnIndex, m_tableTurnIndex-1);
        } else {
            m_proxy.setTableTurnIndex(m_tableTurnIndex);
        }
        break;
     case DifferenceToSpecific:
        m_proxy.setTableTurnDifferenceIndexes(m_tableTurnIndex, m_tableTurnOtherIndex);
        break;
     case Normal:
     case Percentages:
     case RatioOfTotal:
        m_proxy.setTableTurnIndex(m_tableTurnIndex);
        break;
    }
}

void
ScoreDialog::sortTableData()
{
    // ex WScoreTable::updateSortOrder, sort-of
    if (m_tableData.get() != 0) {
        m_tableData->sortRows(CompareRowsByColumn(m_tableSortColumn));
    }
}

/* User actions */

void
ScoreDialog::nextTurn()
{
    // ex WScoreTable::nextTurn
    if (m_tableTurnIndex < m_turnList.size()-1) {
        setTableTurnIndex(m_tableTurnIndex+1);
    }
}

void
ScoreDialog::previousTurn()
{
    // ex WScoreTable::previousTurn
    if (m_tableTurnIndex > 0) {
        setTableTurnIndex(m_tableTurnIndex-1);
    }
}

void
ScoreDialog::setTableTurnIndex(size_t index)
{
    // ex WScoreTable::setCurrentRecord
    m_tableTurnIndex = index;
    requestTableData();
}

void
ScoreDialog::setTableSortColumn(int column)
{
    // ex WScoreTable::setSortOrder
    m_tableSortColumn = column;
    sortTableData();
    renderTableData();
}

void
ScoreDialog::setTableMode(TableMode mode)
{
    m_tableMode = mode;
    m_tableTurnOtherIndex = m_tableTurnIndex;
    requestTableData();
    renderMode();
}

void
ScoreDialog::changeHighlighedPlayer(bool forward)
{
    const ScoreTab* tab = getTab(m_currentTab);
    if (tab != 0 && tab->kind == Table && m_tableData.get() != 0) {
        // ex WScoreTable::nextPlayer, WScoreTable::previousPlayer
        int pl = findNextPlayerInTable(*m_tableData, m_highlightedPlayer, forward, m_tableMode != Percentages);
        setHighlightedPlayer(pl);
    }
    if (tab != 0 && tab->kind == Chart && m_pChart.get() != 0) {
        // ex WScoreChart::handleEvent (sort-of)
        if (const DataTable* tab = m_pChart->getContent()) {
            int pl = findNextPlayerInTable(*tab, m_highlightedPlayer, forward, true);
            setHighlightedPlayer(pl);
        }
    }
}

void
ScoreDialog::setHighlightedPlayer(int playerNr)
{
    // ex WScoreTable::setCurrentPlayer
    if (playerNr != m_highlightedPlayer) {
        m_highlightedPlayer = playerNr;

        const ScoreTab* tab = getTab(m_currentTab);
        if (tab != 0 && tab->kind == Table) {
            renderTableData();
        }
        if (tab != 0 && tab->kind == Chart) {
            renderChartPlayer();
        }
    }
}

void
ScoreDialog::toggleTeams()
{
    if (m_overview.hasTeams) {
        setByTeam(!m_byTeam);
    }
}

void
ScoreDialog::setByTeam(bool flag)
{
    if (m_byTeam != flag) {
        m_byTeam = flag;
        m_proxy.setByTeam(flag);
        renderMode();
    }
}

void
ScoreDialog::toggleCumulativeMode()
{
    setCumulativeMode(!m_cumulativeMode);
}

void
ScoreDialog::setCumulativeMode(bool flag)
{
    if (m_cumulativeMode != flag) {
        m_cumulativeMode = flag;
        m_proxy.setCumulativeMode(flag);
    }
}

/* Rendering */

void
ScoreDialog::renderChartPlayer()
{
    if (m_pChart.get() != 0) {
        for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
            int isMe = (i == m_highlightedPlayer);
            m_pChart->style(i)
                .setLineThickness(isMe ? 3 : 1)
                .setZOrder(isMe ? CURRENT_PLAYER_Z : NORMAL_Z);
        }
    }
}

void
ScoreDialog::renderChartAuxContent(const DataTable& data, DataTable& aux, const ScoreTab& tab)
{
    // ex WScoreChart::drawContent, WScoreChart::recomputeGrid
    const int32_t maxScore = data.getValueRange().max();
    const int maxColumn = data.getNumColumns();

    // Win Limit only when displaying a regular per-player chart
    bool allowWinLimit = !m_byTeam && !m_cumulativeMode;

    // Render aux/grid lines
    if (tab.decay > 0) {
        // There is decay, so draw decaying ("log scale") lines.
        // Let the first line start above the maximum, so that it just comes in range with the first step.
        // Since we'll not be labelling the lines, numbers don't have to be nice.
        int32_t aux_limit = maxScore * 100 / (100-tab.decay);
        int32_t aux_scale = aux_limit / 5;
        if (aux_scale < 10) {
            aux_scale = 10;
        }

        // Only draw when we have a reasonable number of lines.
        // This effectively means we don't draw if there are too few lines.
        if (aux_scale * 3 <= aux_limit) {
            // Each line is drawn as a Row.
            // Up to 20 active lines.
            const size_t MAX_LINES = 20;
            DataTable::Row* rows[MAX_LINES];
            size_t nlines = 0;

            // Lines use alternating IDs and therefore alternating styles with alternating line pattern,
            // to avoid that lines build blocks when running together.
            int counter = 0;
            for (int32_t v = aux_scale; v <= aux_limit && nlines < MAX_LINES; v += aux_scale) {
                rows[nlines] = &aux.addRow((counter & 1) != 0 ? DECAY_LINE_ODD_ID : DECAY_LINE_EVEN_ID);
                rows[nlines]->set(0, v);
                ++nlines;
                ++counter;
            }

            for (int i = 1; i < maxColumn; ++i) {
                for (size_t t = 0; t < nlines; ++t) {
                    int32_t vnew = (rows[t]->get(i-1).orElse(0) * (100 - tab.decay) + 50) / 100;
                    rows[t]->set(i, vnew);
                }

                // When our highest line drops below the limit, add a new one.
                while (nlines > 0 && rows[nlines-1]->get(i).orElse(0) < aux_limit) {
                    if (nlines >= MAX_LINES) {
                        // No room to add it, lose one near the y axis.
                        for (size_t i = 1; i < nlines; ++i) {
                            rows[i-1] = rows[i];
                        }
                        --nlines;
                    }
                    int32_t v = rows[nlines-1]->get(i).orElse(0) + aux_scale;
                    rows[nlines] = &aux.addRow((counter & 1) != 0 ? DECAY_LINE_ODD_ID : DECAY_LINE_EVEN_ID);
                    rows[nlines]->set(i, v);
                    ++nlines;
                }
            }
        }
    } else {
        // No decay, so we'll be making regular lines.
        // We wish the first line to be a little below the maximum, and we want no more than 5 grid lines.
        // We try all distances that have nice numbers, i.e. 1,2,5,10,20,50,100,etc.
        static const int NICE_NUMBERS[] = { 1, 2, 5 };
        const int32_t limit = (maxScore * 23)/25;
        size_t nice_index = 0;
        int32_t nice_scale = 1;
        int32_t aux_scale;

        while ((aux_scale = NICE_NUMBERS[nice_index] * nice_scale) * 5 < limit) {
            ++nice_index;
            if (nice_index >= countof(NICE_NUMBERS)) {
                nice_index = 0;
                nice_scale *= 10;
            }
        }

        // Draw the lines
        for (int32_t value = aux_scale; value <= limit; value += aux_scale) {
            if (!allowWinLimit || value != tab.winLimit) {
                aux.addRow(GRID_LINE_ID).set(0, value);
            }
        }
    }

    // Render winning condition
    if (allowWinLimit && tab.winLimit > 0 && tab.winLimit <= maxScore) {
        aux.addRow(WINLIMIT_LINE_ID).set(0, tab.winLimit);
    }
}

void
ScoreDialog::renderMode()
{
    // Update mode
    String_t modeString = toString(m_tableMode, m_tableTurnOtherIndex, m_turnList, m_translator);
    if (m_overview.hasTeams) {
        if (m_byTeam) {
            util::addListItem(modeString, ", ", m_translator("by team"));
        } else {
            util::addListItem(modeString, ", ", m_translator("by player"));
        }
    }
    m_modeText.setText(modeString);
}

void
ScoreDialog::renderTableData()
{
    // ex WScoreTable::drawContent, CScoreTable.DrawContent
    // Must be on a valid page
    const ScoreTab* tab = getTab(m_currentTab);
    if (tab == 0 || m_tableData.get() == 0) {
        return;
    }

    // First row
    // - turn number
    int32_t turn;
    String_t timeStamp;
    if (m_turnList.get(m_tableTurnIndex, turn, timeStamp)) {
        m_pTable->cell(0, 0).setText(afl::string::Format(m_translator("Turn %d"), turn));
    }

    // - timestamp in separate widget. We get timestamps as 18-character strings.
    //   Make it more human-friendly. For simplicity, instead of trying to make game::Timestamp do it, just insert the ", ".
    timeStamp.insert(10, ", ");
    m_timestampText.setText(timeStamp);

    // - headers
    for (size_t i = 0; i < COLUMNS_PER_TABLE; ++i) {
        int index = static_cast<int>(i + tab->index);
        String_t label = m_tableData->getColumnName(index);
        if (index == m_tableSortColumn) {
            label += UTF_DOWN_ARROW;
        }
        m_pTable->cell(i+1, 0).setText(label);
    }

    // Compute totals
    DataTable totals;
    DataTable::Row& totalsRow = totals.addRow(-444);
    totalsRow.setName(m_translator("\xE2\x96\xB6\xE2\x96\xB6 Totals"));
    for (size_t i = 0, n = m_tableData->getNumRows(); i < n; ++i) {
        totalsRow.add(*m_tableData->getRow(i));
    }

    // Determine comparison/mode
    // - in Percentages mode, compare to highlighted player if any; in RatioOfTotal mode, compare to totalsRow.
    //   For all other modes, don't care.
    const DataTable::Row* compareRow = (m_tableMode == Percentages ? m_tableData->findRowById(m_highlightedPlayer) : &totalsRow);

    // - in DifferenceToPrevious mode, when looking at the first turn, we actually requested only the normal data, no difference.
    //   Therefore, only render normal data.
    const TableMode effMode = (m_tableMode == DifferenceToPrevious && m_tableTurnIndex == 0 ? Normal : m_tableMode);

    // - totals are differences-of-totals in diff modes.
    const bool isDiffMode = isDifferenceMode(effMode);

    // Render regular rows
    const size_t numPlayers = m_overview.players.size();
    for (size_t i = 0; i < numPlayers; ++i) {
        if (const DataTable::Row* r = m_tableData->getRow(i)) {
            renderTableRow(m_pTable->row(i+1), tab->index, effMode, *r, compareRow);
        } else {
            m_pTable->row(i+1).setText(String_t());
        }
    }

    // Render totals row
    renderTableRow(m_pTable->row(numPlayers+1), tab->index, isDiffMode ? DifferenceToSpecific : Normal, totalsRow, 0);

    // Total Ships
    afl::base::Optional<int32_t> totalShips = getTotalShips(totalsRow, m_tableShipColumns);
    m_pTable->cell(0, numPlayers+2).setText(m_translator("Total ships in game:"));
    m_pTable->cell(1, numPlayers+2).setText(totalShips.get() != 0
                                            ? (isDiffMode
                                               ? m_formatter.formatDifference(*totalShips.get())
                                               : m_formatter.formatNumber(*totalShips.get()))
                                            : String_t())
        .setTextAlign(gfx::RightAlign, gfx::TopAlign);
}

void
ScoreDialog::renderTableRow(SimpleTable::Range row, size_t startingIndex, TableMode mode, const DataTable::Row& data, const DataTable::Row* compareData) const
{
    bool hasAnyValue = false;
    row.cell(0).setText(data.getName());
    for (size_t c = 0; c < COLUMNS_PER_TABLE; ++c) {
        int index = static_cast<int>(c + startingIndex);
        if (index < data.getNumColumns()) {
            int32_t value;
            int32_t compareValue;
            if (data.get(index).get(value)) {
                String_t text;
                uint8_t color = m_highlightedPlayer == data.getId() ? COLOR_SELECTED : COLOR_NORMAL;
                switch (mode) {
                 case Normal:
                    text = m_formatter.formatNumber(value);
                    if (value == 0) {
                        color = COLOR_FADED;
                    } else {
                        hasAnyValue = true;
                    }
                    break;

                 case DifferenceToSpecific:
                 case DifferenceToPrevious:
                    text = m_formatter.formatDifference(value);
                    if (value > 0) {
                        color = COLOR_GOOD;
                    }
                    if (value < 0) {
                        color = COLOR_BAD;
                    }
                    hasAnyValue = true;
                    break;

                 case Percentages:
                 case RatioOfTotal:
                    if (compareData != 0 && compareData->get(index).get(compareValue) && compareValue > 0) {
                        const int32_t percent = 100*value / compareValue;
                        if (percent > 9999) {
                            text = "++++";
                        } else {
                            text = afl::string::Format("%d%%", percent);
                        }
                    } else {
                        text = m_formatter.formatNumber(value);
                    }
                    if (value == 0) {
                        color = COLOR_FADED;
                    } else {
                        hasAnyValue = true;
                    }
                    break;
                }
                row.cell(c+1).setText(text)
                    .setColor(color);
            } else {
                row.cell(c+1).setText("-")
                    .setColor(COLOR_FADED);
            }
        } else {
            row.cell(c+1).setText(String_t());
        }
    }
    row.cell(0).setColor(m_highlightedPlayer == data.getId()
                 ? COLOR_SELECTED
                 : hasAnyValue
                 ? COLOR_NORMAL
                 : COLOR_FADED);
}


/*
 *  Main Entry Point
 */

void
client::dialogs::showScores(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
{
    // ex client/scr-score.cc:doScoreScreen
    ScoreDialog dlg(root, gameSender, tx);
    if (!dlg.init()) {
        return;
    }
    dlg.run();
}
