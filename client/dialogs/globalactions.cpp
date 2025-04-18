/**
  *  \file client/dialogs/globalactions.cpp
  *  \brief Global Actions dialog
  */

#include "client/dialogs/globalactions.hpp"
#include "client/dialogs/searchdialog.hpp"
#include "client/downlink.hpp"
#include "client/si/control.hpp"
#include "client/si/userside.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/interface/globalactioncontext.hpp"
#include "game/interface/globalactions.hpp"
#include "game/proxy/globalactionproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/treelistbox.hpp"
#include "ui/window.hpp"

using client::si::OutputState;
using client::si::RequestLink2;
using client::si::ScriptTask;
using game::interface::GlobalActionContext;
using game::interface::GlobalActions;
using interpreter::VariableReference;
using ui::widgets::TreeListbox;

namespace {
    /* OptionGrid Ids */
    enum {
        IdNumericFC,
        IdSpecialFC,
        IdSearchResult,
        IdMarked,
        IdShips,
        IdPlanets,
        IdLocks
    };

    /*
     *  Dialog class
     */
    class Dialog : public client::si::Control {
     public:
        Dialog(client::si::UserSide& us, OutputState& outputState, game::ref::List& searchResult, const VariableReference& ref);

        void init(game::proxy::WaitIndicator& ind);
        void run();

        // Control:
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
        virtual afl::base::Optional<game::Id_t> getFocusedObjectId(game::Reference::Type type) const
            { return defaultGetFocusedObjectId(type); }
        virtual game::interface::ContextProvider* createContextProvider()
            { return 0; }

     private:
        // Infrastructure
        OutputState& m_outputState;
        ui::EventLoop m_loop;
        const VariableReference m_ref;

        // Widgets
        TreeListbox m_tree;
        ui::widgets::OptionGrid m_grid;

        // Dialog status
        game::ref::List& m_searchResult;
        GlobalActions::Flags_t m_flags;
        bool m_useSearchResult;

        // Event handlers
        void onOK();
        void onSearch();
        void onOptionClick(int which);

        // Helpers
        void toggleOption(GlobalActions::Flag flag);
        void addOption(int id, util::Key_t key, String_t label);
        void renderOptions();
        void renderOption(int id, bool value, bool enabled);
        bool hasSearchResult() const;
        void executeGlobalAction(size_t actionId);
        void executeListAction(size_t actionId);
    };
}

Dialog::Dialog(client::si::UserSide& us, OutputState& outputState, game::ref::List& searchResult, const VariableReference& ref)
    : Control(us),
      m_outputState(outputState),
      m_loop(us.root()),
      m_ref(ref),
      m_tree(us.root(), 15, 20*us.root().provider().getFont(gfx::FontRequest())->getEmWidth()),
      m_grid(0, 0, us.root()),
      m_searchResult(searchResult),
      m_flags(GlobalActions::Flags_t()
              + GlobalActions::ExcludeNumericFriendlyCodes
              + GlobalActions::ExcludeSpecialFriendlyCodes),
      m_useSearchResult(hasSearchResult())
{
    // ex WGlobalActionParameter::WGlobalActionParameter (sort-of)
    afl::string::Translator& tx = translator();
    addOption(IdNumericFC,    'n', tx("Exclude numerical FCodes"));
    addOption(IdSpecialFC,    's', tx("Exclude special FCodes"));
    addOption(IdMarked,       'm', tx("Marked objects only"));
    addOption(IdShips,        '1', tx("Include ships"));
    addOption(IdPlanets,      '2', tx("Include planets"));
    addOption(IdLocks,        'l', tx("Override locks"));
    addOption(IdSearchResult, 'r', tx("Objects from search result only"));
    renderOptions();

    m_grid.sig_click.add(this, &Dialog::onOptionClick);
    m_tree.sig_itemDoubleClick.add(this, &Dialog::onOK);
}

void
Dialog::init(game::proxy::WaitIndicator& ind)
{
    util::TreeList list;
    game::proxy::GlobalActionProxy(interface().gameSender()).getActions(ind, list, m_ref);
    m_tree.addTree(0, list, util::TreeList::root);
}

void
Dialog::run()
{
    // ex WGlobalActionDialog::init()
    afl::string::Translator& tx = translator();

    // VBox
    //   HBox
    //     VBox [TreeListbox, Spacer]
    //     VBox
    //       OptionGrid
    //       HBox [Text, Button]
    //       Spacer
    //   StandardDialogButtons
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(tx("Global Actions"), root().provider(), root().colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));

    // Left side
    ui::Group& g1 = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    g1.add(del.addNew(new ui::widgets::ScrollbarContainer(m_tree, root())));
    g1.add(del.addNew(new ui::Spacer()));
    g.add(g1);

    // Right side
    ui::widgets::Button& btnSearch = del.addNew(new ui::widgets::Button(tx("F7 - Search"), util::Key_F7, root()));
    ui::Group& g2 = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::Group& g22 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g2.add(m_grid);
    g2.add(g22);
    g2.add(del.addNew(new ui::Spacer()));
    g22.add(del.addNew(new ui::Spacer()));
    g22.add(btnSearch);
    g.add(g2);
    win.add(g);

    // Bottom
    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(root(), tx));
    btn.ok().setText(tx("Execute"));
    btn.cancel().setText(tx("Close"));
    win.add(btn);

    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(root(), tx, interface().gameSender(), "pcc2:globact"));
    btn.addHelp(help);
    win.add(help);

    win.add(del.addNew(new ui::widgets::Quit(root(), m_loop)));
    win.pack();

    // Eventy
    btn.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
    btn.ok().sig_fire.add(this, &Dialog::onOK);
    btnSearch.sig_fire.add(this, &Dialog::onSearch);

    root().centerWidget(win);
    root().add(win);
    m_loop.run();
}

/* Event handler: OK button (execute action; if not at an action, toggle node) */
void
Dialog::onOK()
{
    // ex WGlobalActionDialog::onExec
    TreeListbox::Node* node = m_tree.getCurrentNode();
    int32_t id = m_tree.getIdFromNode(node);
    if (id != 0) {
        if (m_flags.contains(GlobalActions::ExcludeShips) && m_flags.contains(GlobalActions::ExcludePlanets)) {
            afl::string::Translator& tx = translator();
            ui::dialogs::MessageBox(tx("Please select the \"Ships\" and/or \"Planets\" option before executing a global action."),
                                    tx("Global Actions"),
                                    root())
                .doOkDialog(tx);
        } else {
            size_t actionId = static_cast<size_t>(id-1);
            if (m_useSearchResult) {
                executeListAction(actionId);
            } else {
                executeGlobalAction(actionId);
            }
        }
    } else {
        if (m_tree.hasChildren(node)) {
            m_tree.toggleNode(node);
        }
    }
}

/* Event handler: Search (update search result) */
void
Dialog::onSearch()
{
    client::dialogs::doSearchSubDialog(m_searchResult, interface(), m_outputState);

    // The search dialog shall not perform a state change.
    // However, using specially-configured functions, the search expression can do that.
    if (m_outputState.isValid()) {
        m_loop.stop(1);
    } else {
        m_useSearchResult = hasSearchResult();
        renderOptions();
    }
}

/* Event handler: option toggle */
void
Dialog::onOptionClick(int which)
{
    // ex WGlobalActionParameterView::handleEvent (sort-of)
    switch (which) {
     case IdNumericFC:
        toggleOption(GlobalActions::ExcludeNumericFriendlyCodes);
        break;
     case IdSpecialFC:
        toggleOption(GlobalActions::ExcludeSpecialFriendlyCodes);
        break;
     case IdSearchResult:
        m_useSearchResult = !m_useSearchResult && hasSearchResult();
        renderOptions();
        break;
     case IdMarked:
        toggleOption(GlobalActions::ExcludeUnmarkedObjects);
        break;
     case IdShips:
        toggleOption(GlobalActions::ExcludeShips);
        break;
     case IdPlanets:
        toggleOption(GlobalActions::ExcludePlanets);
        break;
     case IdLocks:
        toggleOption(GlobalActions::OverrideLocks);
        break;
    }
}

/* Toggle a single flag option */
void
Dialog::toggleOption(GlobalActions::Flag flag)
{
    // ex WGlobalActionParameter::toggleFlag
    m_flags ^= flag;
    renderOptions();
}

/* Add an option to the OptionGrid. Perform additional common initialisation for the item. */
void
Dialog::addOption(int id, util::Key_t key, String_t label)
{
    afl::string::Translator& tx = translator();
    m_grid.addItem(id, key, label)
        .addPossibleValue(tx("yes"))
        .addPossibleValue(tx("no"));
}

/* Render all options */
void
Dialog::renderOptions()
{
    // ex WGlobalActionParameterView::drawData (sort-of)
    renderOption(IdNumericFC,    m_flags.contains(GlobalActions::ExcludeNumericFriendlyCodes), true);
    renderOption(IdSpecialFC,    m_flags.contains(GlobalActions::ExcludeSpecialFriendlyCodes), true);
    renderOption(IdSearchResult, m_useSearchResult,                                            hasSearchResult());
    renderOption(IdMarked,       m_flags.contains(GlobalActions::ExcludeUnmarkedObjects),      true);
    renderOption(IdShips,        !m_flags.contains(GlobalActions::ExcludeShips),               true);
    renderOption(IdPlanets,      !m_flags.contains(GlobalActions::ExcludePlanets),             true);
    renderOption(IdLocks,        m_flags.contains(GlobalActions::OverrideLocks),               true);
}

/* Render a single option */
void
Dialog::renderOption(int id, bool value, bool enabled)
{
    afl::string::Translator& tx = translator();
    m_grid.findItem(id)
        .setValue(value ? tx("yes") : tx("no"))
        .setEnabled(enabled);
}

/* Check presence of a search result */
bool
Dialog::hasSearchResult() const
{
    return m_searchResult.size() != 0;
}

/* Execute action globally (compileGlobalAction) */
void
Dialog::executeGlobalAction(size_t actionId)
{
    class Task : public ScriptTask {
     public:
        Task(size_t actionId, GlobalActions::Flags_t flags, const VariableReference& ref)
            : m_actionId(actionId), m_flags(flags), m_ref(ref)
            { }
        virtual void execute(uint32_t pgid, game::Session& session)
            {
                std::auto_ptr<afl::data::Value> value(m_ref.get(session.processList()));
                if (GlobalActionContext* ctx = dynamic_cast<GlobalActionContext*>(value.get())) {
                    interpreter::ProcessList& processList = session.processList();
                    interpreter::Process& proc = processList.create(session.world(), "(Global Actions)");
                    const GlobalActions::Action* a = ctx->data()->actions.getActionByIndex(m_actionId);
                    proc.pushFrame(ctx->data()->actions.compileGlobalAction(a, session.world(), m_flags), false);
                    processList.resumeProcess(proc, pgid);
                }
            }
     private:
        size_t m_actionId;
        GlobalActions::Flags_t m_flags;
        const VariableReference m_ref;
    };
    executeTaskWait(std::auto_ptr<ScriptTask>(new Task(actionId, m_flags, m_ref)));
}

/* Execute action on search result (compileListAction) */
void
Dialog::executeListAction(size_t actionId)
{
    class Task : public ScriptTask {
     public:
        Task(size_t actionId, GlobalActions::Flags_t flags, const game::ref::List& list, const VariableReference ref)
            : m_actionId(actionId), m_flags(flags), m_list(list), m_ref(ref)
            { }
        virtual void execute(uint32_t pgid, game::Session& session)
            {
                std::auto_ptr<afl::data::Value> value(m_ref.get(session.processList()));
                if (GlobalActionContext* ctx = dynamic_cast<GlobalActionContext*>(value.get())) {
                    interpreter::ProcessList& processList = session.processList();
                    interpreter::Process& proc = processList.create(session.world(), "(Global Actions)");
                    const GlobalActions::Action* a = ctx->data()->actions.getActionByIndex(m_actionId);
                    proc.pushFrame(ctx->data()->actions.compileListAction(a, m_list, session.world(), m_flags), false);
                    processList.resumeProcess(proc, pgid);
                }
            }
     private:
        size_t m_actionId;
        GlobalActions::Flags_t m_flags;
        game::ref::List m_list;
        const VariableReference m_ref;
    };
    executeTaskWait(std::auto_ptr<ScriptTask>(new Task(actionId, m_flags, m_searchResult, m_ref)));
}


/*
 *  Main Entry Point
 */

void
client::dialogs::doGlobalActions(client::si::UserSide& us,
                                 client::si::OutputState& outputState,
                                 game::ref::List& searchResult,
                                 interpreter::VariableReference ref)
{
    // ex doGlobalActions, globact.pas:NGlobalActions
    Dialog dlg(us, outputState, searchResult, ref);
    Downlink link(us);
    dlg.init(link);
    dlg.run();
}
