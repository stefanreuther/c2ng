/**
  *  \file client/dialogs/selectionmanager.cpp
  *  \brief Selection Manager dialog
  */

#include "client/dialogs/selectionmanager.hpp"
#include "afl/base/deleter.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/helpdialog.hpp"
#include "client/dialogs/searchdialog.hpp"
#include "client/downlink.hpp"
#include "client/si/control.hpp"
#include "client/si/userside.hpp"
#include "game/proxy/selectionproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/draw.hpp"
#include "ui/group.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"
#include "util/skincolor.hpp"
#include "util/string.hpp"
#include "util/unicodechars.hpp"

using game::proxy::SelectionProxy;
using game::SearchQuery;

namespace {
    enum {
        StopNormal,
        StopSearchMarked
    };

    class SelectionList : public ui::widgets::AbstractListbox {
     public:
        SelectionList(ui::Root& root, afl::string::Translator& tx);

        void setContent(const SelectionProxy::Info& content);
        bool hasObjects(size_t layer) const;

        // Widget:
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

        // AbstractListbox:
        virtual size_t getNumItems() const;
        virtual bool isItemAccessible(size_t n) const;
        virtual int getItemHeight(size_t n) const;
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        bool m_valid;
        SelectionProxy::Info m_info;

        int getItemHeight() const;
        afl::base::Ref<gfx::Font> getFont() const;
    };

    class SelectionManager : public client::si::Control, public gfx::KeyEventConsumer {
     public:
        SelectionManager(client::si::UserSide& ui,
                         ui::Root& root,
                         SelectionProxy& proxy,
                         const SelectionProxy::Info& initialInfo,
                         afl::string::Translator& tx);

        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target);
        virtual void handleEndDialog(client::si::RequestLink2 link, int code);
        virtual void handlePopupConsole(client::si::RequestLink2 link);
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link);
        virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text);
        virtual afl::base::Optional<game::Id_t> getFocusedObjectId(game::Reference::Type type) const;
        virtual game::interface::ContextProvider* createContextProvider();

        int run();
        bool handleKey(util::Key_t key, int prefix);

        client::si::OutputState& outputState();

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        SelectionProxy& m_proxy;
        util::RequestSender<game::Session> m_gameSender;
        SelectionList m_list;
        ui::widgets::KeyForwarder m_keyDispatcher;
        ui::EventLoop m_loop;
        client::si::OutputState m_outputState;
        afl::base::SignalConnection conn_selectionChange;

        void onSelectionChange(const SelectionProxy::Info& info);
        void onOK();
        void doCopy();
        void doSave(String_t title, String_t flags);
        void doLoad(String_t title, String_t flags);
        void addButton(afl::base::Deleter& del, ui::Group& group, String_t name, util::Key_t key, bool left);
        void executeScriptOperationWait(String_t funcName, String_t title, String_t flags);
    };
}


/*
 *  SelectionList
 */

SelectionList::SelectionList(ui::Root& root, afl::string::Translator& tx)
    : m_root(root), m_translator(tx), m_valid(false), m_info()
{ }

void
SelectionList::setContent(const SelectionProxy::Info& content)
{
    bool changePosition = !m_valid || getCurrentItem() == m_info.currentLayer;
    m_info = content;
    m_valid = true;
    if (changePosition) {
        setCurrentItem(m_info.currentLayer);
    }
    requestRedraw();
}

bool
SelectionList::hasObjects(size_t layer) const
{
    return (layer < m_info.layers.size())
        && (m_info.layers[layer].numPlanets > 0
            || m_info.layers[layer].numShips > 0);
}

// Widget:
void
SelectionList::handlePositionChange()
{
    defaultHandlePositionChange();
}

ui::layout::Info
SelectionList::getLayoutInfo() const
{
    const gfx::Point cellSize = getFont()->getCellSize();
    const int maxLines = std::min(20, int(m_info.layers.size()));
    const int width = 20;
    return ui::layout::Info(cellSize.scaledBy(width, maxLines), ui::layout::Info::GrowBoth);
}

bool
SelectionList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

// AbstractListbox:
size_t
SelectionList::getNumItems() const
{
    return m_info.layers.size();
}

bool
SelectionList::isItemAccessible(size_t /*n*/) const
{
    return true;
}

int
SelectionList::getItemHeight(size_t /*n*/) const
{
    return getItemHeight();
}

int
SelectionList::getHeaderHeight() const
{
    return 0;
}

int
SelectionList::getFooterHeight() const
{
    return 0;
}

void
SelectionList::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
SelectionList::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
SelectionList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WSelectionLayerInfo::drawContent, CSelectionView.Draw
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);
    ctx.setColor(util::SkinColor::Static);
    if (item < m_info.layers.size()) {
        afl::base::Ref<gfx::Font> font = getFont();
        ctx.useFont(*font);

        // Marker
        ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
        gfx::Rectangle markerArea = area.splitX(font->getEmWidth());
        if (item == m_info.currentLayer) {
            outTextF(ctx, markerArea, UTF_RIGHT_TRIANGLE);
        }

        // Name
        ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
        gfx::Rectangle letterArea = area.splitX(font->getEmWidth() * 3/2);
        outTextF(ctx, letterArea, String_t(1, char('A' + item)) + ':');

        // Label
        String_t text;
        if (size_t n = m_info.layers[item].numPlanets) {
            util::addListItem(text, ", ", afl::string::Format("%d planet%!1{s%}", n));
        }
        if (size_t n = m_info.layers[item].numShips) {
            util::addListItem(text, ", ", afl::string::Format("%d ship%!1{s%}", n));
        }
        outTextF(ctx, area, text);
    }
}

int
SelectionList::getItemHeight() const
{
    return getFont()->getLineHeight();
}

afl::base::Ref<gfx::Font>
SelectionList::getFont() const
{
    return m_root.provider().getFont("+");
}


/*
 *  SelectionManager
 */

SelectionManager::SelectionManager(client::si::UserSide& ui, ui::Root& root, SelectionProxy& proxy, const SelectionProxy::Info& initialInfo, afl::string::Translator& tx)
    : Control(ui),
      m_root(root),
      m_translator(tx),
      m_proxy(proxy),
      m_gameSender(ui.gameSender()),
      m_list(root, tx),
      m_keyDispatcher(*this),
      m_loop(root),
      m_outputState(),
      conn_selectionChange(proxy.sig_selectionChange.add(this, &SelectionManager::onSelectionChange))
{
    m_list.setContent(initialInfo);
}

void
SelectionManager::handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
{
    // We don't expect this to be called, but it doesn't hurt.
    dialogHandleStateChange(link, target, m_outputState, m_loop, 0);
}

void
SelectionManager::handleEndDialog(client::si::RequestLink2 link, int code)
{
    dialogHandleEndDialog(link, code, m_outputState, m_loop, 0);
}

void
SelectionManager::handlePopupConsole(client::si::RequestLink2 link)
{
    // We don't expect this to be called
    interface().continueProcess(link);
}

void
SelectionManager::handleScanKeyboardMode(client::si::RequestLink2 link)
{
    defaultHandleScanKeyboardMode(link);
}

void
SelectionManager::handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetView(link, name, withKeymap);
}

void
SelectionManager::handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
{
    defaultHandleUseKeymap(link, name, prefix);
}

void
SelectionManager::handleOverlayMessage(client::si::RequestLink2 link, String_t text)
{
    defaultHandleOverlayMessage(link, text);
}

afl::base::Optional<game::Id_t>
SelectionManager::getFocusedObjectId(game::Reference::Type type) const
{
    return defaultGetFocusedObjectId(type);
}

game::interface::ContextProvider*
SelectionManager::createContextProvider()
{
    return 0;
}

int
SelectionManager::run()
{
    afl::base::Deleter del;

    // VBox
    //   HBox
    //     SelectionList
    //     VBox
    //       UIButton "D-Clear"
    //       UIButton "I-Invert"
    //       UIButton "C-Copy"
    //       UIButton "S-Save"
    //       UIButton "R-Load"
    //       UIButton "M-Merge"
    //       UISpacer
    //   HBox
    //     UIButton "H"
    //     UISpacer
    //     UIButton "Enter-Select"
    //     UIButton "ESC-Close"
    ui::Window& win = del.addNew(new ui::Window(m_translator("Selection Manager"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    ui::Group& group1  = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Group& group12 = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::Group& group2  = del.addNew(new ui::Group(ui::layout::HBox::instance5));

    group1.add(m_list);
    group1.add(group12);

    addButton(del, group12, m_translator("D - Clear"),  'd', true);
    addButton(del, group12, m_translator("I - Invert"), 'i', true);
    addButton(del, group12, m_translator("C - Copy"),   'c', true);
    addButton(del, group12, m_translator("S - Save"),   's', true);
    addButton(del, group12, m_translator("L - Load"),   'l', true);
    addButton(del, group12, m_translator("M - Merge"),  'm', true);
    group12.add(del.addNew(new ui::Spacer()));
    win.add(group1);

    addButton(del, group2, m_translator("Help"), 'h', false);
    group2.add(del.addNew(new ui::Spacer()));
    addButton(del, group2, m_translator("Enter - Select"), util::Key_Return, false);
    addButton(del, group2, m_translator("ESC - Close"), util::Key_Escape, false);
    win.add(group2);
    win.add(m_keyDispatcher);
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

    win.pack();
    m_list.requestFocus();
    m_list.sig_itemDoubleClick.add(this, &SelectionManager::onOK);
    m_root.centerWidget(win);
    m_root.add(win);
    return m_loop.run();
}

bool
SelectionManager::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex WSelectionManager::handleEvent(const UIEvent& event, bool second_pass)
    switch (key) {
     case util::Key_Escape:
        m_loop.stop(StopNormal);
        return true;

     case util::Key_Return:
        onOK();
        return true;

     case 'c':
     case util::Key_Insert:
        // Copy
        doCopy();
        return true;

     case 'd':
     case util::Key_Delete:
        // Delete
        if (ui::dialogs::MessageBox(m_translator("Do you want to clear this selection layer?"),
                                    m_translator("Selection Manager"),
                                    m_root).doYesNoDialog(m_translator))
        {
            m_proxy.clearLayer(m_list.getCurrentItem());
        }
        return true;

     case 'd' + util::KeyMod_Ctrl:
     case util::Key_Delete + util::KeyMod_Ctrl:
        // Delete all
        if (ui::dialogs::MessageBox(m_translator("Do you want to clear all selection layers?"),
                                    m_translator("Selection Manager"),
                                    m_root).doYesNoDialog(m_translator))
        {
            m_proxy.clearAllLayers();
        }
        return true;

     case 'i':
     case '*':
        // Invert
        m_proxy.invertLayer(m_list.getCurrentItem());
        return true;

     case 'i' + util::KeyMod_Ctrl:
     case '*' + util::KeyMod_Ctrl:
        // Invert all
        m_proxy.invertAllLayers();
        return true;

     case 's':
        // Save
        doSave(m_translator("Save Selection"), afl::string::Format("%d", m_list.getCurrentItem()));
        return true;

     case 's' + util::KeyMod_Ctrl:
        // Save all
        doSave(m_translator("Save All Selections"), "");
        return true;

     case 'l':
     case 'l' + util::KeyMod_Ctrl:
     case 'r':                           // PCC1 compatibility
     case 'r' + util::KeyMod_Ctrl:       // PCC1 compatibility
        // Load
        doLoad(m_translator("Load Selection"), afl::string::Format("u%d", m_list.getCurrentItem()));
        return true;

     case 'm':
     case 'm' + util::KeyMod_Ctrl:
        // Merge
        doLoad(m_translator("Merge Selection"), afl::string::Format("mu%d", m_list.getCurrentItem()));
        return true;

     case 'h':
     case 'h' + util::KeyMod_Alt:
     case util::Key_F1:
        client::dialogs::doHelpDialog(m_root, m_translator, m_gameSender, "pcc2:selectionmgr");
        return true;

     case util::Key_F7:
        // Search
        if (m_list.hasObjects(m_list.getCurrentItem())) {
            // Activate layer
            m_proxy.setCurrentLayer(m_list.getCurrentItem());

            // Exit dialog
            m_loop.stop(StopSearchMarked);
        }
        return true;

     default:
        return false;
    }
}

inline client::si::OutputState&
SelectionManager::outputState()
{
    return m_outputState;
}

void
SelectionManager::onSelectionChange(const SelectionProxy::Info& info)
{
    m_list.setContent(info);
}

void
SelectionManager::onOK()
{
    m_proxy.setCurrentLayer(m_list.getCurrentItem());
    m_loop.stop(StopNormal);
}

void
SelectionManager::doCopy()
{
    ui::widgets::InputLine input(2000, 25, m_root);
    if (ui::widgets::doStandardDialog(m_translator("Selection Manager"), m_translator("Enter layer/expression to copy from:"), input, false, m_root, m_translator)) {
        String_t error;
        client::Downlink link(m_root, m_translator);
        if (!m_proxy.executeExpression(link, input.getText(), m_list.getCurrentItem(), error)) {
            ui::dialogs::MessageBox(afl::string::Format(m_translator("Invalid selection expression: %s"), error),
                                    m_translator("Selection Manager"),
                                    m_root).doOkDialog(m_translator);
        }
    }
}

void
SelectionManager::doSave(String_t title, String_t flags)
{
    executeScriptOperationWait("CCUI$SAVESELECTION", title, flags);
}

void
SelectionManager::doLoad(String_t title, String_t flags)
{
    executeScriptOperationWait("CCUI$LOADSELECTION", title, flags);
}

void
SelectionManager::addButton(afl::base::Deleter& del, ui::Group& group, String_t name, util::Key_t key, bool left)
{
    ui::widgets::Button& btn = del.addNew(new ui::widgets::Button(name, key, m_root));
    btn.dispatchKeyTo(m_keyDispatcher);
    if (left) {
        btn.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
    }
    group.add(btn);
}

void
SelectionManager::executeScriptOperationWait(String_t funcName, String_t title, String_t flags)
{
    class Task : public client::si::ScriptTask {
     public:
        Task(String_t funcName, String_t title, String_t flags)
            : m_funcName(funcName), m_title(title), m_flags(flags)
            { }
        virtual void execute(uint32_t pgid, game::Session& session)
            {
                // Create bytecode
                interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);

                afl::data::StringValue title(m_title);
                bco->addPushLiteral(&title);

                afl::data::StringValue flags(m_flags);
                bco->addPushLiteral(&flags);

                bco->addInstruction(interpreter::Opcode::maPush,
                                    interpreter::Opcode::sNamedShared,
                                    bco->addName(m_funcName));

                bco->addInstruction(interpreter::Opcode::maIndirect,
                                    interpreter::Opcode::miIMCall,
                                    2);

                // Create process
                interpreter::Process& proc = session.processList().create(session.world(), "(Selection Manager)");
                proc.pushFrame(bco, false);
                session.processList().resumeProcess(proc, pgid);
            }
     private:
        String_t m_funcName;
        String_t m_title;
        String_t m_flags;
    };
    executeTaskWait(std::auto_ptr<client::si::ScriptTask>(new Task(funcName, title, flags)));
}

namespace {
    int doSelectionManagerMain(client::si::UserSide& iface,
                               client::si::OutputState& out)
    {
        // Set up proxy
        SelectionProxy proxy(iface.gameSender(), iface.root().engine().dispatcher());
        SelectionProxy::Info info;
        {
            client::Downlink link(iface.root(), iface.translator());
            proxy.init(link, info);
        }

        // Early exit if proxy not functional
        if (info.layers.empty()) {
            return StopNormal;
        }

        // Dialog
        SelectionManager mgr(iface, iface.root(), proxy, info, iface.translator());
        int code = mgr.run();
        out = mgr.outputState();
        return code;
    }
}


/*
 *  Main Entry Point
 */

void
client::dialogs::doSelectionManager(client::si::UserSide& iface,
                                    client::si::OutputState& out)
{
    // ex doSelectionManager, search.pas:SelectionManager
    const int code = doSelectionManagerMain(iface, out);
    if (code == StopSearchMarked) {
        // Search
        client::si::OutputState out2;
        doSearchDialog(SearchQuery(SearchQuery::MatchTrue,
                                   SearchQuery::SearchObjects_t() + SearchQuery::SearchShips + SearchQuery::SearchPlanets,
                                   "Marked"),
                       game::Reference(), true, iface, out2);

        // Join outbound processes
        if (out.getProcess().isValid()) {
            iface.joinProcess(out.getProcess(), out2.getProcess());
        } else {
            out = out2;
        }
    }
}

afl::base::Optional<game::SearchQuery>
client::dialogs::doSelectionManagerFromSearch(client::si::UserSide& iface,
                                              client::si::OutputState& out)
{
    const int code = doSelectionManagerMain(iface, out);
    if (code == StopSearchMarked) {
        return SearchQuery(SearchQuery::MatchTrue,
                           SearchQuery::SearchObjects_t() + SearchQuery::SearchShips + SearchQuery::SearchPlanets,
                           "Marked");
    } else {
        return afl::base::Nothing;
    }
}
