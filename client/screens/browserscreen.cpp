/**
  *  \file client/screens/browserscreen.cpp
  */

#include <vector>
#include "client/screens/browserscreen.hpp"
#include "client/usercallback.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/button.hpp"
#include "ui/window.hpp"
#include "ui/layout/hbox.hpp"
#include "util/rich/styleattribute.hpp"
#include "game/root.hpp"
#include "game/playerlist.hpp"
#include "afl/string/format.hpp"
#include "ui/skincolorscheme.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/statictext.hpp"
#include "game/browser/account.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "game/turnloader.hpp"

using afl::base::Ptr;
using afl::container::PtrVector;
using game::browser::Session;
using util::rich::StyleAttribute;
using util::rich::Text;

namespace {
    class NewAccountDialog {
     public:
        NewAccountDialog(ui::Root& root)
            : m_userInput(1000, 30, root),
              m_typeInput(1000, 30, root),
              m_hostInput(1000, 30, root),
              m_root(root),
              m_loop(root)
            {
                m_typeInput.setText("pcc");
                m_hostInput.setText("");
            }

        bool run()
            {
                afl::base::Deleter h;
                ui::Window win("!New Account", m_root.provider(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                win.add(h.addNew(new ui::widgets::StaticText("!User name:", util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                win.add(m_userInput);
                win.add(h.addNew(new ui::widgets::StaticText("!Type (\"pcc\" or \"nu\"):", util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                win.add(m_typeInput);
                win.add(h.addNew(new ui::widgets::StaticText("!Address (empty for default):", util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                win.add(m_hostInput);

                ui::widgets::FocusIterator& it = h.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical | ui::widgets::FocusIterator::Tab));
                it.add(m_userInput);
                it.add(m_typeInput);
                it.add(m_hostInput);
                win.add(it);

                ui::widgets::Button& btnOK     = h.addNew(new ui::widgets::Button("!OK", util::Key_Return, m_root.provider(), m_root.colorScheme()));
                ui::widgets::Button& btnCancel = h.addNew(new ui::widgets::Button("!Cancel", util::Key_Escape, m_root.provider(), m_root.colorScheme()));
                btnOK.sig_fire.add(this, &NewAccountDialog::onOK);
                btnCancel.sig_fire.addNewClosure(m_loop.makeStop(0));

                ui::Group& g(h.addNew(new ui::Group(ui::layout::HBox::instance5)));
                g.add(h.addNew(new ui::Spacer()));
                g.add(btnOK);
                g.add(btnCancel);

                win.add(g);
                win.pack();
                m_root.centerWidget(win);
                m_root.add(win);
                return m_loop.run();
            }

        void onOK()
            {
                if (m_userInput.getText().empty() || (m_typeInput.getText() != "nu" && m_typeInput.getText() != "pcc")) {
                    // Failure
                } else {
                    m_loop.stop(1);
                }
            }

        void submit(util::RequestSender<game::browser::Session> sender)
            {
                class Task : public util::Request<game::browser::Session> {
                 public:
                    Task(String_t user, String_t type, String_t host)
                        : m_user(user), m_type(type), m_host(host)
                        { }
                    void handle(game::browser::Session& session)
                        {
                            // FIXME:
                            if (m_host.empty()) {
                                if (m_type == "nu") {
                                    m_host = "planets.nu";
                                } else if (m_type == "pcc") {
                                    m_host = "planetscentral.com";
                                }
                            }
                            if (game::browser::AccountManager* mgr = session.accountManager().get()) {
                                if (mgr->findAccount(m_user, m_type, m_host) != 0) {
                                    // FIXME: handle duplicate
                                } else {
                                    // New account
                                    std::auto_ptr<game::browser::Account> acc(new game::browser::Account());
                                    acc->setName(afl::string::Format("%s @ %s", m_user, m_host));
                                    acc->setUser(m_user);
                                    acc->setType(m_type);
                                    acc->setHost(m_host);
                                    mgr->addNewAccount(acc.release());
                                    mgr->save();
                                }
                            }
                        }
                 private:
                    String_t m_user;
                    String_t m_type;
                    String_t m_host;
                };
                sender.postNewRequest(new Task(m_userInput.getText(),
                                               m_typeInput.getText(),
                                               m_hostInput.getText()));
            }

     private:
        ui::widgets::InputLine m_userInput;
        ui::widgets::InputLine m_typeInput;
        ui::widgets::InputLine m_hostInput;
        ui::Root& m_root;
        ui::EventLoop m_loop;
    };
}

/*
 *  Worker -> UI: Update "Info" panel
 */
class client::screens::BrowserScreen::UpdateInfoTask : public util::Request<BrowserScreen> {
 public:
    UpdateInfoTask(bool current, size_t pos, PtrVector<InfoItem>& info)
        : m_current(current),
          m_pos(pos),
          m_info()
        {
            m_info.swap(info);
        }

    virtual void handle(BrowserScreen& t)
        {
            if (m_current) {
                t.setCurrentInfo(m_info);
            } else {
                t.setChildInfo(m_pos, m_info);
            }
        }
 private:
    bool m_current;
    size_t m_pos;
    PtrVector<InfoItem> m_info;
};

/*
 *  UI -> Worker: Obtain information for "Info" panel. Fires back an UpdateInfoTask.
 */
class client::screens::BrowserScreen::LoadTask : public util::Request<Session> {
 public:
    LoadTask(bool current, size_t pos, util::RequestSender<BrowserScreen> reply)
        : m_current(current),
          m_pos(pos),
          m_reply(reply)
        { }
    void handle(Session& t)
        {
            if (game::browser::Browser* b = t.browser().get()) {
                game::browser::Folder* f;
                if (m_current) {
                    f = &b->currentFolder();
                } else {
                    if (m_pos < b->content().size()) {
                        f = b->content()[m_pos];
                    } else {
                        f = 0;
                    }
                }

                PtrVector<InfoItem> info;
                if (f != 0) {
                    // Info
                    if (!m_current) {
                        b->selectChild(m_pos);
                        b->loadChildRoot();
                        Ptr<game::Root> root = b->getSelectedRoot();
                        Ptr<game::TurnLoader> loader = root.get() != 0 ? root->getTurnLoader() : 0;
                        if (loader.get() != 0) {
                            const game::config::StringOption& name = root->hostConfiguration()[game::config::HostConfiguration::GameName];
                            info.pushBackNew(new InfoItem(Text(name.wasSet() ? name() : f->getName()).withStyle(StyleAttribute::Big).withStyle(StyleAttribute::Bold),
                                                          String_t(),
                                                          NoAction, 0));
                            info.pushBackNew(new InfoItem(Text(afl::string::Format(t.translator().translateString("A %s game").c_str(),
                                                                                   root->hostVersion().toString(t.translator()))),
                                                          String_t(),
                                                          NoAction, 0));
                            buildPlayerList(*root, *loader, info, t.translator());
                            if (f->canEnter()) {
                                info.pushBackNew(new InfoItem(t.translator().translateString("Change into this folder"), String_t(), FolderAction, 0));
                            }
                        } else {
                            buildFolderInfo(*f, info);
                        }
                    } else {
                        // FIXME: unselect in browser?
                        buildFolderInfo(*f, info);
                    }
                }

                m_reply.postNewRequest(new UpdateInfoTask(m_current, m_pos, info));
            }
        }

 private:
    static void buildFolderInfo(game::browser::Folder& f, PtrVector<InfoItem>& info)
        {
            info.pushBackNew(new InfoItem(Text(f.getName()).withStyle(StyleAttribute::Big).withStyle(StyleAttribute::Bold), String_t(), NoAction, 0));

            Text desc = f.getDescription();
            if (!desc.empty()) {
                info.pushBackNew(new InfoItem(desc, String_t(), NoAction, 0));
            }
        }

    static void buildPlayerList(const game::Root& root, game::TurnLoader& loader, PtrVector<InfoItem>& info, afl::string::Translator& tx)
        {
            const game::PlayerList& pl = root.playerList();
            for (game::Player* p = pl.getFirstPlayer(); p != 0; p = pl.getNextPlayer(p)) {
                String_t extra;
                game::TurnLoader::PlayerStatusSet_t st = loader.getPlayerStatus(p->getId(), extra, tx);
                if (!st.empty()) {
                    Text text = String_t(afl::string::Format("%c - %s",
                                                             pl.getCharacterFromPlayer(p->getId()),
                                                             p->getName(game::Player::ShortName)));
                    if (!extra.empty()) {
                        text += Text("\n" + extra).withColor(util::SkinColor::Faded);
                    }
                    info.pushBackNew(new InfoItem(text, String_t(), PlayAction, p->getId()));
                }
            }
        }

    bool m_current;
    size_t m_pos;
    util::RequestSender<BrowserScreen> m_reply;
};

/*
 *  Worker -> UI: Update browser list
 */
class client::screens::BrowserScreen::UpdateTask : public util::Request<BrowserScreen> {
 public:
    UpdateTask(const afl::container::PtrVector<game::browser::Folder>& path,
               const afl::container::PtrVector<game::browser::Folder>& content,
               game::browser::Browser::OptionalIndex_t index,
               afl::string::Translator& tx)
        : m_items(),
          m_crumbs(),
          m_index(0),
          m_hasUp(false)
        {
            typedef client::widgets::FolderListbox::Item Item;
            typedef ui::widgets::SimpleIconBox::Item SItem;
            using game::browser::Folder;
            using client::widgets::FolderListbox;

            m_crumbs.push_back(SItem(tx.translateString("[Places]")));
            for (size_t i = 0; i < path.size(); ++i) {
                m_crumbs.push_back(SItem(path[i]->getName()));
            }
            if (!path.empty()) {
                m_items.push_back(Item(tx.translateString("[Go up one level]"), 0, true, FolderListbox::iUp));
                m_hasUp = true;
            }
            for (size_t i = 0; i < content.size(); ++i) {
                FolderListbox::Icon icon = FolderListbox::iFolder;
                switch (content[i]->getKind()) {
                 case Folder::kRoot:          icon = FolderListbox::iRoot;           break;
                 case Folder::kFolder:        icon = FolderListbox::iFolder;         break;
                 case Folder::kAccount:       icon = FolderListbox::iAccount;        break;
                 case Folder::kLocal:         icon = FolderListbox::iComputer;       break;
                 case Folder::kGame:          icon = FolderListbox::iGame;           break;
                 case Folder::kFavorite:      icon = FolderListbox::iFavorite;       break;
                 case Folder::kFavoriteList:  icon = FolderListbox::iFavoriteFolder; break;
                }
                m_items.push_back(Item(content[i]->getName(), 0, content[i]->canEnter(), icon));
            }

            size_t n;
            if (index.get(n) && n < content.size()) {
                m_index = n + m_hasUp;
            }
        }
    void handle(BrowserScreen& dlg)
        { dlg.setList(m_items, m_crumbs, m_index, m_hasUp); }
 private:
    client::widgets::FolderListbox::Items_t m_items;
    ui::widgets::SimpleIconBox::Items_t m_crumbs;
    size_t m_index;
    bool m_hasUp;
};

/*
 *  UI -> Worker: obtain current content. Fires back an UpdateTask.
 */
class client::screens::BrowserScreen::InitTask : public util::Request<Session> {
 public:
    InitTask(util::RequestSender<BrowserScreen> reply)
        : m_reply(reply)
        { }
    void handle(Session& t)
        {
            std::auto_ptr<game::browser::Browser>& b = t.browser();
            b->loadContent();
            m_reply.postNewRequest(new UpdateTask(b->path(), b->content(), b->getSelectedChild(), t.translator()));
        }
 private:
    util::RequestSender<BrowserScreen> m_reply;
};

/*
 *  UI -> Worker: enter subdirectory. Fires back an UpdateTask.
 */
class client::screens::BrowserScreen::EnterTask : public util::Request<Session> {
 public:
    EnterTask(size_t nr, util::RequestSender<BrowserScreen> reply)
        : m_number(nr),
          m_reply(reply)
        { }
    void handle(Session& t)
        {
            if (game::browser::Browser* b = t.browser().get()) {
                b->openChild(m_number);
                b->loadContent();
                m_reply.postNewRequest(new UpdateTask(b->path(), b->content(), b->getSelectedChild(), t.translator()));
            }
        }
 private:
    size_t m_number;
    util::RequestSender<BrowserScreen> m_reply;
};

/*
 *  UI -> Worker: enter parent directory. Fires back an UpdateTask.
 */
class client::screens::BrowserScreen::UpTask : public util::Request<Session> {
 public:
    UpTask(size_t nr, util::RequestSender<BrowserScreen> reply)
        : m_number(nr),
          m_reply(reply)
        { }
    void handle(Session& t)
        {
            if (game::browser::Browser* b = t.browser().get()) {
                for (size_t i = 0; i < m_number; ++i) {
                    b->openParent();
                }
                b->loadContent();
                m_reply.postNewRequest(new UpdateTask(b->path(), b->content(), b->getSelectedChild(), t.translator()));
            }
        }
 private:
    size_t m_number;
    util::RequestSender<BrowserScreen> m_reply;
};


/***************************** BrowserScreen *****************************/

client::screens::BrowserScreen::BrowserScreen(ui::Root& root, util::RequestSender<game::browser::Session> sender)
    : m_root(root),
      m_sender(sender),
      m_receiver(root.engine().dispatcher(), *this),
      m_list(gfx::Point(20, 20), m_root),
      m_crumbs(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(40, 1), m_root),
      m_info(root.provider(), root.colorScheme()),
      m_optionButton("!Ins - Add Account", util::Key_Insert, root.provider(), root.colorScheme()),
      m_infoItems(),
      m_infoIndex(0),
      m_loop(root),
      m_hasUp(false),
      m_state(Working),
      m_blockState(false),
      m_timer(root.engine().createTimer())
{
    m_crumbs.setChangeOnClick(true);
    m_timer->sig_fire.add(this, &BrowserScreen::onTimer);
    m_list.sig_change.add(this, &BrowserScreen::onListMoved);
    m_list.sig_itemDoubleClick.add(this, &BrowserScreen::onItemDoubleClicked);
    m_crumbs.sig_change.add(this, &BrowserScreen::onCrumbClicked);
    m_optionButton.sig_fire.add(this, &BrowserScreen::onAddAccount);
}

int
client::screens::BrowserScreen::run()
{
#if 1
    ui::Window window("!Select", m_root.provider(), ui::BLUE_BLACK_WINDOW, ui::layout::VBox::instance5);
#else
    ui::Group window(ui::layout::VBox::instance5);
    ui::SkinColorScheme colorScheme(ui::BLACK_COLOR_SET, m_root.colorScheme());
    window.setColorScheme(colorScheme);
#endif

    ui::Group buttons(ui::layout::HBox::instance5);
    ui::widgets::Button btnExit("!Exit", util::Key_Escape, m_root.provider(), m_root.colorScheme());
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
    keys.add(util::Key_F1,                       this, &BrowserScreen::onKeyHelp);
    // keys.add(util::Key_F5,                    this, &BrowserScreen::onKeyPlugin);
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

    UserCallback cb(m_root, m_sender);

    m_sender.postNewRequest(new InitTask(m_receiver.getSender()));
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

// Get sender.
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

void
client::screens::BrowserScreen::requestLoad()
{
    size_t current = m_list.getCurrentItem();
    m_info.clear();
    m_sender.postNewRequest(new LoadTask(isUpLink(current), getIndex(current), m_receiver.getSender()));
}

void
client::screens::BrowserScreen::onItemDoubleClicked(size_t nr)
{
    switch (m_state) {
     case Working:
     case WorkingLoad:
        if (isUpLink(nr)) {
            m_sender.postNewRequest(new UpTask(1, m_receiver.getSender()));
            setState(Blocked);
        } else {
            if (const client::widgets::FolderListbox::Item* p = m_list.getItem(nr)) {
                if (p->canEnter) {
                    m_sender.postNewRequest(new EnterTask(getIndex(nr), m_receiver.getSender()));
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
            m_sender.postNewRequest(new UpTask(n - nr - 1, m_receiver.getSender()));
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
                // FIXME: PCC2 does Unpack when it is applicable. How do we implement that?
                sig_gameSelection.raise(m_infoItems[infoIndex]->actionParameter);
                break;

             case FolderAction:
                onItemDoubleClicked(m_list.getCurrentItem());
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
    // FIXME: port
    //    doHelp("pcc2:gamesel");
}

void
client::screens::BrowserScreen::onKeyQuit(int)
{
    // ex PCC2GameChooserWindow::handleEvent (part)
    m_loop.stop(0);
    // FIXME: port
    //    putKey(vk_Quit);
}

void
client::screens::BrowserScreen::onAddAccount(int)
{
    if (m_state == Working) {
        NewAccountDialog dlg(m_root);
        if (dlg.run()) {
            dlg.submit(m_sender);

            // Refresh. UpTask(0) is a no-op.
            m_sender.postNewRequest(new UpTask(0, m_receiver.getSender()));
            setState(Blocked);
        }
    }
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
client::screens::BrowserScreen::setCurrentInfo(afl::container::PtrVector<InfoItem>& info)
{
    if (m_state == WorkingLoad) {
        size_t current = m_list.getCurrentItem();
        if (isUpLink(current)) {
            m_infoItems.swap(info);
            m_infoIndex = current;
            buildInfo();
            setState(Working);
        } else {
            requestLoad();
            setState(WorkingLoad);
        }
    }
}

void
client::screens::BrowserScreen::setChildInfo(size_t pos, afl::container::PtrVector<InfoItem>& info)
{
    if (m_state == WorkingLoad) {
        size_t current = m_list.getCurrentItem();
        if (!isUpLink(current) && getIndex(current) == pos) {
            m_infoItems.swap(info);
            m_infoIndex = current;
            buildInfo();
            setState(Working);
        } else {
            requestLoad();
            setState(WorkingLoad);
        }
    }
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
