/**
  *  \file client/dialogs/processlistdialog.cpp
  *  \brief Process List Dialog
  */

#include "client/dialogs/processlistdialog.hpp"
#include "afl/base/observable.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/notifications.hpp"
#include "client/downlink.hpp"
#include "client/si/control.hpp"
#include "client/si/stringlistdialogwidget.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/mutexlistproxy.hpp"
#include "game/proxy/processlistproxy.hpp"
#include "gfx/context.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/draw.hpp"
#include "ui/group.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

using game::Reference;
using game::interface::ProcessListEditor;
using game::proxy::MutexListProxy;
using game::proxy::ProcessListProxy;
using ui::dialogs::MessageBox;

namespace {

    void addButton(ui::Root& root, afl::base::Deleter& del, ui::Group& g, ui::Widget& keyHandler, String_t label, util::Key_t key)
    {
        ui::widgets::Button& btn = del.addNew(new ui::widgets::Button(label, key, root));
        btn.setFont(gfx::FontRequest());
        btn.dispatchKeyTo(keyHandler);
        g.add(btn);
    }

    void addText(ui::Root& root, afl::base::Deleter& del, ui::Group& g, String_t text)
    {
        g.add(del.addNew(new ui::widgets::StaticText(text, util::SkinColor::Static, gfx::FontRequest(), root.provider())));
    }

    void performChanges(client::Downlink& link,
                        ProcessListProxy& proxy,
                        client::si::Control& ctl)
    {
        // ex WProcessManagerDialog::performChanges
        // Commit and build a process group
        uint32_t pgid = proxy.commit(link);

        // Run that process group by moving it into the one provided by ScriptSide
        class Task : public client::si::ScriptTask {
         public:
            Task(uint32_t pgid)
                : m_pgid(pgid)
                { }
            virtual void execute(uint32_t pgid, game::Session& session)
                { session.processList().joinProcessGroup(m_pgid, pgid); }
         private:
            uint32_t m_pgid;
        };
        ctl.executeTaskWait(std::auto_ptr<client::si::ScriptTask>(new Task(pgid)));
    }


    /*
     *  ProcessListWidget - display list of processes
     */

    class ProcessListWidget : public ui::widgets::AbstractListbox {
     public:
        enum {
            NameWidth   = 20,
            MsgWidth    = 3,
            WhereWidth  = 5,
            PriWidth    = 2,
            StatusWidth = 8,

            TotalWidth = NameWidth + MsgWidth + WhereWidth + PriWidth + StatusWidth
        };

        ProcessListWidget(ui::Root& root, afl::string::Translator& tx);

        virtual size_t getNumItems() const;
        virtual bool isItemAccessible(size_t n) const;
        virtual int getItemHeight(size_t n) const;
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

        void setContent(const ProcessListProxy::Infos_t& other);
        void scrollToProcess(uint32_t pid);
        const ProcessListProxy::Info_t* getSelectedProcess() const;
        bool getSelectedProcessId(uint32_t& pid) const;

     private:
        afl::base::Ref<gfx::Font> getFont() const;

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        ProcessListProxy::Infos_t m_content;
    };


    /*
     *  ProcessListKeyHandler - handle most keys of ProcessListDialog
     */

    class ProcessListKeyHandler : public ui::InvisibleWidget {
     public:
        ProcessListKeyHandler(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, ProcessListProxy& proxy, MutexListProxy& mProxy, ProcessListWidget& list, client::si::Control& parent);

        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;
        ProcessListProxy& m_proxy;
        MutexListProxy& m_mutexProxy;
        ProcessListWidget& m_list;
        client::si::Control& m_parent;

        void setAllProcessState(ProcessListEditor::State st);
        void setCurrentProcessState(ProcessListEditor::State st);
        void changePriority();
        void listMutexes(bool all);
        void showNotifications();
    };


    /*
     *  ProcessListDialog
     */

    class ProcessListDialog : public client::si::Control {
     public:
        ProcessListDialog(client::si::UserSide& iface,
                          ui::Root& root,
                          ProcessListProxy& proxy,
                          MutexListProxy& mProxy,
                          afl::string::Translator& tx,
                          client::si::OutputState& out);

        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target);
        virtual void handleEndDialog(client::si::RequestLink2 link, int code);
        virtual void handlePopupConsole(client::si::RequestLink2 link);
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link);
        virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text);
        virtual afl::base::Optional<game::Id_t> getFocusedObjectId(game::Reference::Type type) const;
        virtual game::interface::ContextProvider* createContextProvider();

        void onListChange(const ProcessListProxy::Infos_t& content);
        void init(client::Downlink& link, game::Reference invokingObject);
        void run();
        void onGoTo();
        void onClose();
        void onExecute();

     private:
        // References
        ui::Root& m_root;
        client::si::OutputState& m_outputState;

        // Proxies
        ProcessListProxy& m_proxy;
        MutexListProxy& m_mutexProxy;

        // Widgets
        ui::EventLoop m_loop;
        ProcessListWidget m_list;
        ui::widgets::Button m_gotoButton;

        afl::base::SignalConnection conn_listChange;

        void postGoToScreen(int screen, int id);
    };


    /*
     *  ExtraControl - just receive script requests
     */

    class ExtraControl : public client::si::Control {
     public:
        ExtraControl(client::si::UserSide& iface, ui::Root& root, client::si::OutputState& out)
            : Control(iface),
              m_outputState(out),
              m_loop(root)
            { }

        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
            { dialogHandleStateChange(link, target, m_outputState, m_loop, 0); }

        virtual void handleEndDialog(client::si::RequestLink2 link, int /*code*/)
            {
                // We have just closed the dialog, nothing more to do.
                interface().continueProcess(link);
            }

        virtual void handlePopupConsole(client::si::RequestLink2 link)
            {
                // FIXME
                interface().continueProcess(link);
            }

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
        // References
        client::si::OutputState& m_outputState;
        ui::EventLoop m_loop;
    };

}

/*
 *  ProcessListWidget
 */

ProcessListWidget::ProcessListWidget(ui::Root& root, afl::string::Translator& tx)
    : m_root(root),
      m_translator(tx),
      m_content()
{
    // WProcessList::WProcessList
}

size_t
ProcessListWidget::getNumItems() const
{
    return m_content.size();
}

bool
ProcessListWidget::isItemAccessible(size_t /*n*/) const
{
    return true;
}

int
ProcessListWidget::getItemHeight(size_t /*n*/) const
{
    return getFont()->getLineHeight();
}

int
ProcessListWidget::getHeaderHeight() const
{
    return getFont()->getLineHeight();
}

int
ProcessListWidget::getFooterHeight() const
{
    return 0;
}

void
ProcessListWidget::drawHeader(gfx::Canvas& can, gfx::Rectangle area)
{
    // WProcessListHeader::drawContent
    afl::base::Ref<gfx::Font> font = getFont();
    const int em = font->getEmWidth();

    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*font);
    ctx.setColor(util::SkinColor::Static);
    outTextF(ctx, area.splitX(NameWidth*em),   m_translator("Name"));
    outTextF(ctx, area.splitX(MsgWidth*em),    m_translator("Msg"));
    outTextF(ctx, area.splitX(WhereWidth*em),  m_translator("Where"));
    outTextF(ctx, area.splitX(PriWidth*em),    m_translator("Pri"));
    outTextF(ctx, area.splitX(StatusWidth*em), m_translator("Status"));
}

void
ProcessListWidget::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
ProcessListWidget::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WProcessList::drawPart, CTasklist.DrawPart
    // Prepare
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

    if (item < m_content.size()) {
        const ProcessListProxy::Info_t& info = m_content[item];
        afl::base::Ref<gfx::Font> font = getFont();
        afl::base::Ref<gfx::Font> boldFont = m_root.provider().getFont("b");
        const int em = font->getEmWidth();
        ctx.useFont(*font);

        // Name
        gfx::Rectangle nameArea = area.splitX(NameWidth*em);
        nameArea.consumeX(5);
        outTextF(ctx, nameArea, info.name);

        // Msg
        gfx::Rectangle msgArea = area.splitX(MsgWidth*em);
        switch (info.notificationStatus) {
         case ProcessListEditor::NoMessage:
            break;
         case ProcessListEditor::UnreadMessage:
            ctx.useFont(*boldFont);
            outTextF(ctx, msgArea, m_translator("New"));
            break;
         case ProcessListEditor::ConfirmedMessage:
            outTextF(ctx, msgArea, m_translator("OK"));
            break;
        }

        // Where
        ctx.useFont(*font);
        gfx::Rectangle whereArea = area.splitX(WhereWidth*em);
        switch (info.invokingObject.getType()) {
         case Reference::Ship:     outTextF(ctx, whereArea, afl::string::Format("s%d", info.invokingObject.getId())); break;
         case Reference::Planet:   outTextF(ctx, whereArea, afl::string::Format("p%d", info.invokingObject.getId())); break;
         case Reference::Starbase: outTextF(ctx, whereArea, afl::string::Format("b%d", info.invokingObject.getId())); break;
         default: break;
        }

        // Pri
        outTextF(ctx, area.splitX(PriWidth*em), afl::string::Format("%d", info.priority));

        // Status
        // FIXME: bold if changed
        outTextF(ctx, area.splitX(StatusWidth*em), info.status);
    }
}

void
ProcessListWidget::handlePositionChange()
{
    return defaultHandlePositionChange();
}

ui::layout::Info
ProcessListWidget::getLayoutInfo() const
{
    gfx::Point pt = getFont()->getCellSize().scaledBy(TotalWidth, 14);
    return ui::layout::Info(pt, ui::layout::Info::GrowBoth);
}

bool
ProcessListWidget::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

void
ProcessListWidget::setContent(const ProcessListProxy::Infos_t& other)
{
    uint32_t pid = 0;
    bool pidOK = getSelectedProcessId(pid);

    m_content = other;
    requestRedraw();

    if (pidOK) {
        scrollToProcess(pid);
    }
    sig_change.raise();
}

void
ProcessListWidget::scrollToProcess(uint32_t pid)
{
    // WProcessList::scrollToProcess
    for (size_t i = 0, n = m_content.size(); i < n; ++i) {
        if (m_content[i].processId == pid) {
            setCurrentItem(i);
            break;
        }
    }
}

const ProcessListProxy::Info_t*
ProcessListWidget::getSelectedProcess() const
{
    size_t pos = getCurrentItem();
    if (pos < m_content.size()) {
        return &m_content[pos];
    } else {
        return 0;
    }
}

bool
ProcessListWidget::getSelectedProcessId(uint32_t& pid) const
{
    // WProcessList::getSelectedProcess
    if (const ProcessListProxy::Info_t* p = getSelectedProcess()) {
        pid = p->processId;
        return true;
    } else {
        return false;
    }
}

afl::base::Ref<gfx::Font>
ProcessListWidget::getFont() const
{
    return m_root.provider().getFont("");
}


/*
 *  ProcessListKeyHandler
 */

inline
ProcessListKeyHandler::ProcessListKeyHandler(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, ProcessListProxy& proxy, MutexListProxy& mProxy, ProcessListWidget& list, client::si::Control& parent)
    : m_root(root),
      m_gameSender(gameSender),
      m_translator(tx),
      m_proxy(proxy),
      m_mutexProxy(mProxy),
      m_list(list),
      m_parent(parent)
{ }

bool
ProcessListKeyHandler::handleKey(util::Key_t key, int /*prefix*/)
{
    // WProcessManagerDialog::handleEvent
    switch (key) {
     case 't':
        setCurrentProcessState(ProcessListEditor::Terminated);
        return true;
     case 't' + util::KeyMod_Ctrl:
        setAllProcessState(ProcessListEditor::Terminated);
        return true;
     case 's':
        setCurrentProcessState(ProcessListEditor::Suspended);
        return true;
     case 's' + util::KeyMod_Ctrl:
        setAllProcessState(ProcessListEditor::Suspended);
        return true;
     case 'r':
        setCurrentProcessState(ProcessListEditor::Runnable);
        return true;
     case 'r' + util::KeyMod_Ctrl:
        setAllProcessState(ProcessListEditor::Runnable);
        return true;
     case 'p':
        changePriority();
        return true;
     case 'n':
        showNotifications();
        return true;
     case 'l':
        listMutexes(false);
        return true;
     case 'l' + util::KeyMod_Ctrl:
        listMutexes(true);
        return true;
     default:
        return false;
    }
}

inline void
ProcessListKeyHandler::setAllProcessState(ProcessListEditor::State st)
{
    m_proxy.setAllProcessState(st);
}

inline void
ProcessListKeyHandler::setCurrentProcessState(ProcessListEditor::State st)
{
    uint32_t pid = 0;
    if (m_list.getSelectedProcessId(pid)) {
        m_proxy.setProcessState(pid, st);
    }
}

inline void
ProcessListKeyHandler::changePriority()
{
    // WProcessManagerDialog::changePriority, CTasklist.RescheduleCurrent
    if (const ProcessListProxy::Info_t* p = m_list.getSelectedProcess()) {
        uint32_t processId = p->processId;

        afl::base::Observable<int32_t> priority(p->priority);

        afl::base::Deleter del;
        ui::widgets::DecimalSelector& sel = del.addNew(new ui::widgets::DecimalSelector(m_root, m_translator, priority, 0, 99, 10));
        ui::Widget& w = sel.addButtons(del, m_root);
        if (ui::widgets::doStandardDialog(m_translator("Process Manager"), m_translator("Enter new process priority:"), w, true, m_root, m_translator)) {
            // FIXME: help: "pcc2:processmgr"
            m_proxy.setProcessPriority(processId, priority.get());
        }
    }
}

void
ProcessListKeyHandler::listMutexes(bool all)
{
    // WProcessManagerDialog::listLocks, hooks.pas:NListLocks
    // Fetch list; handle errors
    MutexListProxy::Infos_t list;
    client::Downlink link(m_root, m_translator);
    if (all) {
        m_mutexProxy.enumMutexes(link, list);
        if (list.empty()) {
            MessageBox(m_translator("No locks active in system."), m_translator("Locks"), m_root).doOkDialog(m_translator);
            return;
        }
    } else {
        uint32_t processId;
        if (!m_list.getSelectedProcessId(processId)) {
            return;
        }
        m_mutexProxy.enumMutexes(link, list, processId);
        if (list.empty()) {
            MessageBox(m_translator("This process does not own any locks."), m_translator("Locks"), m_root).doOkDialog(m_translator);
            return;
        }
    }

    // Build list to show
    // The StringListDialogWidget was intended for scripting use, but is useful here as well.
    client::si::StringListDialogWidget box(m_root.provider(), m_root.colorScheme(), m_translator("Locks"), 0, 0, 0, "pcc2:processmgr");
    for (size_t i = 0, n = list.size(); i < n; ++i) {
        box.addItem(int32_t(list[i].processId), list[i].name);
    }
    box.sortItemsAlphabetically();

    // Do it
    if (box.run(m_root, m_translator, m_gameSender)) {
        if (all) {
            int32_t key;
            if (box.getCurrentKey().get(key)) {
                m_list.scrollToProcess(uint32_t(key));
            }
        }
    }
}

void
ProcessListKeyHandler::showNotifications()
{
    uint32_t pid = 0;
    if (m_list.getSelectedProcessId(pid)) {
        client::si::OutputState out;
        client::dialogs::showNotifications(pid, m_proxy, m_parent.interface(), m_parent.root(), m_parent.translator(), out);
        m_parent.handleStateChange(out.getProcess(), out.getTarget());
    }
}


/*
 *  ProcessListDialog
 */

ProcessListDialog::ProcessListDialog(client::si::UserSide& iface,
                                     ui::Root& root,
                                     ProcessListProxy& proxy,
                                     MutexListProxy& mProxy,
                                     afl::string::Translator& tx,
                                     client::si::OutputState& out)
    : Control(iface),
      m_root(root),
      m_outputState(out),
      m_proxy(proxy),
      m_mutexProxy(mProxy),
      m_loop(root),
      m_list(root, tx),
      m_gotoButton(tx("G - Go To"), 'g', root),
      conn_listChange(m_proxy.sig_listChange.add(this, &ProcessListDialog::onListChange))
{
    m_gotoButton.sig_fire.add(this, &ProcessListDialog::onGoTo);
}

void
ProcessListDialog::handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
{
    dialogHandleStateChange(link, target, m_outputState, m_loop, 0);
}

void
ProcessListDialog::handleEndDialog(client::si::RequestLink2 link, int code)
{
    dialogHandleEndDialog(link, code, m_outputState, m_loop, 0);
}

void
ProcessListDialog::handlePopupConsole(client::si::RequestLink2 link)
{
    // FIXME
    interface().continueProcess(link);
}

void
ProcessListDialog::handleScanKeyboardMode(client::si::RequestLink2 link)
{
    defaultHandleScanKeyboardMode(link);
}

void
ProcessListDialog::handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetView(link, name, withKeymap);
}

void
ProcessListDialog::handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
{
    defaultHandleUseKeymap(link, name, prefix);
}

void
ProcessListDialog::handleOverlayMessage(client::si::RequestLink2 link, String_t text)
{
    defaultHandleOverlayMessage(link, text);
}

afl::base::Optional<game::Id_t>
ProcessListDialog::getFocusedObjectId(game::Reference::Type type) const
{
    return defaultGetFocusedObjectId(type);
}

game::interface::ContextProvider*
ProcessListDialog::createContextProvider()
{
    return 0;
}

void
ProcessListDialog::onListChange(const ProcessListProxy::Infos_t& content)
{
    m_list.setContent(content);
}

void
ProcessListDialog::init(client::Downlink& link, game::Reference invokingObject)
{
    ProcessListProxy::Infos_t content;
    m_proxy.init(link, content);
    m_list.setContent(content);

    // ex WProcessManagerDialog::findObject
    if (invokingObject.isSet()) {
        for (size_t i = 0; i < content.size(); ++i) {
            if (invokingObject == content[i].invokingObject) {
                m_list.setCurrentItem(i);
                break;
            }
        }
    }
}

void
ProcessListDialog::run()
{
    // WProcessManagerDialog::init (sort-of)
    afl::string::Translator& tx = translator();
    afl::base::Deleter del;

    ui::Widget& keyHandler = del.addNew(new ProcessListKeyHandler(m_root, interface().gameSender(), tx, m_proxy, m_mutexProxy, m_list, *this));

    ui::Window& win = del.addNew(new ui::Window(tx("Process Manager"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(del.addNew(new ui::widgets::ScrollbarContainer(m_list, m_root)));

    ui::Group& g1 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    addButton(m_root, del, g1, keyHandler, "R", 'r');
    addText(m_root, del, g1, tx("Run"));
    addButton(m_root, del, g1, keyHandler, "T", 't');
    addText(m_root, del, g1, tx("Terminate"));
    addButton(m_root, del, g1, keyHandler, "S", 's');
    addText(m_root, del, g1, tx("Suspend"));
    g1.add(del.addNew(new ui::Spacer()));
    addButton(m_root, del, g1, keyHandler, "N", 'n');
    addText(m_root, del, g1, tx("Notification"));
    addButton(m_root, del, g1, keyHandler, "L", 'l');
    addText(m_root, del, g1, tx("Locks"));
    addButton(m_root, del, g1, keyHandler, "P", 'p');
    addText(m_root, del, g1, tx("Priority"));
    win.add(g1);

    ui::Widget& helper = del.addNew(new client::widgets::HelpWidget(m_root, tx, interface().gameSender(), "pcc2:processmgr"));

    ui::Group& g2 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::Button& btnExec = del.addNew(new ui::widgets::Button(tx("X - Execute"), 'x', m_root));
    ui::widgets::Button& btnClose = del.addNew(new ui::widgets::Button(tx("Close"), util::Key_Escape, m_root));
    ui::widgets::Button& btnHelp = del.addNew(new ui::widgets::Button(tx("Help"), 'h', m_root));
    g2.add(btnExec);
    g2.add(m_gotoButton);
    g2.add(del.addNew(new ui::Spacer()));
    g2.add(btnClose);
    g2.add(btnHelp);
    btnClose.sig_fire.add(this, &ProcessListDialog::onClose);
    btnExec.sig_fire.add(this, &ProcessListDialog::onExecute);
    btnHelp.dispatchKeyTo(helper);
    win.add(g2);

    win.add(keyHandler);
    win.add(helper);

    // Do NOT handle Key_Quit here.
    // The process manager will run processes on close.

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

void
ProcessListDialog::onGoTo()
{
    Reference ref;
    if (const ProcessListProxy::Info_t* p = m_list.getSelectedProcess()) {
        ref = p->invokingObject;
    }
    switch (ref.getType()) {
     case Reference::Ship:      postGoToScreen(1, ref.getId()); break;
     case Reference::Planet:    postGoToScreen(2, ref.getId()); break;
     case Reference::Starbase:  postGoToScreen(3, ref.getId()); break;
     default:                                                   break;
    }
}

void
ProcessListDialog::onClose()
{
    m_loop.stop(0);
}

void
ProcessListDialog::onExecute()
{
    // Commit
    client::Downlink link(m_root, translator());
    performChanges(link, m_proxy, *this);

    // Reload if needed
    if (!m_loop.isStopped()) {
        init(link, game::Reference());
    }
}

void
ProcessListDialog::postGoToScreen(int screen, int id)
{
    String_t command = afl::string::Format("Try UI.GotoScreen %d, %d", screen, id);
    executeCommandWait(command, false, command);
}


void
client::dialogs::doProcessListDialog(game::Reference invokingObject,
                                     client::si::UserSide& iface,
                                     client::si::Control& ctl,
                                     client::si::OutputState& out)
{
    client::si::OutputState out1, out2;
    Downlink link(ctl.root(), ctl.translator());
    ProcessListProxy proxy(iface.gameSender(), ctl.root().engine().dispatcher());
    MutexListProxy mProxy(iface.gameSender());

    // Dialog
    {
        ProcessListDialog dlg(iface, ctl.root(), proxy, mProxy, ctl.translator(), out1);
        dlg.init(link, invokingObject);
        dlg.run();
    }

    // We need an extra Control instance here to receive performChanges' requests.
    // The outer Control is still waiting for completion of the command that invoked doProcessListDialog.
    // Also, we can only produce a process in OutputState, not a process group;
    // performChanges will end the wait with a process.
    {
        ExtraControl extra(iface, ctl.root(), out2);
        performChanges(link, proxy, extra);
    }

    // Merge the processes.
    client::si::RequestLink2 proc;
    if (out2.getProcess().isValid()) {
        proc = out2.getProcess();
        if (out1.getProcess().isValid()) {
            iface.joinProcess(proc, out1.getProcess());
        }
    } else {
        proc = out1.getProcess();
    }

    // Merge the target.
    // out2 wins, that is, if a process set to 'Runnable' causes a state change
    // that overrides a 'g' (onGoTo) command.
    client::si::OutputState::Target target =
        out2.getTarget() != client::si::OutputState::NoChange
        ? out2.getTarget()
        : out1.getTarget();

    out.set(proc, target);
}
