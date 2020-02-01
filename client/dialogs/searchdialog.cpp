/**
  *  \file client/dialogs/searchdialog.cpp
  *
  *  Missing features:
  *    - focus on current object
  *    - LRU / predef
  *    - selection handling
  *    - global actions
  */

#include "client/dialogs/searchdialog.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/deleter.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/proxy/configurationproxy.hpp"
#include "client/proxy/referencelistproxy.hpp"
#include "client/proxy/searchproxy.hpp"
#include "client/si/control.hpp"
#include "client/widgets/referencelistbox.hpp"
#include "game/interface/referencecontext.hpp"
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
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/numberformatter.hpp"
#include "util/translation.hpp"

using game::SearchQuery;
using afl::functional::createStringTable;
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


    /*
     *  Search Object Selection Dialog
     */

    class SearchObjectDialog {
     public:
        // ex WSearchObjectSelector (sort-of)
        SearchObjectDialog(ui::Root& root, afl::string::Translator& tx)
            : m_root(root),
              m_translator(tx),
              m_buttons(root)
            { }

        void run(SearchQuery::SearchObjects_t& objs)
            {
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(m_translator("Search Object"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
                ui::widgets::FocusIterator& it = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab + ui::widgets::FocusIterator::Vertical));
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
                m_buttons.ok().setState(ui::Widget::DisabledState, getStatus().empty());
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
        afl::base::Observable<int> m_values[NUM_SEARCH_OBJECT];
        ui::widgets::StandardDialogButtons m_buttons;
    };


    /*
     *  Search Dialog
     */

    class SearchDialog : public client::si::Control {
     public:
        SearchDialog(const SearchQuery& initialQuery, client::si::UserSide& iface, ui::Root& root, afl::string::Translator& tx, util::NumberFormatter fmt, client::si::OutputState& out);

        void run(bool immediate);

        virtual void handleStateChange(client::si::UserSide& ui, client::si::RequestLink2 link, client::si::OutputState::Target target);
        virtual void handleEndDialog(client::si::UserSide& ui, client::si::RequestLink2 link, int code);
        virtual void handlePopupConsole(client::si::UserSide& ui, client::si::RequestLink2 link);
        virtual void handleSetViewRequest(client::si::UserSide& ui, client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual client::si::ContextProvider* createContextProvider();

     private:
        enum State {
            Idle,
            Searching,
            Sorting
        };

        // References
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::NumberFormatter m_format;
        client::si::OutputState& m_outputState;

        // Proxies
        client::proxy::ReferenceListProxy m_refListProxy;
        client::proxy::SearchProxy m_searchProxy;

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
        client::widgets::ReferenceListbox m_refList;

        // Status
        SearchQuery m_query;

        void onSearch();
        void onSuccess(const game::ref::List& list);
        void onError(String_t err);
        void onListChange(const game::ref::UserList& list);
        void onOptionClick(int id);
        void onGoto();

        void onReturn();
        void onDown();

        void editSearchObjects();
        void editMatchType();
        void setValues();

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
SearchDialog::SearchDialog(const SearchQuery& initialQuery, client::si::UserSide& iface, ui::Root& root, afl::string::Translator& tx, util::NumberFormatter fmt, client::si::OutputState& out)
    : Control(iface, root, tx),
      m_root(root),
      m_translator(tx),
      m_format(fmt),
      m_outputState(out),
      m_refListProxy(root, iface.gameSender(), tx),
      m_searchProxy(root, iface.gameSender()),
      m_loop(root),
      m_input(1000, 30, root),
      m_options(0, 0, root),
      m_resultStatus(String_t(), util::SkinColor::Static, gfx::FontRequest().addSize(1), root.provider(), 0),
      m_btnSearch(tx("Search!"), 0, root),
      m_btnGoto(tx("Go to"), 0, root),
      m_btnClose(tx("Close"), util::Key_Escape, root),
      m_btnMark(tx("Mark..."), 'm', root),
      m_btnGlobal(tx("Global..."), 'g', root),
      m_btnHelp(tx("Help"), 'h', root),
      m_refList(root),
      m_query(initialQuery)
{
    // ex WSearchDialog::WSearchDialog
    m_searchProxy.sig_success.add(this, &SearchDialog::onSuccess);
    m_searchProxy.sig_error.add(this, &SearchDialog::onError);
    m_refListProxy.sig_listChange.add(this, &SearchDialog::onListChange);
    m_refListProxy.setConfigurationSelection(game::ref::SEARCH);
    m_options.sig_click.add(this, &SearchDialog::onOptionClick);
    m_btnSearch.sig_fire.add(this, &SearchDialog::onSearch);
}

void
SearchDialog::run(bool immediate)
{
    // ex WSearchDialog::init (sort-of)
    // VBox
    //   OptionGrid
    //   FrameGroup > InputLine          // tbd: history drop-down
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

    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Search Object"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    // Options
    // FIXME: addPossibleValues for objects
    m_options.addItem(Option_SearchObjects, 'o', m_translator("Objects"))
        .addPossibleValues(SearchObjectLabel(m_translator));
    m_options.addItem(Option_MatchType,     't', m_translator("Search type"))
        .addPossibleValues(createStringTable(MATCH_TYPES).map(m_translator));
    m_options.addItem(Option_PlayedOnly,    'p', m_translator("Played objects only"))
        .addPossibleValues(createStringTable(NO_YES).map(m_translator));
    win.add(m_options);

    // Input
    win.add(FrameGroup::wrapWidget(del, m_root.colorScheme(), FrameGroup::LoweredFrame, m_input));
    m_input.setFont(gfx::FontRequest().addSize(1));

    // "Search!" button
    ui::Group& g2 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    m_resultStatus.setIsFlexible(true);
    g2.add(m_resultStatus);
    g2.add(m_btnSearch);
    win.add(g2);

    // Result list
    win.add(FrameGroup::wrapWidget(del, m_root.colorScheme(), FrameGroup::LoweredFrame, m_refList));
    m_refList.setNumLines(20);

    // Lower buttons
    ui::Group& g4 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g4.add(m_btnGoto);
    g4.add(m_btnClose);
    // FIXME: g4.add(m_btnMark);
    // FIXME: g4.add(m_btnGlobal);
    g4.add(del.addNew(new ui::Spacer()));
    g4.add(m_btnHelp);
    win.add(g4);
    m_btnGoto.sig_fire.add(this, &SearchDialog::onGoto);
    m_btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));

    // Admin
    ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
    disp.add(util::Key_Return, this, &SearchDialog::onReturn);
    disp.add(util::Key_Down,   this, &SearchDialog::onDown);
    disp.add(util::Key_F7,     &m_input, &ui::widgets::InputLine::requestFocus);
    win.add(disp);

    ui::widgets::FocusIterator& it = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab));
    it.add(m_input);
    it.add(m_refList);
    win.add(it);

    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

    // Setup
    setValues();
    m_input.setText(m_query.getQuery());
    m_input.requestFocus();
    onListChange(game::ref::UserList());

    // Run
    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    if (immediate) {
        onSearch();
    }
    m_loop.run();
}

void
SearchDialog::handleStateChange(client::si::UserSide& ui, client::si::RequestLink2 link, client::si::OutputState::Target target)
{
    using client::si::OutputState;
    switch (target) {
     case OutputState::NoChange:
        ui.continueProcess(link);
        break;

     case OutputState::PlayerScreen:
     case OutputState::ExitProgram:
     case OutputState::ExitGame:
     case OutputState::ShipScreen:
     case OutputState::PlanetScreen:
     case OutputState::BaseScreen:
     case OutputState::ShipTaskScreen:
     case OutputState::PlanetTaskScreen:
     case OutputState::BaseTaskScreen:
     case OutputState::Starchart:
        ui.detachProcess(link);
        m_outputState.set(link, target);
        m_loop.stop(0);
        break;
    }
}

void
SearchDialog::handleEndDialog(client::si::UserSide& ui, client::si::RequestLink2 link, int /*code*/)
{
    ui.detachProcess(link);
    m_outputState.set(link, client::si::OutputState::NoChange);
    m_loop.stop(0);
}

void
SearchDialog::handlePopupConsole(client::si::UserSide& ui, client::si::RequestLink2 link)
{
    defaultHandlePopupConsole(ui, link);
}

void
SearchDialog::handleSetViewRequest(client::si::UserSide& ui, client::si::RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetViewRequest(ui, link, name, withKeymap);
}

client::si::ContextProvider*
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
    m_searchProxy.search(m_query);
}

void
SearchDialog::onSuccess(const game::ref::List& list)
{
    // ex WSearchDialog::onResultChange (part)
    m_resultStatus.setText(afl::string::Format(m_translator("%d result%!1{s%}"),
                                               m_format.formatNumber(static_cast<int32_t>(list.size()))));

    if (list.size() == 0) {
        // Nothing found
        // FIXME: give advice
        ui::dialogs::MessageBox(m_translator("Your query didn't match any object."),
                                m_translator("Search Object"),
                                m_root).doOkDialog();
    } else {
        // Set list content. This will answer with onListChange.
        setListContent(list);
    }
}

void
SearchDialog::onError(String_t err)
{
    ui::dialogs::MessageBox(err, m_translator("Search Object"), m_root).doOkDialog();
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

    // Update keyboard focus
    if (oldEmpty && !newEmpty) {
        m_refList.requestFocus();
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
}

void
SearchDialog::onGoto()
{
    class ReferenceTask : public client::si::ScriptTask {
     public:
        ReferenceTask(game::Reference ref)
            : m_ref(ref)
            { }
        virtual void execute(uint32_t pgid, game::Session& session)
            {
                // Create BCO
                interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
                game::interface::ReferenceContext ctx(m_ref, session);
                bco->setName("(Search Result)");
                bco->setIsProcedure(true);
                bco->addPushLiteral(&ctx);
                bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sNamedShared, bco->addName("UI.GOTOREFERENCE"));
                bco->addInstruction(interpreter::Opcode::maIndirect, interpreter::Opcode::miIMCall, 1);

                // Create process
                interpreter::ProcessList& processList = session.world().processList();
                interpreter::Process& proc = processList.create(session.world(), "(Search Result)");
                proc.pushFrame(bco, false);
                processList.resumeProcess(proc, pgid);
            }
     private:
        game::Reference m_ref;
    };

    game::Reference ref = m_refList.getCurrentReference();
    if (ref.isSet()) {
        std::auto_ptr<client::si::ScriptTask> t(new ReferenceTask(ref));
        executeTaskWait(t);
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
    // if (event.key.code == SDLK_DOWN && query_input->hasState(st_Focused)) {
    //     onQueryDropdown();
    //     return true;
    // }
}

void
SearchDialog::editSearchObjects()
{
    SearchQuery::SearchObjects_t objs = m_query.getSearchObjects();
    SearchObjectDialog(m_root, m_translator).run(objs);
    m_query.setSearchObjects(objs);
}

void
SearchDialog::editMatchType()
{
    ui::widgets::StringListbox box(m_root.provider(), m_root.colorScheme());
    for (size_t i = 0; i < countof(MATCH_TYPES); ++i) {
        box.addItem(int32_t(i), afl::string::Format("%d - %s", i+1, m_translator(MATCH_TYPES[i])));
    }
    box.setCurrentKey(m_query.getMatchType());
    if (ui::widgets::doStandardDialog(m_translator("Search Object"), m_translator("Search type"), box, true, m_root)) {
        int32_t k;
        if (box.getCurrentKey(k)) {
            m_query.setMatchType(SearchQuery::MatchType(k));
        }
    }
}

void
SearchDialog::setValues()
{
    m_options.findItem(Option_MatchType)
        .setValue(m_translator(createStringTable(MATCH_TYPES)(m_query.getMatchType())));
    m_options.findItem(Option_SearchObjects)
        .setValue(SearchQuery::formatSearchObjects(m_query.getSearchObjects(), m_translator));
    m_options.findItem(Option_PlayedOnly)
        .setValue(m_translator(createStringTable(NO_YES)(m_query.getPlayedOnly())));
}

void
SearchDialog::setListContent(const game::ref::List& list)
{
    class Init : public client::proxy::ReferenceListProxy::Initializer_t {
     public:
        Init(const game::ref::List& list)
            : m_list(list)
            { }
        void call(game::Session& /*session*/, game::ref::ListObserver& obs)
            { obs.setList(m_list); }
        Init* clone() const
            { return new Init(m_list); }
     private:
        game::ref::List m_list;
    };
    m_refListProxy.setContentNew(std::auto_ptr<client::proxy::ReferenceListProxy::Initializer_t>(new Init(list)));
}

void
client::dialogs::doSearchDialog(const game::SearchQuery& initialQuery,
                                bool immediate,
                                client::si::UserSide& iface,
                                ui::Root& root,
                                afl::string::Translator& tx,
                                client::si::OutputState& out)
{
    Downlink link(root);
    client::proxy::ConfigurationProxy config(iface.gameSender());

    SearchDialog dlg(initialQuery, iface, root, tx, config.getNumberFormatter(link), out);
    dlg.run(immediate);
}
