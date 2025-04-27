/**
  *  \file client/dialogs/searchdialog.cpp
  *  \brief Search Dialog
  */

#include "client/dialogs/searchdialog.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/deleter.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/selectionmanager.hpp"
#include "client/downlink.hpp"
#include "client/proxy/screenhistoryproxy.hpp"
#include "client/si/control.hpp"
#include "client/widgets/expressionlist.hpp"
#include "client/widgets/referencelistbox.hpp"
#include "game/interface/referencecontext.hpp"
#include "game/interface/referencelistcontext.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/expressionlistproxy.hpp"
#include "game/proxy/referencelistproxy.hpp"
#include "game/proxy/referenceproxy.hpp"
#include "game/proxy/searchproxy.hpp"
#include "game/proxy/selectionproxy.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/process.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/checkbox.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/numberformatter.hpp"
#include "util/translation.hpp"
#include "util/unicodechars.hpp"

using afl::functional::createStringTable;
using game::SearchQuery;
using game::proxy::SelectionProxy;
using interpreter::Opcode;
using ui::widgets::FrameGroup;

namespace {
    static const char*const NO_YES[] = {
        N_("no"),
        N_("yes"),
    };

    // Definition of "Match type" values
    static const char*const MATCH_TYPES[] = {
        N_("Name/Id/Comment"),
        N_("Expression true"),
        N_("Expression false"),
        N_("Location"),
    };
    static_assert(SearchQuery::MatchName == 0, "MatchName");
    static_assert(SearchQuery::MatchTrue == 1, "MatchTrue");
    static_assert(SearchQuery::MatchFalse == 2, "MatchFalse");
    static_assert(SearchQuery::MatchLocation == 3, "MatchLocation");

    enum {
        Option_SearchObjects,
        Option_MatchType,
        Option_PlayedOnly
    };

    // Definition of "Search Object" values
    struct SearchObjectDefinition {
        SearchQuery::SearchObject obj;
        util::Key_t key;
        const char* name;
    };
    const SearchObjectDefinition SEARCH_OBJECTS[] = {
        { SearchQuery::SearchShips,   's', N_("Starships") },
        { SearchQuery::SearchPlanets, 'p', N_("Planets")   },
        { SearchQuery::SearchBases,   'b', N_("Starbases") },
        { SearchQuery::SearchUfos,    'u', N_("Ufos")      },
        { SearchQuery::SearchOthers,  'o', N_("Others")    },
    };
    const size_t NUM_SEARCH_OBJECT = countof(SEARCH_OBJECTS);


    /* Locate current object in list.
       IFUISearch will only provide a Ship or Planet references.
       However, a search result may contain Starbase references which we want to treat identically to Planet. */
    bool findObject(const game::ref::UserList& list, game::Reference currentObject, size_t& newPos)
    {
        return list.find(currentObject).get(newPos)
            || (currentObject.getType() == game::Reference::Planet
                && list.find(game::Reference(game::Reference::Starbase, currentObject.getId())).get(newPos));
    }


    /*
     *  Search Object Selection Dialog
     */

    class SearchObjectDialog {
     public:
        // ex WSearchObjectSelector (sort-of)
        SearchObjectDialog(ui::Root& root, afl::string::Translator& tx)
            : m_root(root),
              m_translator(tx),
              m_buttons(root, tx),
              m_inhibitClick()
            { }

        void run(SearchQuery::SearchObjects_t& objs)
            {
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(m_translator("Search Object"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
                ui::widgets::FocusIterator& it = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab + ui::widgets::FocusIterator::Vertical));

                ui::widgets::Checkbox& cbAll = del.addNew(new ui::widgets::Checkbox(m_root, 'a', m_translator("All"), m_all));
                cbAll.addDefaultImages();
                win.add(cbAll);
                it.add(cbAll);
                m_all.sig_change.add(this, &SearchObjectDialog::onAllChange);

                for (size_t i = 0; i < NUM_SEARCH_OBJECT; ++i) {
                    ui::widgets::Checkbox& cb = del.addNew(new ui::widgets::Checkbox(m_root, SEARCH_OBJECTS[i].key, m_translator(SEARCH_OBJECTS[i].name), m_values[i]));
                    cb.addDefaultImages();
                    win.add(cb);
                    it.add(cb);
                    m_values[i].set(objs.contains(SEARCH_OBJECTS[i].obj));
                    m_values[i].sig_change.add(this, &SearchObjectDialog::onClick);
                }
                win.add(m_buttons);
                win.add(it);
                onClick();

                ui::EventLoop loop(m_root);
                m_buttons.addStop(loop);

                win.pack();
                m_root.centerWidget(win);
                m_root.add(win);
                if (loop.run() != 0) {
                    objs = getStatus();
                }
            }

        void onClick()
            {
                if (!m_inhibitClick) {
                    m_buttons.ok().setState(ui::Widget::DisabledState, getStatus().empty());
                    m_all.set(isAll());
                }
            }

        bool isAll()
            {
                for (size_t i = 0; i < NUM_SEARCH_OBJECT; ++i) {
                    if (m_values[i].get() == 0) {
                        return false;
                    }
                }
                return true;
            }

        void onAllChange()
            {
                if (m_all.get() != 0) {
                    // This will trigger onClick() notifications; suppress processing those.
                    // They would re-trigger this callback, which is harmless, but unnecessary.
                    m_inhibitClick = true;
                    for (size_t i = 0; i < NUM_SEARCH_OBJECT; ++i) {
                        m_values[i].set(1);
                    }
                    m_inhibitClick = false;
                }
            }

        SearchQuery::SearchObjects_t getStatus() const
            {
                SearchQuery::SearchObjects_t result;
                for (size_t i = 0; i < NUM_SEARCH_OBJECT; ++i) {
                    if (m_values[i].get() != 0) {
                        result += SEARCH_OBJECTS[i].obj;
                    }
                }
                return result;
            }

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        afl::base::Observable<int> m_all;
        afl::base::Observable<int> m_values[NUM_SEARCH_OBJECT];
        ui::widgets::StandardDialogButtons m_buttons;
        bool m_inhibitClick;
    };


    /*
     *  Search Dialog
     */

    enum {
        Stop_Normal,            // Dialog exits normally
        Stop_Global             // Global Actions requested
    };

    class SearchDialog : public client::si::Control {
     public:
        SearchDialog(const SearchQuery& initialQuery, game::Reference currentObject, game::ref::List& result, client::si::UserSide& iface, util::NumberFormatter fmt, client::si::OutputState& out, bool isSubDialog);

        void loadQuery(game::proxy::WaitIndicator& ind);
        int run(bool immediate);

        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target);
        virtual void handleEndDialog(client::si::RequestLink2 link, int code);
        virtual void handlePopupConsole(client::si::RequestLink2 link);
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link);
        virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text);
        virtual afl::base::Optional<game::Id_t> getFocusedObjectId(game::Reference::Type type) const;
        virtual game::interface::ContextProvider* createContextProvider();

     private:
        enum State {
            Idle,
            Searching,
            Sorting
        };

        // References
        util::NumberFormatter m_format;
        client::si::OutputState& m_outputState;

        // Proxies
        game::proxy::ReferenceListProxy m_refListProxy;
        game::proxy::SearchProxy m_searchProxy;
        game::proxy::ExpressionListProxy m_exProxy;

        // Widgets
        ui::EventLoop m_loop;
        ui::widgets::InputLine m_input;
        ui::widgets::OptionGrid m_options;
        ui::widgets::StaticText m_resultStatus;
        ui::widgets::Button m_btnSearch;
        ui::widgets::Button m_btnGoto;
        ui::widgets::Button m_btnClose;
        ui::widgets::Button m_btnMark;
        ui::widgets::Button m_btnGlobal;
        ui::widgets::Button m_btnHelp;
        ui::widgets::Button m_btnHistory;
        client::widgets::ReferenceListbox m_refList;
        bool m_isSubDialog;

        // Status
        SearchQuery m_query;
        game::ref::List& m_result;

        // Current object
        const game::Reference m_currentObject;

        void onSearch();
        void onSuccess(const game::ref::List& list);
        void onError(String_t err);
        void onListChange(const game::ref::UserList& list);
        void onOptionClick(int id);
        void onGoto();
        void onMark();

        void onReturn();
        void onDown();
        void onHistory();
        void onGoToShip();
        void onGoToPlanet();
        void onGoToBase();
        void onGoToMap();
        void onGoToHistory();

        void onSelectionManager();
        void onPreviousSelectionLayer();
        void onNextSelectionLayer();

        void editSearchObjects();
        void editMatchType();
        void setValues();
        void openControlScreen(game::Reference::Type refType,
                               client::ScreenHistory::Type histType,
                               client::si::OutputState::Target target);

        void setListContent(const game::ref::List& list);
    };

    class SearchObjectLabel : public afl::functional::StringTable_t {
     public:
        SearchObjectLabel(afl::string::Translator& tx)
            : m_translator(tx)
            { }
        virtual String_t get(int32_t a) const
            { return SearchQuery::formatSearchObjects(SearchQuery::SearchObjects_t::fromInteger(a), m_translator); }
        virtual bool getFirstKey(int32_t& a) const
            { a = 0; return true; }
        virtual bool getNextKey(int32_t& a) const
            { ++a; return a <= int32_t(SearchQuery::allObjects().toInteger()); }
     private:
        afl::string::Translator& m_translator;
    };
}


inline
SearchDialog::SearchDialog(const SearchQuery& initialQuery, game::Reference currentObject, game::ref::List& result, client::si::UserSide& iface, util::NumberFormatter fmt, client::si::OutputState& out, bool isSubDialog)
    : Control(iface),
      m_format(fmt),
      m_outputState(out),
      m_refListProxy(iface.gameSender(), root().engine().dispatcher()),
      m_searchProxy(iface.gameSender(), root().engine().dispatcher()),
      m_exProxy(iface.gameSender(), game::config::ExpressionLists::Search),
      m_loop(root()),
      m_input(1000, 30, root()),
      m_options(0, 0, root()),
      m_resultStatus(String_t(), util::SkinColor::Static, gfx::FontRequest().addSize(1), root().provider(), gfx::LeftAlign),
      m_btnSearch(translator()("Search!"), 0, root()),
      m_btnGoto(translator()("Go to"), 0, root()),
      m_btnClose(translator()("Close"), util::Key_Escape, root()),
      m_btnMark(translator()("Mark..."), 'm', root()),
      m_btnGlobal(translator()("Global..."), 'g', root()),
      m_btnHelp(translator()("Help"), 'h', root()),
      m_btnHistory(UTF_DOWN_ARROW, 0, root()),
      m_refList(root()),
      m_isSubDialog(isSubDialog),
      m_query(initialQuery),
      m_result(result),
      m_currentObject(currentObject)
{
    // ex WSearchDialog::WSearchDialog
    m_searchProxy.sig_success.add(this, &SearchDialog::onSuccess);
    m_searchProxy.sig_error.add(this, &SearchDialog::onError);
    m_refListProxy.sig_listChange.add(this, &SearchDialog::onListChange);
    m_refListProxy.setConfigurationSelection(game::ref::SEARCH);
    m_options.sig_click.add(this, &SearchDialog::onOptionClick);
    m_btnSearch.sig_fire.add(this, &SearchDialog::onSearch);
    m_btnHistory.sig_fire.add(this, &SearchDialog::onHistory);
    m_refList.sig_itemDoubleClick.add(this, &SearchDialog::onReturn);
}

void
SearchDialog::loadQuery(game::proxy::WaitIndicator& ind)
{
    m_query = m_searchProxy.getSavedQuery(ind);
}

int
SearchDialog::run(bool immediate)
{
    // ex WSearchDialog::init (sort-of)
    // VBox
    //   OptionGrid
    //   HBox
    //   HBox g1
    //     FrameGroup > InputLine
    //     Button (history dropdown)
    //   HBox g2
    //     StaticText m_resultStatus
    //     Button "Search!"
    //   FrameGroup > ReferenceListbox   // tbd: scrollbar
    //   HBox g4
    //     Button "Go to"
    //     Button "Close"
    //     Button "Mark"
    //     Button "Global"
    //     Spacer
    //     Button "Help"

    afl::string::Translator& tx = translator();
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(tx("Search Object"), root().provider(), root().colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    // Options
    m_options.addItem(Option_SearchObjects, 'o', tx("Objects"))
        .addPossibleValues(SearchObjectLabel(tx));
    m_options.addItem(Option_MatchType,     't', tx("Search type"))
        .addPossibleValues(createStringTable(MATCH_TYPES).map(tx));
    m_options.addItem(Option_PlayedOnly,    'p', tx("Played objects only"))
        .addPossibleValues(createStringTable(NO_YES).map(tx));
    win.add(m_options);

    // Input
    ui::Group& g1 = del.addNew(new ui::Group(ui::layout::HBox::instance0));
    g1.add(FrameGroup::wrapWidget(del, root().colorScheme(), ui::LoweredFrame, m_input));
    g1.add(m_btnHistory);
    win.add(g1);
    m_input.setFont(gfx::FontRequest().addSize(1));

    // "Search!" button
    ui::Group& g2 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    m_resultStatus.setIsFlexible(true);
    g2.add(m_resultStatus);
    g2.add(m_btnSearch);
    win.add(g2);

    // Result list
    win.add(FrameGroup::wrapWidget(del, root().colorScheme(), ui::LoweredFrame,
                                   del.addNew(new ui::widgets::ScrollbarContainer(m_refList, root()))));
    m_refList.setNumLines(20);

    // Lower buttons
    ui::Group& g4 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    if (m_isSubDialog) {
        g4.add(m_btnClose);
        g4.add(m_btnMark);
    } else {
        g4.add(m_btnGoto);
        g4.add(m_btnClose);
        g4.add(m_btnMark);
        g4.add(m_btnGlobal);
    }
    g4.add(del.addNew(new ui::Spacer()));
    g4.add(m_btnHelp);
    win.add(g4);
    m_btnGoto.sig_fire.add(this, &SearchDialog::onGoto);
    m_btnClose.sig_fire.addNewClosure(m_loop.makeStop(Stop_Normal));
    m_btnMark.sig_fire.add(this, &SearchDialog::onMark);
    m_btnGlobal.sig_fire.addNewClosure(m_loop.makeStop(Stop_Global));

    // Admin
    ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
    disp.add(util::Key_Return, this, &SearchDialog::onReturn);
    disp.add(util::Key_Down,   this, &SearchDialog::onDown);
    disp.add(util::Key_F7,     &m_input, &ui::widgets::InputLine::requestFocus);
    disp.add(util::Key_F1,     this, &SearchDialog::onGoToShip);
    disp.add(util::Key_F2,     this, &SearchDialog::onGoToPlanet);
    disp.add(util::Key_F3,     this, &SearchDialog::onGoToBase);
    disp.add(util::Key_F4,     this, &SearchDialog::onGoToMap);
    disp.add(util::Key_F6,     this, &SearchDialog::onGoToHistory);
    disp.add(util::KeyMod_Alt + '.', this, &SearchDialog::onSelectionManager);
    disp.add(util::KeyMod_Alt + util::Key_Left, this, &SearchDialog::onPreviousSelectionLayer);
    disp.add(util::KeyMod_Alt + util::Key_Right, this, &SearchDialog::onNextSelectionLayer);
    disp.add(util::KeyMod_Shift + util::Key_Return, this, &SearchDialog::onGoToMap);
    disp.add(util::KeyMod_Ctrl + util::Key_Pause, &interface(), &client::si::UserSide::interruptRunningProcesses);
    win.add(disp);

    ui::widgets::FocusIterator& it = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab));
    it.add(m_input);
    it.add(m_refList);
    win.add(it);

    win.add(del.addNew(new ui::widgets::Quit(root(), m_loop)));

    // Setup
    setValues();
    m_input.setText(m_query.getQuery());
    m_input.requestFocus();
    onListChange(game::ref::UserList());

    // Run
    win.pack();
    root().centerWidget(win);
    root().add(win);
    if (immediate) {
        onSearch();
    }
    return m_loop.run();
}

void
SearchDialog::handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
{
    dialogHandleStateChange(link, target, m_outputState, m_loop, Stop_Normal);
}

void
SearchDialog::handleEndDialog(client::si::RequestLink2 link, int code)
{
    dialogHandleEndDialog(link, code, m_outputState, m_loop, Stop_Normal);
}

void
SearchDialog::handlePopupConsole(client::si::RequestLink2 link)
{
    defaultHandlePopupConsole(link);
}

void
SearchDialog::handleScanKeyboardMode(client::si::RequestLink2 link)
{
    defaultHandleScanKeyboardMode(link);
}

void
SearchDialog::handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetView(link, name, withKeymap);
}

void
SearchDialog::handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
{
    defaultHandleUseKeymap(link, name, prefix);
}

void
SearchDialog::handleOverlayMessage(client::si::RequestLink2 link, String_t text)
{
    defaultHandleOverlayMessage(link, text);
}

afl::base::Optional<game::Id_t>
SearchDialog::getFocusedObjectId(game::Reference::Type type) const
{
    return defaultGetFocusedObjectId(type);
}

game::interface::ContextProvider*
SearchDialog::createContextProvider()
{
    return 0; // or m_parentControl.createContextProvider()?
}

void
SearchDialog::onSearch()
{
    // ex WSearchDialog::onSearch
    // Clear result list.
    setListContent(game::ref::List());
    m_resultStatus.setText(String_t());
    m_input.requestFocus();

    // Update search query
    m_query.setQuery(m_input.getText());

    // Execute query; this will answer with onSuccess or onError.
    m_searchProxy.search(m_query, true);
}

void
SearchDialog::onSuccess(const game::ref::List& list)
{
    // ex WSearchDialog::onResultChange (part)
    afl::string::Translator& tx = translator();
    m_resultStatus.setText(afl::string::Format(tx("%d result%!1{s%}"),
                                               m_format.formatNumber(static_cast<int32_t>(list.size()))));
    m_result = list;

    if (list.size() == 0) {
        // Nothing found
        ui::dialogs::MessageBox(tx("Your query didn't match any object."),
                                tx("Search Object"),
                                root()).doOkDialog(tx);
    } else {
        // Set list content. This will answer with onListChange.
        setListContent(list);
    }

    if (afl::string::strTrim(m_query.getQuery()).empty()) {
        // empty query (match all); ignore
    } else if (m_query.getMatchType() == SearchQuery::MatchTrue) {
        m_exProxy.pushRecent("[]", m_query.getQuery());
    } else if (m_query.getMatchType() == SearchQuery::MatchFalse) {
        m_exProxy.pushRecent("[!]", m_query.getQuery());
    } else {
        // query for name/position does not go on LRU list
    }
}

void
SearchDialog::onError(String_t err)
{
    afl::string::Translator& tx = translator();
    ui::dialogs::MessageBox(err, tx("Search Object"), root()).doOkDialog(tx);
    m_input.requestFocus();
}

void
SearchDialog::onListChange(const game::ref::UserList& list)
{
    // ex WSearchDialog::onResultChange (part)
    bool oldEmpty = (m_refList.getNumItems() == 0);
    m_refList.setContent(list);
    bool newEmpty = (m_refList.getNumItems() == 0);

    // Update disabled-state
    m_refList.setState(ui::Widget::DisabledState, newEmpty);
    m_btnGoto.setState(ui::Widget::DisabledState, newEmpty);
    m_btnMark.setState(ui::Widget::DisabledState, newEmpty);
    m_btnGlobal.setState(ui::Widget::DisabledState, newEmpty);

    // Update keyboard focus and position
    // List transitions to empty > nonempty when a search result applies.
    // Do not change anything on a nonempty > nonempty transition, e.g. data change.
    if (oldEmpty && !newEmpty) {
        m_refList.requestFocus();

        size_t newPos = 0;
        if (findObject(list, m_currentObject, newPos)) {
            m_refList.setCurrentItem(newPos);
        }
    }
}

void
SearchDialog::onOptionClick(int id)
{
    switch (id) {
     case Option_SearchObjects:
        editSearchObjects();
        break;
     case Option_MatchType:
        editMatchType();
        break;
     case Option_PlayedOnly:
        m_query.setPlayedOnly(!m_query.getPlayedOnly());
        break;
    }
    setValues();
    m_input.requestFocus();
}

void
SearchDialog::onGoto()
{
    if (m_isSubDialog) {
        m_loop.stop(Stop_Normal);
    } else {
        executeGoToReferenceWait("(Search Result)", m_refList.getCurrentReference(), ShowUnit);
    }
}

void
SearchDialog::onMark()
{
    // ex WSearchDialog::doSelectionCommands
    enum { Mark, MarkOnly, Unmark };

    afl::string::Translator& tx = translator();
    ui::widgets::StringListbox list(root().provider(), root().colorScheme());
    list.addItem(Mark,     tx("Mark found objects"));
    list.addItem(MarkOnly, tx("Mark only found objects"));
    list.addItem(Unmark,   tx("Unmark found objects"));

    ui::EventLoop loop(root());
    if (!ui::widgets::MenuFrame(ui::layout::VBox::instance0, root(), loop).doMenu(list, m_btnMark.getExtent().getBottomLeft())) {
        return;
    }

    // Create a short-lived SelectionProxy; we don't need any callbacks that would necessitate a long-lived one.
    SelectionProxy proxy(interface().gameSender(), root().engine().dispatcher());

    // Commands
    const int32_t key = list.getCurrentKey().orElse(0);
    switch (key) {
     case Mark:
        proxy.markList(game::map::Selections::CurrentLayer, m_result, true);
        break;
     case MarkOnly:
        proxy.clearLayer(game::map::Selections::CurrentLayer);
        proxy.markList(game::map::Selections::CurrentLayer, m_result, true);
        break;
     case Unmark:
        proxy.markList(game::map::Selections::CurrentLayer, m_result, false);
        break;
    }
}

void
SearchDialog::onReturn()
{
    if (m_refList.hasState(ui::Widget::FocusedState)) {
        onGoto();
    } else {
        onSearch();
    }
}

void
SearchDialog::onDown()
{
    if (m_input.hasState(ui::Widget::FocusedState)) {
        onHistory();
    }
}

void
SearchDialog::onHistory()
{
    // ex WSearchDialog::onQueryDropdown
    m_input.requestFocus();

    String_t value = m_input.getText();
    String_t flags;
    client::Downlink link(interface());
    if (client::widgets::doExpressionListPopup(root(), link, m_exProxy, m_btnHistory.getExtent().getBottomLeft(), value, flags)) {
        // User has selected an item. Parse it.
        SearchQuery::SearchObjects_t obj;
        SearchQuery::MatchType type = SearchQuery::MatchTrue;

        if (!flags.empty()) {
            for (size_t i = 0; i < flags.size(); ++i) {
                switch (flags[i]) {
                 case 'S': case 's':
                    obj += SearchQuery::SearchShips;
                    break;
                 case 'P': case 'p':
                    obj += SearchQuery::SearchPlanets;
                    break;
                 case 'B': case 'b':
                    obj += SearchQuery::SearchBases;
                    break;
                 case '!':
                    type = SearchQuery::MatchFalse;
                    break;
                }
            }
        } else {
            obj = SearchQuery::SearchObjects_t()
                + SearchQuery::SearchShips
                + SearchQuery::SearchPlanets
                + SearchQuery::SearchBases
                + SearchQuery::SearchUfos
                + SearchQuery::SearchOthers;
        }

        // If it specifies an object type, set that
        if (!obj.empty()) {
            m_query.setSearchObjects(obj);
        }

        // Set type and query
        m_query.setMatchType(type);
        m_input.setText(value);
        setValues();
    }
}

void
SearchDialog::onGoToShip()
{
    openControlScreen(game::Reference::Ship, client::ScreenHistory::Ship, client::si::OutputState::ShipScreen);
}

void
SearchDialog::onGoToPlanet()
{
    openControlScreen(game::Reference::Planet, client::ScreenHistory::Planet, client::si::OutputState::PlanetScreen);
}

void
SearchDialog::onGoToBase()
{
    openControlScreen(game::Reference::Planet, client::ScreenHistory::Starbase, client::si::OutputState::BaseScreen);
}

void
SearchDialog::onGoToMap()
{
    executeGoToReferenceWait("(Search)", m_refList.getCurrentReference(), ShowOnMap);
}

void
SearchDialog::onGoToHistory()
{
    openControlScreen(game::Reference::Planet, client::ScreenHistory::HistoryShip, client::si::OutputState::HistoryScreen);
}

void
SearchDialog::onSelectionManager()
{
    client::si::OutputState out;
    afl::base::Optional<SearchQuery> q = client::dialogs::doSelectionManagerFromSearch(interface(), out);
    if (out.isValid()) {
        // Outbound process.
        // Selection manager will not generate this together with a search query.
        // but if it did, handling the outbound process would be more important than handling the query.
        handleStateChange(out.getProcess(), out.getTarget());
    } else if (q.get(m_query)) {
        // Search query (i.e. F7)
        setValues();
        m_input.setText(m_query.getQuery());
        m_input.requestFocus();
        onSearch();
    } else {
        // No change (i.e. ESC)
    }
}

void
SearchDialog::onPreviousSelectionLayer()
{
    executeCommandWait("CC$PreviousSelection", false, "(Search)");
}

void
SearchDialog::onNextSelectionLayer()
{
    executeCommandWait("CC$NextSelection", false, "(Search)");
}

void
SearchDialog::editSearchObjects()
{
    SearchQuery::SearchObjects_t objs = m_query.getSearchObjects();
    SearchObjectDialog(root(), translator()).run(objs);
    m_query.setSearchObjects(objs);
}

void
SearchDialog::editMatchType()
{
    afl::string::Translator& tx = translator();
    ui::widgets::StringListbox box(root().provider(), root().colorScheme());
    for (size_t i = 0; i < countof(MATCH_TYPES); ++i) {
        box.addItem(int32_t(i), afl::string::Format("%d - %s", i+1, tx(MATCH_TYPES[i])));
    }
    box.setCurrentKey(m_query.getMatchType());
    if (ui::widgets::doStandardDialog(tx("Search Object"), tx("Search type"), box, true, root(), tx)) {
        int32_t k;
        if (box.getCurrentKey().get(k)) {
            m_query.setMatchType(SearchQuery::MatchType(k));
        }
    }
}

void
SearchDialog::setValues()
{
    afl::string::Translator& tx = translator();
    m_options.findItem(Option_MatchType)
        .setValue(tx(createStringTable(MATCH_TYPES)(m_query.getMatchType())));
    m_options.findItem(Option_SearchObjects)
        .setValue(SearchQuery::formatSearchObjects(m_query.getSearchObjects(), tx));
    m_options.findItem(Option_PlayedOnly)
        .setValue(tx(createStringTable(NO_YES)(m_query.getPlayedOnly())));
}

void
SearchDialog::openControlScreen(game::Reference::Type refType,
                                client::ScreenHistory::Type histType,
                                client::si::OutputState::Target target)
{
    game::Reference ref = m_refList.getCurrentReference();
    if (m_refList.hasState(ui::Widget::FocusedState) && ref.getType() == refType) {
        client::Downlink link(interface());
        client::proxy::ScreenHistoryProxy proxy(interface().gameSender());
        client::ScreenHistory::Reference histRef(histType, ref.getId(), 0);
        if (proxy.validateReference(link, histRef)) {
            proxy.activateReference(link, histRef);
            handleStateChange(client::si::RequestLink2(), target);
        }
    }
}

void
SearchDialog::setListContent(const game::ref::List& list)
{
    class Init : public game::proxy::ReferenceListProxy::Initializer_t {
     public:
        Init(const game::ref::List& list)
            : m_list(list)
            { }
        void call(game::Session& /*session*/, game::ref::ListObserver& obs)
            { obs.setList(m_list); }
     private:
        game::ref::List m_list;
    };
    m_refListProxy.setContentNew(std::auto_ptr<game::proxy::ReferenceListProxy::Initializer_t>(new Init(list)));
}


/*
 *  Entry Points
 */

void
client::dialogs::doSearchDialog(const game::SearchQuery& initialQuery,
                                game::Reference currentObject,
                                bool immediate,
                                client::si::UserSide& iface,
                                client::si::OutputState& out)
{
    Downlink link(iface);
    game::proxy::ConfigurationProxy config(iface.gameSender());
    game::ref::List list;

    // Execute dialog
    // Must take the object off the stack to remove its Control before we branch to global actions.
    int code;
    {
        SearchDialog dlg(initialQuery, currentObject, list, iface, config.getNumberFormatter(link), out, false);
        code = dlg.run(immediate);
    }

    // Optionally, branch to global actions.
    // We cannot invoke the dialog directly, because we need to run a script to prepare the input.
    // It's easiest to have the script call the dialog.
    if (code == Stop_Global) {
        /* A Control to receive callbacks from the process created by TransferTask
           (and, eventually, the Global Actions dialog called from it).
           As of 20220909, this is the same implementation as in ProcessListDialog;
           keep it separate for now in case we change it later.
           In particular, handlePopupConsole() should probably be implemented as defaultHandlePopupConsole(). */
        class ExtraControl : public client::si::Control {
         public:
            ExtraControl(client::si::UserSide& iface, ui::Root& root, client::si::OutputState& out)
                : Control(iface), m_outputState(out), m_loop(root)
                { }
            virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
                { dialogHandleStateChange(link, target, m_outputState, m_loop, 0); }
            virtual void handleEndDialog(client::si::RequestLink2 link, int /*code*/)
                { interface().continueProcess(link); }
            virtual void handlePopupConsole(client::si::RequestLink2 link)
                { interface().continueProcess(link); }
            virtual void handleScanKeyboardMode(client::si::RequestLink2 link)
                { defaultHandleScanKeyboardMode(link); }
            virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
                { defaultHandleSetView(link, name, withKeymap); }
            virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
                { defaultHandleUseKeymap(link, name, prefix); }
            virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text)
                { defaultHandleOverlayMessage(link, text); }
            virtual afl::base::Optional<game::Id_t> getFocusedObjectId(game::Reference::Type type) const
                { return defaultGetFocusedObjectId(type); }
            virtual game::interface::ContextProvider* createContextProvider()
                { return 0; }
         private:
            client::si::OutputState& m_outputState;
            ui::EventLoop m_loop;
        };

        /* Task to invoke "UI.GlobalActions <TheSearchResult>" */
        class TransferTask : public client::si::ScriptTask {
         public:
            TransferTask(const game::ref::List& list)
                : m_listData(*new game::interface::ReferenceListContext::Data())
                { m_listData->list = list; }
            virtual void execute(uint32_t pgid, game::Session& session)
                {
                    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
                    game::interface::ReferenceListContext ctx(m_listData, session);
                    bco->addPushLiteral(&ctx);
                    bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("UI.GLOBALACTIONS"));
                    bco->addInstruction(Opcode::maIndirect, Opcode::miIMCall, 1);

                    interpreter::Process& proc = session.processList().create(session.world(), "(Global Actions)");
                    proc.pushFrame(bco, false);

                    session.processList().resumeProcess(proc, pgid);
                }

         private:
            afl::base::Ref<game::interface::ReferenceListContext::Data> m_listData;
        };

        /* Invoke the task */
        ExtraControl(iface, iface.root(), out).executeTaskWait(std::auto_ptr<client::si::ScriptTask>(new TransferTask(list)));
    }
}

void
client::dialogs::doSearchSubDialog(game::ref::List& list,
                                   client::si::UserSide& iface,
                                   client::si::OutputState& out)
{
    Downlink link(iface);
    game::proxy::ConfigurationProxy config(iface.gameSender());

    // Execute dialog
    // - no focused object, because user-perceived location is the dialog we are a sub-dialog to, not an object
    // - load query ourselves so caller doesn't have to
    SearchDialog dlg(game::SearchQuery(), game::Reference(), list, iface, config.getNumberFormatter(link), out, true);
    dlg.loadQuery(link);
    dlg.run(list.size() != 0);
}
