/**
  *  \file client/dialogs/imperialstats.cpp
  *  \brief Imperial Statistics dialog
  */

#include "client/dialogs/imperialstats.hpp"
#include "afl/base/deleter.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/xml/nodereader.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/planetinfodialog.hpp"
#include "client/dialogs/sessionfileselectiondialog.hpp"
#include "client/si/control.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/map/info/scriptlinkbuilder.hpp"
#include "game/proxy/imperialstatsproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/documentparser.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/string.hpp"
#include "util/stringparser.hpp"

using afl::string::Format;
using client::si::OutputState;
using client::si::RequestLink2;
using client::widgets::HelpWidget;
using game::Reference;
using game::map::info::Nodes_t;
using game::map::info::Page;
using game::map::info::PageOptions_t;
using ui::EventLoop;
using ui::Group;
using ui::Spacer;
using ui::rich::DocumentView;
using ui::widgets::Button;

namespace {
    class Dialog;

    const char*const TASK_NAME = "(Imperial Statistics)";

    /*
     *  Button to select a page
     */
    class PageButton : public Button {
     public:
        PageButton(Page page, const util::KeyString& ks, ui::Root& root, Dialog& dlg)
            : Button(ks, root),
              m_page(page),
              m_dialog(dlg)
            {
                sig_fire.add(this, &PageButton::onClick);
                setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
            }

        void onClick();

        Page getPage() const
            { return m_page; }

     private:
        Page m_page;
        Dialog& m_dialog;
    };

    /*
     *  LinkBuilder
     *
     *  We want to offer more than just "activate this link" with planets,
     *  so we need to associate some metainformation with the links to allow a UI-side decision to be made.
     */
    class LinkBuilder : public game::map::info::LinkBuilder {
     public:
        virtual String_t makePlanetLink(const game::map::Planet& pl) const
            {
                game::map::Point pt;
                if (pl.isPlayable(game::map::Object::ReadOnly) && pl.getPosition().get(pt)) {
                    return Format("pl:%d,%d,%d,%d", pl.getId(), int(pl.hasBase()), pt.getX(), pt.getY());
                } else {
                    return String_t();
                }
            }
        virtual String_t makeSearchLink(const game::SearchQuery& q) const
            {
                // Keep regular 'q' format for those
                return game::map::info::ScriptLinkBuilder().makeSearchLink(q);
            }
    };

    struct PlanetLink {
        int id;
        int hasBase;
        int x, y;
    };

    /* Check and parse a link created by LinkBuilder::makePlanetLink() */
    bool parsePlanetLink(const String_t& str, PlanetLink& out)
    {
        util::StringParser p(str);
        return p.parseString("pl:") && p.parseInt(out.id)
            && p.parseString(",") && p.parseInt(out.hasBase)
            && p.parseString(",") && p.parseInt(out.x)
            && p.parseString(",") && p.parseInt(out.y)
            && p.parseEnd();
    }


    /*
     *  Imperial Statistics dialog
     *
     *  This hooks together an ImperialStatsProxy and a DocumentView,
     *  with some buttons to select the current page and options.
     *  Links are directly executed.
     */
    class Dialog : public client::si::Control, private gfx::KeyEventConsumer {
     public:
        Dialog(client::si::UserSide& userSide, OutputState& outputState)
            : Control(userSide),
              m_userSide(userSide),
              m_outputState(outputState),
              m_loop(userSide.root()),
              m_proxy(userSide.gameSender(), userSide.root().engine().dispatcher(), std::auto_ptr<game::map::info::LinkBuilder>(new LinkBuilder())),
              m_docView(userSide.root().provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(10, 10), DocumentView::fl_SingleHyper, userSide.root().provider()),
              m_optionsButton(userSide.translator()("# - Options"), '#', userSide.root()),
              m_pageButtons(),
              m_currentPage(game::map::info::TotalsPage),
              m_numOptionRequests(0),
              m_options(),
              m_currentOptions()
            {
                // Events
                m_proxy.sig_pageContent.add(this, &Dialog::onPageContent);
                m_proxy.sig_pageOptions.add(this, &Dialog::onPageOptions);
                m_optionsButton.sig_fire.add(this, &Dialog::onOptions);
                m_docView.sig_linkClick.add(this, &Dialog::onLinkClick);

                // Page buttons
                afl::string::Translator& tx = translator();
                addPageButton(game::map::info::TotalsPage,    tx("T - Totals"));
                addPageButton(game::map::info::MineralsPage,  tx("M - Minerals"));
                addPageButton(game::map::info::PlanetsPage,   tx("P - Planets"));
                addPageButton(game::map::info::ColonyPage,    tx("O - Colony"));
                addPageButton(game::map::info::StarbasePage,  tx("B - Starbases"));
                addPageButton(game::map::info::StarshipPage,  tx("S - Starships"));
                addPageButton(game::map::info::CapitalPage,   tx("C - Capital"));
                addPageButton(game::map::info::StarchartPage, tx("A - Starchart"));
                addPageButton(game::map::info::WeaponsPage,   tx("W - Weapons"));
            }

        void addPageButton(Page page, String_t label);
        void run();
        void onPageContent(Nodes_t& nodes);
        void onPageOptions(const util::StringList& opts, PageOptions_t current);
        void onLinkClick(String_t link);
        void onOptions();
        void onSave();

        virtual bool handleKey(util::Key_t key, int prefix);

        void setPage(Page page);
        void browsePage(bool down);

        /*
         *  Control Methods
         */
        virtual void handleStateChange(RequestLink2 link, OutputState::Target target)
            { dialogHandleStateChange(link, target, m_outputState, m_loop, 0); }
        virtual void handleEndDialog(RequestLink2 link, int code)
            { dialogHandleEndDialog(link, code, m_outputState, m_loop, 0); }
        virtual void handlePopupConsole(RequestLink2 link)
            { defaultHandlePopupConsole(link); }
        virtual void handleScanKeyboardMode(RequestLink2 link)
            { defaultHandleScanKeyboardMode(link); }
        virtual void handleSetView(RequestLink2 link, String_t name, bool withKeymap)
            { defaultHandleSetView(link, name, withKeymap); }
        virtual void handleUseKeymap(RequestLink2 link, String_t name, int prefix)
            { defaultHandleUseKeymap(link, name, prefix); }
        virtual void handleOverlayMessage(RequestLink2 link, String_t text)
            { defaultHandleOverlayMessage(link, text); }
        virtual game::interface::ContextProvider* createContextProvider()
            { return 0; }

     private:
        void requestPage();
        void highlightPage();
        bool parseCurrentPlanetLink(PlanetLink& out) const;

        /*
         *  UI components / Plumbing
         */
        client::si::UserSide& m_userSide;
        OutputState& m_outputState;
        EventLoop m_loop;
        game::proxy::ImperialStatsProxy m_proxy;
        DocumentView m_docView;
        Button m_optionsButton;

        /*
         *  Page selection
         */
        afl::container::PtrVector<PageButton> m_pageButtons;
        Page m_currentPage;

        /*
         *  Option choices
         *
         *  We count the number of requestPageOptions() sent,
         *  and only allow the user to open the menu when no call is outstanding,
         *  to avoid opening the wrong menu.
         */
        int m_numOptionRequests;
        util::StringList m_options;
        PageOptions_t m_currentOptions;
    };
}

void
PageButton::onClick()
{
    m_dialog.setPage(m_page);
}

void
Dialog::addPageButton(Page page, String_t label)
{
    m_pageButtons.pushBackNew(new PageButton(page, util::KeyString(label), root(), *this));
}

void
Dialog::run()
{
    // ex WImperialStatisticsWindow::init()
    // Group [HBox]
    //   DocumentView
    //   Group [VBox]
    //     Button "Totals"
    //     ...
    //     Spacer
    //     Group [HBox]
    //       "Help"
    //       Spacer
    //       "Close"

    afl::string::Translator& tx = m_userSide.translator();
    ui::Root& root = m_userSide.root();
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(tx("Imperial Statistics"), root.provider(), root.colorScheme(), ui::BLUE_DARK_WINDOW, ui::layout::HBox::instance5));

    // Left side (document view)
    win.add(m_docView);

    // Right side (buttons)
    Button& btnSave = del.addNew(new Button(tx("Ctrl-S - Save"), 's' + util::KeyMod_Ctrl, root));
    Group& g1 = del.addNew(new Group(ui::layout::VBox::instance5));
    for (size_t i = 0; i < m_pageButtons.size(); ++i) {
        g1.add(*m_pageButtons[i]);
    }
    g1.add(del.addNew(new Spacer(gfx::Point(10, 10))));
    g1.add(m_optionsButton);
    g1.add(btnSave);
    g1.add(del.addNew(new Spacer()));

    // Bottom-right buttons
    Button& btnHelp  = del.addNew(new Button(tx("Help"),  'h',              root));
    Button& btnClose = del.addNew(new Button(tx("Close"), util::Key_Escape, root));
    Group& g11 = del.addNew(new Group(ui::layout::HBox::instance5));
    g11.add(btnHelp);
    g11.add(del.addNew(new Spacer()));
    g11.add(btnClose);
    g1.add(g11);
    win.add(g1);

    // Utilities
    HelpWidget& help = del.addNew(new HelpWidget(root, tx, m_userSide.gameSender(), "pcc2:imperial"));
    win.add(help);
    win.add(del.addNew(new ui::widgets::Quit(root, m_loop)));
    win.add(del.addNew(new ui::widgets::KeyForwarder(*this)));

    // Events
    btnSave.sig_fire.add(this, &Dialog::onSave);
    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnHelp.dispatchKeyTo(help);

    // Request content
    requestPage();
    highlightPage();

    // Operate window
    win.setExtent(root.getExtent());
    root.add(win);
    m_loop.run();
}

/* Event handler: content from ImperialStatsProxy; render */
void
Dialog::onPageContent(Nodes_t& nodes)
{
    // Build a reader
    afl::io::xml::NodeReader rdr;
    for (size_t i = 0, n = nodes.size(); i < n; ++i) {
        rdr.addNode(nodes[i]);
    }

    // Render
    ui::rich::Document& doc = m_docView.getDocument();
    doc.clear();
    m_docView.setTopY(0);
    ui::rich::DocumentParser parser(doc, rdr);
    parser.parseDocument();
    doc.finish();
    m_docView.handleDocumentUpdate();
}

/* Event handler: options from ImperialStatsProxy; render */
void
Dialog::onPageOptions(const util::StringList& opts, PageOptions_t current)
{
    if (m_numOptionRequests > 0) {
        --m_numOptionRequests;
    }
    m_options = opts;
    m_optionsButton.setState(ui::Widget::DisabledState, m_options.empty());
    m_currentOptions = current;
}

/* Event handler: link clicked in document */
void
Dialog::onLinkClick(String_t link)
{
    // ex WImperialStatisticsWindow::onLinkClick(string_t link)
    PlanetLink pl;
    if (parsePlanetLink(link, pl)) {
        executeGoToReferenceWait(TASK_NAME, Reference(Reference::Planet, pl.id));
    } else if (const char* cmd = util::strStartsWith(link, "q:")) {
        executeCommandWait(cmd, false, TASK_NAME);
    } else {
        // what?
    }
}

/* Event handler: "Options" button */
void
Dialog::onOptions()
{
    // ex WImperialStatisticsWindow::doOptions(UIBaseWidget& sender)
    if (m_numOptionRequests == 0 && !m_options.empty()) {
        ui::Root& root = m_userSide.root();
        EventLoop loop(root);

        ui::widgets::StringListbox list(root.provider(), root.colorScheme());
        list.setItems(m_options);
        list.setCurrentKey(m_currentOptions);

        if (ui::widgets::MenuFrame(ui::layout::HBox::instance5, root, loop).doMenu(list, m_optionsButton.getExtent().getBottomLeft())) {
            int32_t opts;
            if (list.getCurrentKey(opts)) {
                m_proxy.setPageOptions(m_currentPage, static_cast<PageOptions_t>(opts));
                requestPage();
            }
        }
    }
}

/* Event handler: "Save" button */
void
Dialog::onSave()
{
    ui::Root& root = m_userSide.root();
    afl::string::Translator& tx = translator();

    client::dialogs::SessionFileSelectionDialog dlg(root, tx, m_userSide.gameSender(), tx("Save Page"));
    dlg.setPattern(util::FileNamePattern::getAllFilesWithExtensionPattern("html"));
    dlg.setDefaultExtension("html");
    client::Downlink link(root, tx);
    if (dlg.runDefault(link)) {
        String_t fileName = dlg.getResult();
        String_t errorMessage;
        if (!m_proxy.savePageAsHTML(link, m_currentPage, fileName, errorMessage)) {
            ui::dialogs::MessageBox(afl::string::Format("Unable to save %s: %s", fileName, errorMessage), tx("Save Page"), root)
                .doOkDialog(tx);
        }
    }
}

/* Key event handler */
bool
Dialog::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex WImperialStatisticsWindow::handleEvent
    PlanetLink pl;
    switch (key) {
     case util::Key_Up:
     case util::Key_PgUp:
        browsePage(false);
        return true;

     case util::Key_Down:
     case util::Key_PgDn:
        browsePage(true);
        return true;

     // TODO: PCC1 handles F1, F6, but for now we do not generate links to ships

     case util::Key_F2:
        if (parseCurrentPlanetLink(pl)) {
            executeGoToReferenceWait(TASK_NAME, Reference(Reference::Planet, pl.id));
        }
        return true;

     case util::Key_F3:
        if (parseCurrentPlanetLink(pl) && pl.hasBase != 0) {
            executeGoToReferenceWait(TASK_NAME, Reference(Reference::Starbase, pl.id));
        }
        return true;

     case util::Key_F4:
        if (parseCurrentPlanetLink(pl)) {
            executeGoToReferenceWait(TASK_NAME, Reference(game::map::Point(pl.x, pl.y)));
        }
        return true;

     case util::Key_F5:
        if (parseCurrentPlanetLink(pl)) {
            client::dialogs::doPlanetInfoDialog(root(), interface().gameSender(), pl.id, translator());
        }
        return true;

     default:
        return false;
    }
}

/* Show page, given a page Id */
void
Dialog::setPage(Page page)
{
    // ex WImperialStatisticsWindow::setPage(int n)
    if (page != m_currentPage) {
        m_currentPage = page;
        highlightPage();
        requestPage();
    }
}

/* Browse through pages in up/down fashion */
void
Dialog::browsePage(bool down)
{
    size_t currentIndex = 0;
    for (size_t i = 0; i < m_pageButtons.size(); ++i) {
        if (m_pageButtons[i]->getPage() == m_currentPage) {
            currentIndex = i;
            break;
        }
    }
    if (down) {
        ++currentIndex;
        if (currentIndex >= m_pageButtons.size()) {
            currentIndex = 0;
        }
    } else {
        if (currentIndex == 0) {
            currentIndex = m_pageButtons.size();
        }
        --currentIndex;
    }
    if (currentIndex < m_pageButtons.size()) {
        setPage(m_pageButtons[currentIndex]->getPage());
    }
}

/* Request page and meta-info from proxy */
void
Dialog::requestPage()
{
    m_proxy.requestPageContent(m_currentPage);
    m_proxy.requestPageOptions(m_currentPage);
    ++m_numOptionRequests;
}

/* Highlight current page */
void
Dialog::highlightPage()
{
    for (size_t i = 0; i < m_pageButtons.size(); ++i) {
        m_pageButtons[i]->setFlag(ui::HighlightedButton, m_pageButtons[i]->getPage() == m_currentPage);
    }
}

/* Parse current link as planet link */
bool
Dialog::parseCurrentPlanetLink(PlanetLink& out) const
{
    return parsePlanetLink(m_docView.getDocument().getLinkTarget(m_docView.getSelectedLink()), out);
}


/*
 *  Main Entry Point
 */

void
client::dialogs::doImperialStatistics(client::si::UserSide& userSide, client::si::OutputState& outputState)
{
    // ex doImperialStatistics
    Dialog(userSide, outputState).run();
}
