/**
  *  \file client/screens/browserscreen.cpp
  *  \brief Class client::screens::BrowserScreen
  */

#include <vector>
#include "client/screens/browserscreen.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/directorysetup.hpp"
#include "client/dialogs/folderconfigdialog.hpp"
#include "client/dialogs/helpdialog.hpp"
#include "client/dialogs/newaccount.hpp"
#include "client/dialogs/pluginmanager.hpp"
#include "client/dialogs/simpleconsole.hpp"
#include "client/dialogs/sweep.hpp"
#include "client/dialogs/unpack.hpp"
#include "client/downlink.hpp"
#include "client/help.hpp"
#include "client/si/nullcontrol.hpp"
#include "client/si/scripttask.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/exception.hpp"
#include "game/interface/plugins.hpp"
#include "game/playerlist.hpp"
#include "game/proxy/maintenanceadaptor.hpp"
#include "game/proxy/maintenanceproxy.hpp"
#include "game/root.hpp"
#include "ui/defaultresourceprovider.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/widgets/transparentwindow.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/translation.hpp"

using afl::container::PtrVector;
using afl::string::Format;
using client::dialogs::SimpleConsole;
using game::proxy::BrowserProxy;
using util::rich::StyleAttribute;
using util::rich::Text;

namespace {
    enum Action {
        xLocalSetup,
        xConfiguration,
        xSweep,
        xUnpack,
        xMaketurn
    };
    typedef afl::bits::SmallSet<Action> Actions_t;

    struct ActionInfo {
        const char* name;
        Action action : 8;
        bool isAction;
    };
    const ActionInfo ACTION_INFO[] = {
        { N_("Set up for playing"), xLocalSetup,    false },
        { N_("Configure"),          xConfiguration, false },
        { N_("Unpack"),             xUnpack,        true },
        { N_("Maketurn"),           xMaketurn,      true },
        { N_("Sweep"),              xSweep,         true },
    };

    static Actions_t getActions(const game::Root::Actions_t as)
    {
        using game::Root;
        Actions_t result;
        if (as.contains(Root::aLocalSetup)) {
            result += xLocalSetup;
        }
        if (as.containsAnyOf(Root::Actions_t() + Root::aConfigureCharset + Root::aConfigureFinished + Root::aConfigureReadOnly)) {
            result += xConfiguration;
        }
        if (as.contains(Root::aSweep)) {
            result += xSweep;
        }
        if (as.contains(Root::aUnpack)) {
            result += xUnpack;
        }
        if (as.contains(Root::aMaketurn)) {
            result += xMaketurn;
        }
        return result;
    }

    bool checkLocalSetup(ui::Root& root, afl::string::Translator& tx, BrowserProxy& proxy)
    {
        client::Downlink link(root, tx);
        return proxy.isSelectedFolderSetupSuggested(link);
    }

    game::Root& mustHaveRoot(game::Root* pRoot)
    {
        if (pRoot == 0) {
            throw game::Exception(game::Exception::eUser);
        }
        return *pRoot;
    }

    class MaintenanceBrowserAdaptor : public game::proxy::MaintenanceAdaptor {
     public:
        MaintenanceBrowserAdaptor(game::browser::Session& bro)
            : m_browserSession(bro),
              m_root(mustHaveRoot(bro.browser().getSelectedRoot().get()))
            { }
        virtual afl::io::Directory& targetDirectory()
            { return m_root->gameDirectory(); }
        virtual afl::string::Translator& translator()
            { return m_browserSession.translator(); }
        virtual afl::charset::Charset& charset()
            { return m_root->charset(); }
        virtual const game::PlayerList& playerList()
            { return m_root->playerList(); }
        virtual afl::io::FileSystem& fileSystem()
            { return m_browserSession.browser().fileSystem(); }
        virtual game::config::UserConfiguration& userConfiguration()
            { return m_root->userConfiguration(); }
     private:
        game::browser::Session& m_browserSession;
        afl::base::Ref<game::Root> m_root;
    };

    class MaintenanceFromBrowser : public afl::base::Closure<game::proxy::MaintenanceAdaptor*(game::browser::Session&)> {
     public:
        virtual game::proxy::MaintenanceAdaptor* call(game::browser::Session& bro)
            { return new MaintenanceBrowserAdaptor(bro); }
    };
}

/***************************** BrowserScreen *****************************/

client::screens::BrowserScreen::BrowserScreen(client::si::UserSide& us, game::proxy::BrowserProxy& proxy, util::RequestSender<game::browser::Session> browserSender)
    : m_userSide(us),
      m_root(us.root()),
      m_translator(us.translator()),
      m_gameSender(us.gameSender()),
      m_browserSender(browserSender),
      m_receiver(m_root.engine().dispatcher(), *this),
      m_proxy(proxy),
      m_list(gfx::Point(20, 20), m_root),
      m_crumbs(m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(40, 1), m_root),
      m_info(m_root.provider(), m_root.colorScheme()),
      m_optionButton(m_translator("Ins - Add Account"), util::Key_Insert, m_root),
      m_infoItems(),
      m_infoIndex(0),
      m_loop(m_root),
      m_hasUp(false),
      m_state(Working),
      m_blockState(false),
      m_timer(m_root.engine().createTimer())
{
    m_crumbs.setChangeOnClick(true);
    m_timer->sig_fire.add(this, &BrowserScreen::onTimer);
    m_list.sig_change.add(this, &BrowserScreen::onListMoved);
    m_list.sig_itemDoubleClick.add(this, &BrowserScreen::onItemDoubleClicked);
    m_crumbs.sig_change.add(this, &BrowserScreen::onCrumbClicked);
    m_optionButton.sig_fire.add(this, &BrowserScreen::onAddAccount);
    m_info.setRenderFlag(m_info.UseBackgroundColorScheme, true);
    m_info.setRenderFlag(m_info.NoShade, true);

    conn_browserUpdate = m_proxy.sig_update.add(this, &BrowserScreen::onUpdate);
    conn_browserSelectedInfoUpdate = m_proxy.sig_selectedInfoUpdate.add(this, &BrowserScreen::onSelectedInfoUpdate);
}

int
client::screens::BrowserScreen::run(gfx::ColorScheme<util::SkinColor::Color>& parentColors)
{
    ui::widgets::TransparentWindow window(parentColors, ui::layout::VBox::instance5);

    ui::Group buttons(ui::layout::HBox::instance5);
    ui::widgets::Button btnExit(m_translator("Exit"), util::Key_Escape, m_root);
    ui::Spacer btnSpacer;
    buttons.add(btnExit);
    buttons.add(btnSpacer);
    buttons.add(m_optionButton);

    window.add(m_crumbs);

    ui::widgets::KeyDispatcher keys;
    keys.add(util::Key_Tab,                      this, &BrowserScreen::onKeyTab);
    keys.add(util::Key_Tab + util::KeyMod_Shift, this, &BrowserScreen::onKeyTab);
    keys.add(util::Key_Return,                   this, &BrowserScreen::onKeyEnter);
    keys.add(util::Key_Right,                    this, &BrowserScreen::onKeyEnter);
    keys.add(util::Key_Left,                     this, &BrowserScreen::onKeyLeft);
    keys.add('h',                                this, &BrowserScreen::onKeyHelp);
    keys.add('m',                                this, &BrowserScreen::onMaketurnAction);
    keys.add('s',                                this, &BrowserScreen::onSweepAction);
    keys.add('u',                                this, &BrowserScreen::onUnpackAction);
    keys.add(util::Key_F1,                       this, &BrowserScreen::onKeyHelp);
    keys.add(util::Key_F5,                       this, &BrowserScreen::onKeyPlugin);
    keys.add(util::Key_Quit,                     this, &BrowserScreen::onKeyQuit);
    window.add(keys);

    ui::Group g(ui::layout::HBox::instance5);
    g.add(m_list);
    g.add(m_info);
    window.add(g);
    window.add(buttons);
    window.pack();
    m_root.centerWidget(window);
    m_root.add(window);
    m_list.requestFocus();

    m_proxy.loadContent();
    setState(Blocked);

    btnExit.sig_fire.add(&m_loop, &ui::EventLoop::stop);
    return m_loop.run();
}

void
client::screens::BrowserScreen::stop(int n)
{
    m_loop.stop(n);
}

void
client::screens::BrowserScreen::setBlockState(bool flag)
{
    m_blockState = flag;
    setState(m_state);
}

util::RequestSender<client::screens::BrowserScreen>
client::screens::BrowserScreen::getSender()
{
    return m_receiver.getSender();
}

bool
client::screens::BrowserScreen::isUpLink(size_t index) const
{
    return index == 0 && m_hasUp;
}

size_t
client::screens::BrowserScreen::getIndex(size_t index) const
{
    return m_hasUp ? index-1 : index;
}

game::proxy::BrowserProxy::OptionalIndex_t
client::screens::BrowserScreen::getEffectiveIndex(size_t index) const
{
    if (m_hasUp) {
        if (index == 0) {
            return BrowserProxy::OptionalIndex_t();
        } else {
            return index-1;
        }
    } else {
        return index;
    }
}

void
client::screens::BrowserScreen::requestLoad()
{
    m_info.clear();
    m_proxy.selectFolder(getEffectiveIndex(m_list.getCurrentItem()));
}

void
client::screens::BrowserScreen::onItemDoubleClicked(size_t nr)
{
    switch (m_state) {
     case Working:
     case WorkingLoad:
        if (isUpLink(nr)) {
            m_proxy.openParent(1);
            setState(Blocked);
        } else {
            if (const client::widgets::FolderListbox::Item* p = m_list.getItem(nr)) {
                if (p->canEnter) {
                    m_proxy.openChild(getIndex(nr));
                    setState(Blocked);
                }
            }
        }
        m_list.requestFocus();
        break;

     case Disabled:
     case Blocked:
        break;
    }
}

void
client::screens::BrowserScreen::onCrumbClicked(size_t nr)
{
    switch (m_state) {
     case Working:
     case WorkingLoad:
     {
        size_t n = m_crumbs.getNumItems();
        if (nr != n-1) {
            m_proxy.openParent(n - nr - 1);
            setState(Blocked);
        }
        break;
     }

     case Disabled:
     case Blocked:
        break;
    }
}

void
client::screens::BrowserScreen::onTimer()
{
    if (m_state == Blocked) {
        setState(Disabled);
    }
}

void
client::screens::BrowserScreen::onListMoved()
{
    if (m_state == Working) {
        size_t current = m_list.getCurrentItem();
        if (m_infoIndex != current) {
            requestLoad();
            setState(WorkingLoad);
        }
    }
}

void
client::screens::BrowserScreen::onKeyTab(int)
{
    // ex PCC2GameChooserWindow::handleEvent (part)
    if (m_list.hasState(m_list.FocusedState)) {
        // Focus is on folder list. Activate info list if possible.
        if (m_info.isItemAccessible(m_info.getCurrentItem())) {
            m_info.requestFocus();
        }
    } else {
        // Focus not on folder list. Activate it.
        m_list.requestFocus();
    }
}

void
client::screens::BrowserScreen::onKeyEnter(int)
{
    // ex PCC2GameChooserWindow::handleEvent (part)
    // c2ng change: PCC2 handles Enter/Right differently
    if (m_state != Working) {
        // We don't know what is on the right
    } else if (m_list.hasState(m_list.FocusedState)) {
        // Focus is on folder list. Activate info list if possible, otherwise enter folder.
        if (m_info.isItemAccessible(m_info.getCurrentItem())) {
            m_info.requestFocus();
        } else {
            onItemDoubleClicked(m_list.getCurrentItem());
        }
    } else {
        // Focus on info list. Enter game.
        size_t infoIndex = m_info.getCurrentItem();
        if (infoIndex < m_infoItems.size()) {
            switch (m_infoItems[infoIndex]->action) {
             case NoAction:
                break;

             case PlayAction:
                if (preparePlayAction(infoIndex)) {
                    // FIXME: PCC2 does Unpack when it is applicable. How do we implement that?
                    sig_gameSelection.raise(m_infoItems[infoIndex]->actionParameter);
                }
                break;

             case FolderAction:
                onItemDoubleClicked(m_list.getCurrentItem());
                break;

             case RootAction:
                onRootAction(infoIndex);
                break;
            }
        }
    }
}

void
client::screens::BrowserScreen::onKeyLeft(int)
{
    // ex PCC2GameChooserWindow::handleEvent (part)
    if (m_list.hasState(m_list.FocusedState)) {
        // Focus is on folder list. Go to parent.
        if (isUpLink(0)) {
            onItemDoubleClicked(0);
        }
    } else {
        // Focus is on info list. Activate folder list.
        m_list.requestFocus();
    }
}

void
client::screens::BrowserScreen::onKeyHelp(int)
{
    // ex PCC2GameChooserWindow::handleEvent (part)
    client::dialogs::doHelpDialog(m_root, m_translator, m_gameSender, "pcc2:gamesel");
}

void
client::screens::BrowserScreen::onKeyPlugin(int)
{
    class LocalPluginManager : public client::dialogs::PluginManager {
     public:
        LocalPluginManager(client::si::UserSide& us)
            : PluginManager(us.root(), us.gameSender(), us.translator()),
              m_userSide(us)
            { }
        virtual void unloadPlugin(const String_t& id)
            {
                class Confirmer : public util::Request<game::proxy::WaitIndicator> {
                 public:
                    virtual void handle(game::proxy::WaitIndicator& ind)
                        { ind.post(true); }
                };

                class ManagerRequest : public util::Request<ui::res::Manager> {
                 public:
                    ManagerRequest(const String_t& id, util::RequestSender<game::proxy::WaitIndicator> reply)
                        : m_id(id), m_reply(reply)
                        { }

                    ~ManagerRequest()
                        { m_reply.postNewRequest(new Confirmer()); }

                    virtual void handle(ui::res::Manager& mgr)
                        { mgr.removeProvidersByKey(m_id); }
                 private:
                    const String_t m_id;
                    util::RequestSender<game::proxy::WaitIndicator> m_reply;
                };

                class HelpRequest : public util::Request<game::Session> {
                 public:
                    HelpRequest(const String_t& id)
                        : m_id(id)
                        { }
                    virtual void handle(game::Session& session)
                        { getHelpIndex(session).removeFilesByOrigin(m_id); }
                 private:
                    const String_t m_id;
                };

                Downlink link(root(), translator());
                if (ui::DefaultResourceProvider* drp = dynamic_cast<ui::DefaultResourceProvider*>(&root().provider())) {
                    util::RequestReceiver<game::proxy::WaitIndicator> linkReceiver(root().engine().dispatcher(), link);
                    drp->postNewManagerRequest(new ManagerRequest(id, linkReceiver.getSender()), true);
                    link.wait();
                }

                HelpRequest ht(id);
                link.call(gameSender(), ht);
            }
        virtual void loadPlugin(const String_t& id)
            {
                class Task : public client::si::ScriptTask {
                 public:
                    Task(const String_t& id)
                        : m_id(id)
                        { }
                    virtual void execute(uint32_t pgid, game::Session& session)
                        {
                            interpreter::ProcessList& list = session.processList();
                            interpreter::Process& proc = list.create(session.world(), "(Plugin Loader)");
                            if (util::plugin::Plugin* plug = session.plugins().getPluginById(m_id)) {
                                proc.pushFrame(game::interface::createPluginLoader(*plug), false);
                                plug->setLoaded(true);
                            }
                            list.resumeProcess(proc, pgid);
                        }
                 private:
                    const String_t m_id;
                };

                client::si::NullControl(m_userSide).executeTaskWait(std::auto_ptr<client::si::ScriptTask>(new Task(id)));
            }
     private:
        client::si::UserSide& m_userSide;
    };

    LocalPluginManager(m_userSide).run();
}

void
client::screens::BrowserScreen::onKeyQuit(int)
{
    // ex PCC2GameChooserWindow::handleEvent (part)
    m_loop.stop(0);
    m_root.ungetKeyEvent(util::Key_Quit, 0);
}

void
client::screens::BrowserScreen::onAddAccount(int)
{
    if (m_state == Working) {
        client::widgets::HelpWidget help(m_root, m_translator, m_gameSender, "pcc2:addaccount");
        if (client::dialogs::doNewAccountDialog(m_proxy, &help, m_root, m_translator)) {
            // Refresh
            m_proxy.loadContent();
            setState(Blocked);
        }
    }
}

void
client::screens::BrowserScreen::onRootAction(size_t index)
{
    Actions_t actions = getActions(m_infoActions);

    ui::widgets::StringListbox list(m_root.provider(), m_root.colorScheme());
    for (size_t i = 0; i < countof(ACTION_INFO); ++i) {
        if (actions.contains(ACTION_INFO[i].action)) {
            list.addItem(ACTION_INFO[i].action, m_translator(ACTION_INFO[i].name) + String_t("..."));
        }
    }
    list.setPreferredHeight(int(list.getNumItems()));

    ui::EventLoop loop(m_root);
    if (!ui::widgets::MenuFrame(ui::layout::HBox::instance5, m_root, loop).doMenu(list, m_info.getAbsoluteItemPosition(index).getBottomLeft())) {
        return;
    }

    int32_t k = 0;
    if (!list.getCurrentKey(k)) {
        return;
    }

    switch (Action(k)) {
     case xLocalSetup:
        break;
     case xConfiguration:
        client::dialogs::doFolderConfigDialog(m_root, m_proxy, m_translator);
        break;
     case xSweep:
        onSweepAction();
        break;
     case xUnpack:
        onUnpackAction();
        break;
     case xMaketurn:
        onMaketurnAction();
        break;
    }
}

void
client::screens::BrowserScreen::onUnpackAction()
{
    if (m_state == Working && getActions(m_infoActions).contains(xUnpack)) {
        game::proxy::MaintenanceProxy proxy(m_browserSender.makeTemporary(new MaintenanceFromBrowser()), m_root.engine().dispatcher());
        client::widgets::HelpWidget help(m_root, m_translator, m_gameSender, "pcc2:unpack");
        if (client::dialogs::doUnpackDialog(proxy, &help, m_root, m_translator)) {
            // Refresh
            m_proxy.loadContent();
            setState(Blocked);
        }
    }
}

void
client::screens::BrowserScreen::onMaketurnAction()
{
    if (m_state == Working && getActions(m_infoActions).contains(xMaketurn)) {
        game::proxy::MaintenanceProxy proxy(m_browserSender.makeTemporary(new MaintenanceFromBrowser()), m_root.engine().dispatcher());
        Downlink link(m_root, m_translator);
        game::proxy::MaintenanceProxy::MaketurnStatus st = proxy.prepareMaketurn(link);
        if (st.valid) {
            // Do it
            SimpleConsole console(m_root, m_translator);
            proxy.sig_message.add(&console, &SimpleConsole::addMessage);
            proxy.sig_actionComplete.add(&console, &SimpleConsole::enableClose);
            proxy.startMaketurn(st.availablePlayers);
            console.run(m_translator("Maketurn"));

            // Refresh
            m_proxy.loadContent();
            setState(Blocked);
        }
    }
}

void
client::screens::BrowserScreen::onSweepAction()
{
    if (m_state == Working && getActions(m_infoActions).contains(xSweep)) {
        game::proxy::MaintenanceProxy proxy(m_browserSender.makeTemporary(new MaintenanceFromBrowser()), m_root.engine().dispatcher());
        client::widgets::HelpWidget help(m_root, m_translator, m_gameSender, "pcc2:sweep");
        if (client::dialogs::doSweepDialog(proxy, &help, m_root, m_translator)) {
            // Refresh
            m_proxy.loadContent();
            setState(Blocked);
        }
    }
}

bool
client::screens::BrowserScreen::preparePlayAction(size_t /*index*/)
{
    if (checkLocalSetup(m_root, m_translator, m_proxy)) {
        client::widgets::HelpWidget help(m_root, m_translator, m_gameSender, "pcc2:gamedirsetup");
        if (!client::dialogs::doDirectorySetupDialog(m_proxy, &help, m_root, m_translator)) {
            return false;
        }
    }
    return true;
}

void
client::screens::BrowserScreen::setState(State st)
{
    m_state = st;
    switch (st) {
     case Working:
        // Steady state, all information available
        m_list.setFlag(m_list.Blocked,        m_blockState);
        m_list.setState(m_list.DisabledState, false);
        m_info.setFlag(m_list.Blocked,        m_blockState);
        m_info.setState(m_list.DisabledState, false);
        m_optionButton.setState(m_optionButton.DisabledState, m_hasUp /*FIXME: correct condition!*/);
        break;

     case WorkingLoad:
        // List is in steady state, info is loading; block it.
        m_list.setFlag(m_list.Blocked,        m_blockState);
        m_list.setState(m_list.DisabledState, false);
        m_info.setFlag(m_list.Blocked,        true);
        m_info.setState(m_list.DisabledState, false);
        m_optionButton.setState(m_optionButton.DisabledState, m_hasUp /*FIXME: correct condition!*/);
        break;

     case Blocked:
        // List is loading, block both lists.
        m_list.setFlag(m_list.Blocked,        true);
        m_list.setState(m_list.DisabledState, false);
        m_info.setFlag(m_list.Blocked,        true);
        m_info.setState(m_list.DisabledState, false);
        m_timer->setInterval(500);
        m_optionButton.setState(m_optionButton.DisabledState, true);
        break;

     case Disabled:
        // Loading takes too long, disable both lists.
        m_list.setFlag(m_list.Blocked,        true);
        m_list.setState(m_list.DisabledState, true);
        m_info.setFlag(m_list.Blocked,        true);
        m_info.setState(m_list.DisabledState, true);
        m_info.clear();
        m_optionButton.setState(m_optionButton.DisabledState, true);
        break;
    }
}

inline void
client::screens::BrowserScreen::setList(client::widgets::FolderListbox::Items_t& items,
                                        ui::widgets::SimpleIconBox::Items_t& crumbs,
                                        size_t index,
                                        bool hasUp)
{
    // Update list and crumbs
    m_list.swapItems(items);
    m_list.setCurrentItem(0);
    m_list.setCurrentItem(index);
    m_crumbs.swapContent(crumbs, crumbs.size()-1);
    m_hasUp = hasUp;

    // Update info
    requestLoad();
    setState(WorkingLoad);
}

void
client::screens::BrowserScreen::onUpdate(const game::proxy::BrowserProxy::Info& info)
{
    afl::string::Translator& tx = m_translator;

    client::widgets::FolderListbox::Items_t items;
    ui::widgets::SimpleIconBox::Items_t crumbs;
    size_t index = 0;
    bool hasUp = false;

    typedef client::widgets::FolderListbox::Item Item;
    typedef ui::widgets::SimpleIconBox::Item SItem;
    using game::browser::Folder;
    using client::widgets::FolderListbox;

    crumbs.push_back(SItem(tx("[Places]")));
    for (size_t i = 0; i < info.path.size(); ++i) {
        crumbs.push_back(SItem(info.path[i].name));
    }
    if (!info.path.empty()) {
        items.push_back(Item(tx("[Go up one level]"), 0, true, FolderListbox::iUp));
        hasUp = true;
    }
    for (size_t i = 0; i < info.content.size(); ++i) {
        FolderListbox::Icon icon = FolderListbox::iFolder;
        switch (info.content[i].kind) {
         case Folder::kRoot:          icon = FolderListbox::iRoot;           break;
         case Folder::kFolder:        icon = FolderListbox::iFolder;         break;
         case Folder::kAccount:       icon = FolderListbox::iAccount;        break;
         case Folder::kLocal:         icon = FolderListbox::iComputer;       break;
         case Folder::kGame:          icon = FolderListbox::iGame;           break;
         case Folder::kFavorite:      icon = FolderListbox::iFavorite;       break;
         case Folder::kFavoriteList:  icon = FolderListbox::iFavoriteFolder; break;
        }
        items.push_back(Item(info.content[i].name, 0, info.content[i].canEnter, icon));
    }

    size_t n;
    if (info.index.get(n) && n < info.content.size()) {
        index = n + hasUp;
    }

    setList(items, crumbs, index, hasUp);
}

void
client::screens::BrowserScreen::onSelectedInfoUpdate(game::proxy::BrowserProxy::OptionalIndex_t index, const game::proxy::BrowserProxy::FolderInfo& info)
{
    // Headings
    PtrVector<InfoItem> out;
    if (!info.title.empty()) {
        out.pushBackNew(new InfoItem(Text(info.title).withStyle(StyleAttribute::Big).withStyle(StyleAttribute::Bold), String_t(), NoAction, 0));
    }
    if (!info.subtitle.empty()) {
        out.pushBackNew(new InfoItem(info.subtitle, String_t(), NoAction, 0));
    }

    // Players
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        if (info.availablePlayers.contains(i)) {
            Text text = String_t(Format("%c - %s", game::PlayerList::getCharacterFromPlayer(i), info.playerNames.get(i)));
            String_t extra = info.playerExtra.get(i);
            if (!extra.empty()) {
                text += Text("\n" + extra).withColor(util::SkinColor::Faded);
            }
            out.pushBackNew(new InfoItem(text, String_t(), PlayAction, i));
        }
    }

    // Enter folder
    if (info.canEnter) {
        out.pushBackNew(new InfoItem(m_translator("Change into this folder"), String_t(), FolderAction, 0));
    }

    // Other actions
    Actions_t as = getActions(info.possibleActions);
    bool hasSetup = false;
    bool hasActions = false;
    for (size_t i = 0; i < countof(ACTION_INFO); ++i) {
        if (as.contains(ACTION_INFO[i].action)) {
            if (ACTION_INFO[i].isAction) {
                hasActions = true;
            } else {
                hasSetup = true;
            }
        }
    }
    if (hasSetup) {
        if (hasActions) {
            out.pushBackNew(new InfoItem(m_translator("Setup & Actions..."), String_t(), RootAction, 0));
        } else {
            out.pushBackNew(new InfoItem(m_translator("Setup..."), String_t(), RootAction, 0));
        }
    } else {
        if (hasActions) {
            out.pushBackNew(new InfoItem(m_translator("Actions..."), String_t(), RootAction, 0));
        }
    }

    // Publish
    if (m_state == WorkingLoad) {
        if (getEffectiveIndex(m_list.getCurrentItem()).isSame(index)) {
            m_infoItems.swap(out);
            m_infoIndex = m_list.getCurrentItem();
            buildInfo();
            setState(Working);
        } else {
            requestLoad();
            setState(WorkingLoad);
        }
    }
    m_infoActions = info.possibleActions;
}

void
client::screens::BrowserScreen::buildInfo()
{
    m_info.clear();
    for (size_t i = 0; i < m_infoItems.size(); ++i) {
        bool enableThis = false;
        switch (m_infoItems[i]->action) {
         case PlayAction:
         case FolderAction:
         case RootAction:
            enableThis = true;
            break;
         case NoAction:
            enableThis = false;
            break;
        }
        m_info.addItem(m_infoItems[i]->text, 0 /*FIXME: use image*/, enableThis);
    }
    m_info.setCurrentItem(0, m_info.GoDown);
}
