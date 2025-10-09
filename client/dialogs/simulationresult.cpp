/**
  *  \file client/dialogs/simulationresult.cpp
  *  \brief Simulation Result Dialog
  */

#include "client/dialogs/simulationresult.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/log.hpp"
#include "client/dialogs/vcrplayer.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/simulationlist.hpp"
#include "client/widgets/simulationresultlist.hpp"
#include "client/widgets/stoppablebusyindicator.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/fleetcostproxy.hpp"
#include "game/proxy/playerproxy.hpp"
#include "ui/cardgroup.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/cardtabbar.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/rich/linkattribute.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/rich/text.hpp"
#include "util/stringparser.hpp"
#include "util/translation.hpp"

using afl::string::Format;
using client::dialogs::SimulationResultStatus;
using game::proxy::SimulationRunProxy;
using game::proxy::SimulationSetupProxy;
using ui::widgets::Button;
using ui::widgets::CardTabBar;
using util::rich::LinkAttribute;
using util::rich::StyleAttribute;
using util::rich::Text;

namespace {
    /*
     *  Ad-hoc table metrics
     */

    struct TableMetrics {
        int labelWidth;
        int totalX;
        int minX;
        int aveX;
        int maxX;
    };

    const char*const LABELS[] = {
        N_("Fought"),
        N_("Survived"),
        N_("Captured"),
    };

    const char*const HEADERS[] = {
        N_("min."),
        N_("average"),
        N_("max."),
    };

    TableMetrics computeUnitResultTableMetrics(ui::Root& root, afl::string::Translator& tx)
    {
        afl::base::Ref<gfx::Font> font = root.provider().getFont(gfx::FontRequest());
        const int em = font->getEmWidth();

        TableMetrics result = {0,0,0,0,0};

        // Label widths
        for (size_t i = 0; i <= game::sim::ResultList::UnitInfo::MAX_TYPE; ++i) {
            result.labelWidth = std::max(result.labelWidth, font->getTextWidth(game::sim::toString(static_cast<game::sim::ResultList::UnitInfo::Type>(i), tx)));
        }
        for (size_t i = 0; i < countof(LABELS); ++i) {
            result.labelWidth = std::max(result.labelWidth, font->getTextWidth(tx(LABELS[i])));
        }
        result.labelWidth += 10;

        // Total: PCC2 just uses 6em
        int totalWidth = 6*em;
        result.totalX = result.labelWidth + totalWidth;

        // Min/Max: PCC2 uses max(3em, label)
        int minWidth = std::max(3*em, font->getTextWidth(tx(HEADERS[0]))) + 5;
        int maxWidth = std::max(3*em, font->getTextWidth(tx(HEADERS[2]))) + 5;

        // Average: PCC2 uses max(4em, label)
        int aveWidth = std::max(4*em, font->getTextWidth(tx(HEADERS[1]))) + 5;
        result.minX = result.labelWidth + minWidth;
        result.aveX = result.minX       + aveWidth;
        result.maxX = result.aveX       + maxWidth;

        return result;
    }

    void renderCount(ui::rich::Document& doc,
                     int32_t value,
                     const SimulationRunProxy::UnitInfo_t& info,
                     const TableMetrics& m,
                     const util::NumberFormatter& fmt)
    {
        if (info.hasAbsoluteCounts) {
            doc.addRight(m.totalX, fmt.formatNumber(value));
            doc.add(Format(" (%.1f%%)", 100.0 * value / info.cumulativeWeight));
        } else {
            doc.addRight(m.totalX, String_t(Format("%.1f%%", 100.0 * value / info.cumulativeWeight)));
        }
    }

    void renderLink(ui::rich::Document& doc, int x, String_t value, bool hasSample, String_t link)
    {
        if (hasSample) {
            doc.addRight(x, Text(value).withNewAttribute(new LinkAttribute(link)));
        } else {
            doc.addRight(x, value);
        }
    }


    /*
     *  Helper class to set up and connect a temporary StoppableBusyIndicator
     */
    class RunHelper {
     public:
        RunHelper(SimulationRunProxy& runner, ui::Root& root, afl::string::Translator& tx)
            : m_stopper(root, tx),
              conn1(m_stopper.sig_stop.add(&runner, &SimulationRunProxy::stop)),
              conn2(runner.sig_stop.add(&m_stopper, &client::widgets::StoppableBusyIndicator::stop))
            { }

        void run()
            { m_stopper.run(); }

     private:
        client::widgets::StoppableBusyIndicator m_stopper;
        afl::base::SignalConnection conn1;
        afl::base::SignalConnection conn2;
    };


    /*
     *  Dialog Class
     */
    class SimulationResultDialog {
     public:
        SimulationResultDialog(SimulationSetupProxy& setupProxy,
                               SimulationRunProxy& runProxy,
                               ui::Root& root,
                               afl::string::Translator& tx,
                               util::RequestSender<game::Session> gameSender);

        void init();
        void run();

        void render();
        void renderUnitResult();
        void onUpdate();
        void onScroll();
        void onEdit();
        void onWatchClassSample();
        void onLinkClick(String_t link);

        void playBattle(util::RequestSender<game::proxy::VcrDatabaseAdaptor> adaptor);

        void runOnce();
        void runSeries();
        void runInfinite();

        SimulationResultStatus getResult() const;

     private:
        // Environment
        SimulationSetupProxy& m_setupProxy;
        SimulationRunProxy& m_runProxy;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::Session> m_gameSender;
        util::NumberFormatter m_numberFormatter;

        // Widgets
        client::widgets::SimulationResultList m_resultList;
        client::widgets::SimulationList m_unitList;
        ui::rich::DocumentView m_unitResult;
        ui::widgets::StaticText m_summaryLine1;
        ui::widgets::StaticText m_summaryLine2;
        ui::EventLoop m_loop;

        // Result
        SimulationResultStatus m_result;

        // Signals
        afl::base::SignalConnection conn_update;

        Button& addResultButtons(ui::Group& out, afl::base::Deleter& del, String_t text, util::Key_t key);
    };
}



SimulationResultDialog::SimulationResultDialog(SimulationSetupProxy& setupProxy,
                                               SimulationRunProxy& runProxy,
                                               ui::Root& root,
                                               afl::string::Translator& tx,
                                               util::RequestSender<game::Session> gameSender)
    : m_setupProxy(setupProxy),
      m_runProxy(runProxy),
      m_root(root),
      m_translator(tx),
      m_gameSender(gameSender),
      m_numberFormatter(false, false),
      m_resultList(root),
      m_unitList(root, tx),
      m_unitResult(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(30, 15), 0, m_root.provider()),
      m_summaryLine1("", util::SkinColor::Static, "+", root.provider()),
      m_summaryLine2("", util::SkinColor::Static, "+", root.provider()),
      m_loop(root),
      m_result(),
      conn_update(m_runProxy.sig_update.add(this, &SimulationResultDialog::onUpdate))
{
    m_unitList.setPreferredHeight(12);
    m_unitList.sig_change.add(this, &SimulationResultDialog::onScroll);
    m_unitResult.sig_linkClick.add(this, &SimulationResultDialog::onLinkClick);
}

void
SimulationResultDialog::init()
{
    client::Downlink link(m_root, m_translator);

    // List of players; available on FleetCostProxy
    m_resultList.setPlayers(game::proxy::FleetCostProxy(m_setupProxy.adaptorSender()).getInvolvedPlayers(link));

    // Player names
    m_resultList.setPlayerNames(game::proxy::PlayerProxy(m_gameSender).getPlayerNames(link, game::Player::AdjectiveName));

    // List of units; available on SimulationSetupProxy
    SimulationSetupProxy::ListItems_t list;
    m_setupProxy.getList(link, list);
    m_unitList.setContent(list);

    // NumberFormatter
    m_numberFormatter = game::proxy::ConfigurationProxy(m_gameSender).getNumberFormatter(link);
}

void
SimulationResultDialog::run()
{
    // ex WSimResultWindow::init
    /* Widget structure now looks like this:
         Window
           CardTabBar
           CardGroup
             summaryTab
               list with scrollbar
               m_summaryLine1
               button bar
             resultTab
               list, info widget
               m_summaryLine2
               button bar
       That is, we have two copies of the m_summaryLine, and two copies
       of the buttons, although we only want to exchange one button, namely
       'Watch sample' vs 'Edit ship'. Possible alternatives:
       - rearrange stuff. Tried it, but I'm so used to the CCBSim 1.x layout
         that I think the changed one makes it worse.
       - change the label of the button on the fly. Needs dynamic re-layout. */
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Simulation Results"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    ui::Group& summaryTab = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::Group& resultTab = del.addNew(new ui::Group(ui::layout::VBox::instance5));

    summaryTab.add(del.addNew(new ui::widgets::ScrollbarContainer(m_resultList, m_root)));
    summaryTab.add(m_summaryLine1);

    ui::Group& resultContent = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    resultContent.add(ui::widgets::FrameGroup::wrapWidget(
                          del, m_root.colorScheme(), ui::LoweredFrame,
                          del.addNew(new ui::widgets::ScrollbarContainer(m_unitList, m_root))));
    resultContent.add(m_unitResult);

    resultTab.add(resultContent);
    resultTab.add(m_summaryLine2);

    Button& btnEnter = addResultButtons(summaryTab, del, m_translator("ENTER - Watch sample"), util::Key_Return);
    Button& btnEdit  = addResultButtons(resultTab, del, m_translator("E - Edit this unit"), 'e');

    ui::CardGroup& cards = del.addNew(new ui::CardGroup());
    cards.add(summaryTab);
    cards.add(resultTab);

    CardTabBar& tabs = del.addNew(new CardTabBar(m_root, cards));
    tabs.addPage(util::KeyString(m_translator("Totals")), summaryTab);
    tabs.addPage(util::KeyString(m_translator("Details")), resultTab);
    tabs.setKeys(CardTabBar::Tab + CardTabBar::CtrlTab + CardTabBar::F6 + CardTabBar::Arrows);

    win.add(tabs);
    win.add(cards);
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

    m_resultList.requestFocus();
    win.pack();

    render();

    btnEdit.sig_fire.add(this, &SimulationResultDialog::onEdit);
    btnEnter.sig_fire.add(this, &SimulationResultDialog::onWatchClassSample);

    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

void
SimulationResultDialog::render()
{
    // ex WSimResultWindow::updateResults (sort-of)
    // Class result list
    m_resultList.setClassResults(m_runProxy.getClassResults());

    // Number of units
    String_t countInfo = Format(m_translator("%d simulation%!1{s%}"), m_numberFormatter.formatNumber(static_cast<int32_t>(m_runProxy.getNumBattles())));
    m_summaryLine1.setText(countInfo);
    m_summaryLine2.setText(countInfo);

    // Single unit result
    renderUnitResult();
}

void
SimulationResultDialog::renderUnitResult()
{
    ui::rich::Document& doc = m_unitResult.getDocument();
    doc.clear();

    const size_t index = m_unitList.getCurrentItem();
    if (const SimulationRunProxy::UnitInfo_t* p = m_runProxy.getUnitInfo(index)) {
        afl::string::Translator& tx = m_translator;
        const TableMetrics m = computeUnitResultTableMetrics(m_root, m_translator);

        // ex WSimUnitResultSummary::drawContent
        doc.add(tx(LABELS[0]));
        renderCount(doc, p->numFights, *p, m, m_numberFormatter);
        doc.addNewline();

        doc.add(tx(LABELS[1]));
        renderCount(doc, p->numFightsWon, *p, m, m_numberFormatter);
        doc.addNewline();

        doc.add(tx(LABELS[2]));
        renderCount(doc, p->numCaptures, *p, m, m_numberFormatter);
        doc.addNewline();
        doc.addNewline();

        // ex WSimUnitStat::render
        // Headings
        doc.addRight(m.minX, Text(tx(HEADERS[0])).withStyle(StyleAttribute::Underline));
        doc.addRight(m.aveX, Text(tx(HEADERS[1])).withStyle(StyleAttribute::Underline));
        doc.addRight(m.maxX, Text(tx(HEADERS[2])).withStyle(StyleAttribute::Underline));
        doc.addNewline();

        // Content
        for (size_t i = 0, n = p->info.size(); i < n; ++i) {
            const SimulationRunProxy::UnitInfo_t::Item& item = p->info[i];
            const String_t linkPrefix = Format("%d,%d,", index, static_cast<int>(item.type));
            doc.add(game::sim::toString(item.type, tx));
            renderLink(doc, m.minX, m_numberFormatter.formatNumber(item.min), item.hasMinSample, linkPrefix + "0");
            doc.addRight(m.aveX, Text(Format("%.1f", item.average)));
            renderLink(doc, m.maxX, m_numberFormatter.formatNumber(item.max), item.hasMaxSample, linkPrefix + "1");
            doc.addNewline();
        }
    }

    m_unitResult.handleDocumentUpdate();
}

void
SimulationResultDialog::onUpdate()
{
    render();
}

void
SimulationResultDialog::onScroll()
{
    // ex WSimUnitStat::onScroll
    renderUnitResult();
}

void
SimulationResultDialog::onEdit()
{
    m_result.status = SimulationResultStatus::ScrollToSlot;
    m_result.slot = m_unitList.getCurrentItem();
    m_loop.stop(0);
}

void
SimulationResultDialog::onWatchClassSample()
{
    playBattle(m_runProxy.makeClassResultBattleAdaptor(m_resultList.getCurrentItem()));
}

void
SimulationResultDialog::onLinkClick(String_t link)
{
    // Link format is <index>,<type>,<max>
    int index, type, max;
    util::StringParser p(link);
    bool ok = p.parseInt(index) && p.parseCharacter(',')
        && p.parseInt(type) && p.parseCharacter(',')
        && p.parseInt(max) && p.parseEnd();

    if (ok) {
        playBattle(m_runProxy.makeUnitResultBattleAdaptor(static_cast<size_t>(index),
                                                          static_cast<game::sim::ResultList::UnitInfo::Type>(type),
                                                          max != 0));
    }
}

void
SimulationResultDialog::playBattle(util::RequestSender<game::proxy::VcrDatabaseAdaptor> adaptor)
{
    afl::sys::Log log; // FIXME: for now, ground the logs
    game::Reference ref = client::dialogs::playCombat(m_root, m_translator, adaptor, m_gameSender, log);
    if (ref.isSet()) {
        m_result.status = SimulationResultStatus::GoToReference;
        m_result.reference = ref;
        m_loop.stop(0);
    }
}

void
SimulationResultDialog::runOnce()
{
    RunHelper h(m_runProxy, m_root, m_translator);
    m_runProxy.runFinite(1);
    h.run();
}

void
SimulationResultDialog::runSeries()
{
    RunHelper h(m_runProxy, m_root, m_translator);
    m_runProxy.runSeries();
    h.run();
}

void
SimulationResultDialog::runInfinite()
{
    RunHelper h(m_runProxy, m_root, m_translator);
    m_runProxy.runInfinite();
    h.run();
}

client::dialogs::SimulationResultStatus
SimulationResultDialog::getResult() const
{
    return m_result;
}

Button&
SimulationResultDialog::addResultButtons(ui::Group& out, afl::base::Deleter& del, String_t text, util::Key_t key)
{
    // Buttons
    Button& btnClose  = del.addNew(new Button(m_translator("Close"),         util::Key_Escape, m_root));
    Button& btnHelp   = del.addNew(new Button(m_translator("Help"),          'h',              m_root));
    Button& btnOnce   = del.addNew(new Button(m_translator("Space - Again"), ' ',              m_root));
    Button& btnSeries = del.addNew(new Button(m_translator("S - Series"),    's',              m_root));
    Button& btnRepeat = del.addNew(new Button(m_translator("R - Repeat"),    'r',              m_root));
    Button& btnView   = del.addNew(new Button(text,                          key,              m_root));

    client::widgets::HelpWidget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:simresult"));

    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnHelp.dispatchKeyTo(help);
    btnOnce.sig_fire.add(this, &SimulationResultDialog::runOnce);
    btnSeries.sig_fire.add(this, &SimulationResultDialog::runSeries);
    btnRepeat.sig_fire.add(this, &SimulationResultDialog::runInfinite);

    // First line
    ui::Group& g1 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g1.add(btnOnce);
    g1.add(btnSeries);
    g1.add(btnRepeat);
    g1.add(del.addNew(new ui::Spacer()));
    out.add(g1);

    // Second line
    ui::Group& g2 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g2.add(btnView);
    g2.add(btnClose);
    g2.add(del.addNew(new ui::Spacer()));
    g2.add(btnHelp);
    out.add(g2);
    out.add(help);

    return btnView;
}


/*
 *  Entry Point
 */

client::dialogs::SimulationResultStatus
client::dialogs::doBattleSimulationResults(game::proxy::SimulationSetupProxy& setupProxy,
                                           game::proxy::SimulationRunProxy& runProxy,
                                           ui::Root& root,
                                           afl::string::Translator& tx,
                                           util::RequestSender<game::Session> gameSender)
{
    SimulationResultDialog dlg(setupProxy, runProxy, root, tx, gameSender);
    dlg.init();
    dlg.run();
    return dlg.getResult();
}
