/**
  *  \file client/screens/browserscreen.cpp
  */

#include <vector>
#include "client/screens/browserscreen.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/observable.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/directoryselectiondialog.hpp"
#include "client/dialogs/folderconfigdialog.hpp"
#include "client/downlink.hpp"
#include "client/imageloader.hpp"
#include "client/usercallback.hpp"
#include "client/widgets/busyindicator.hpp"
#include "game/browser/account.hpp"
#include "game/playerlist.hpp"
#include "game/root.hpp"
#include "game/turnloader.hpp"
#include "gfx/complex.hpp"
#include "gfx/rgbapixmap.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/statictext.hpp"
#include "ui/skincolorscheme.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/radiobutton.hpp"
#include "ui/widgets/richlistbox.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/widgets/transparentwindow.hpp"
#include "ui/window.hpp"
#include "util/rich/parser.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/translation.hpp"

using afl::base::Ptr;
using afl::container::PtrVector;
using game::browser::Session;
using util::rich::StyleAttribute;
using util::rich::Text;

namespace {
    class NewAccountDialog {
     public:
        NewAccountDialog(ui::Root& root)
            : m_typeValue(0),
              m_userInput(1000, 30, root),
              m_typePlanetsCentral(root, 'p', "PlanetsCentral", m_typeValue, 0),
              m_typeNu            (root, 'n', "planets.nu",     m_typeValue, 1),
              m_hostInput(1000, 30, root),
              m_root(root),
              m_loop(root)
            {
                m_hostInput.setText("");
            }

        bool run()
            {
                afl::base::Deleter h;
                ui::Window win(_("Add Account"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                win.add(h.addNew(new ui::widgets::StaticText(_("User name:"), util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                win.add(m_userInput);
                win.add(h.addNew(new ui::widgets::StaticText(_("Type:"), util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                win.add(m_typePlanetsCentral);
                win.add(m_typeNu);
                win.add(h.addNew(new ui::widgets::StaticText(_("Address (empty for default):"), util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                win.add(m_hostInput);

                ui::widgets::FocusIterator& it = h.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical | ui::widgets::FocusIterator::Tab));
                it.add(m_userInput);
                it.add(m_typePlanetsCentral);
                it.add(m_typeNu);
                it.add(m_hostInput);
                win.add(it);

                ui::widgets::Button& btnOK     = h.addNew(new ui::widgets::Button(_("OK"), util::Key_Return, m_root));
                ui::widgets::Button& btnCancel = h.addNew(new ui::widgets::Button(_("Cancel"), util::Key_Escape, m_root));
                btnOK.sig_fire.add(this, &NewAccountDialog::onOK);
                btnCancel.sig_fire.addNewClosure(m_loop.makeStop(0));

                ui::Group& g(h.addNew(new ui::Group(ui::layout::HBox::instance5)));
                g.add(h.addNew(new ui::Spacer()));
                g.add(btnOK);
                g.add(btnCancel);

                ui::widgets::Quit q(m_root, m_loop);
                win.add(q);

                win.add(g);
                win.pack();
                m_root.centerWidget(win);
                m_root.add(win);
                return m_loop.run();
            }

        void onOK()
            {
                if (m_userInput.getText().empty()) {
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
                                               m_typeValue.get() == 0 ? "pcc" : "nu",
                                               m_hostInput.getText()));
            }

     private:
        afl::base::Observable<int> m_typeValue;
        ui::widgets::InputLine m_userInput;
        ui::widgets::RadioButton m_typePlanetsCentral;
        ui::widgets::RadioButton m_typeNu;
        ui::widgets::InputLine m_hostInput;
        ui::Root& m_root;
        ui::EventLoop m_loop;
    };

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

    static Actions_t getActions(const game::Root& root)
    {
        using game::Root;
        Root::Actions_t as = root.getPossibleActions();
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

    game::Root::Actions_t getPossibleActions(ui::Root& root, util::RequestSender<game::browser::Session> sender)
    {
        struct Querier : public util::Request<game::browser::Session> {
         public:
            virtual void handle(game::browser::Session& s)
                {
                    if (game::browser::Browser* b = s.browser().get()) {
                        afl::base::Ptr<game::Root> root = b->getSelectedRoot();
                        if (root.get() != 0) {
                            m_actions = root->getPossibleActions();
                        }
                    }
                }
            game::Root::Actions_t m_actions;
        };
        client::Downlink link(root);
        Querier q;
        link.call(sender, q);
        return q.m_actions;
    }

    bool checkLocalSetup(ui::Root& root, util::RequestSender<game::browser::Session> sender)
    {
        struct Querier : public util::Request<game::browser::Session> {
         public:
            Querier()
                : m_result(false)
                { }
            virtual void handle(game::browser::Session& s)
                {
                    using game::Root;
                    if (game::browser::Browser* b = s.browser().get()) {
                        afl::base::Ptr<Root> root = b->getSelectedRoot();
                        game::config::UserConfiguration* config = b->getSelectedConfiguration();
                        if (root.get() != 0 && config != 0) {
                            Root::Actions_t as = root->getPossibleActions();
                            if (as.contains(Root::aLocalSetup) && !as.contains(Root::aLoadEditable)) {
                                if (!as.contains(Root::aConfigureReadOnly) || !(*config)[game::config::UserConfiguration::Game_ReadOnly]()) {
                                    m_result = true;
                                }
                            }
                        }
                    }
                }
            bool m_result;
        };
        client::Downlink link(root);
        Querier q;
        link.call(sender, q);
        return q.m_result;
    }

    void setLocalDirectoryAutomatically(ui::Root& root, util::RequestSender<game::browser::Session> sender)
    {
        class Setter : public util::Request<game::browser::Session> {
         public:
            virtual void handle(game::browser::Session& session)
                {
                    // FIXME: the who-does-what is undecided here: need to save both the pcc2.ini and network.ini files
                    if (game::browser::Browser* b = session.browser().get()) {
                        b->setSelectedLocalDirectoryAutomatically();
                        b->updateConfiguration();
                    }
                    if (game::browser::AccountManager* a = session.accountManager().get()) {
                        a->save();
                    }
                }
        };
        client::Downlink link(root);
        Setter q;
        link.call(sender, q);
    }

    void setLocalDirectory(ui::Root& root, util::RequestSender<game::browser::Session> sender, const String_t& dirName)
    {
        class Setter : public util::Request<game::browser::Session> {
         public:
            Setter(const String_t& dirName)
                : m_dirName(dirName)
                { }
            virtual void handle(game::browser::Session& session)
                {
                    // FIXME: the who-does-what is undecided here: need to save both the pcc2.ini and network.ini files
                    if (game::browser::Browser* b = session.browser().get()) {
                        b->setSelectedLocalDirectoryName(m_dirName);
                        b->updateConfiguration();
                    }
                    if (game::browser::AccountManager* a = session.accountManager().get()) {
                        a->save();
                    }
                }
         private:
            String_t m_dirName;
        };
        client::Downlink link(root);
        Setter q(dirName);
        link.call(sender, q);
    }

    void setLocalDirectoryNone(ui::Root& root, util::RequestSender<game::browser::Session> sender)
    {
        class Setter : public util::Request<game::browser::Session> {
         public:
            virtual void handle(game::browser::Session& session)
                {
                    using game::config::UserConfiguration;
                    if (game::browser::Browser* b = session.browser().get()) {
                        if (UserConfiguration* config = b->getSelectedConfiguration()) {
                            (*config)[UserConfiguration::Game_ReadOnly].set(1);
                            b->updateConfiguration();
                        }
                    }
                }
        };
        client::Downlink link(root);
        Setter q;
        link.call(sender, q);
    }

    bool verifyLocalDirectory(ui::Root& root, util::RequestSender<game::browser::Session> sender, const String_t& dirName)
    {
        enum Result {
            Missing,
            Success,
            NotEmpty,
            NotWritable
        };
        class Verifier : public util::Request<game::browser::Session> {
         public:
            Verifier(const String_t& dirName)
                : m_dirName(dirName),
                  m_result(Missing)
                { }
            virtual void handle(game::browser::Session& session)
                {
                    if (game::browser::Browser* b = session.browser().get()) {
                        try {
                            afl::io::FileSystem& fs = b->fileSystem();
                            afl::base::Ref<afl::io::Directory> dir = fs.openDirectory(m_dirName);

                            // Try creating files
                            bool ok = false;
                            for (int i = 0; i < 1000; ++i) {
                                String_t fileName = afl::string::Format("_%d.tmp", i);
                                if (dir->openFileNT(fileName, afl::io::FileSystem::CreateNew).get() != 0) {
                                    dir->eraseNT(fileName);
                                    ok = true;
                                    break;
                                }
                            }
                            if (!ok) {
                                m_result = NotWritable;
                                return;
                            }

                            // Check content
                            afl::base::Ptr<afl::io::DirectoryEntry> e;
                            if (dir->getDirectoryEntries()->getNextElement(e)) {
                                m_result = NotEmpty;
                                return;
                            }

                            // Success
                            m_result = Success;
                        }
                        catch (...) {
                            m_result = Missing;
                        }
                    }
                }
            Result getResult() const
                { return m_result; }
         private:
            String_t m_dirName;
            Result m_result;
        };

        client::Downlink link(root);
        Verifier v(dirName);
        link.call(sender, v);

        bool ok = false;
        switch (v.getResult()) {
         case Missing:
            ui::dialogs::MessageBox(afl::string::Format(_("The directory \"%s\" is not accessible and cannot be used.").c_str(), dirName),
                                    _("Game Directory Setup"), root).doOkDialog();
            break;

         case Success:
            ok = true;
            break;

         case NotWritable:
            ui::dialogs::MessageBox(afl::string::Format(_("The directory \"%s\" is not writable and cannot be used.").c_str(), dirName),
                                    _("Game Directory Setup"), root).doOkDialog();
            break;

         case NotEmpty:
            ok = ui::dialogs::MessageBox(afl::string::Format(_("The directory \"%s\" is not empty. Use anyway?").c_str(), dirName),
                                         _("Game Directory Setup"), root).doYesNoDialog();
            break;
        }
        return ok;
    }

    afl::base::Ptr<gfx::Canvas> makeSubImage(afl::base::Ptr<gfx::Canvas> orig, int x, int y, int w, int h)
    {
        if (orig.get() == 0) {
            return 0;
        } else {
            // FIXME: we should be able to make a canvas compatible to the UI window
            afl::base::Ref<gfx::Canvas> pix = gfx::RGBAPixmap::create(w, h)->makeCanvas();
            pix->blit(gfx::Point(-x, -y), *orig, gfx::Rectangle(x, y, w, h));
            return pix.asPtr();
        }
    }
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
                            buildActionInfo(*root, info, t.translator());
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


    static void buildActionInfo(const game::Root& root, PtrVector<InfoItem>& info, afl::string::Translator& tx)
        {
            Actions_t as = getActions(root);
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
            // using game::Root;
            // Root::Actions_t as = root.getPossibleActions();
            // bool hasSetup = as.containsAnyOf(Root::Actions_t()
            //                                  + Root::aLocalSetup
            //                                  + Root::aConfigureCharset
            //                                  + Root::aConfigureFinished
            //                                  + Root::aConfigureReadOnly);
            // bool hasActions = as.containsAnyOf(Root::Actions_t()
            //                                    + Root::aSweep
            //                                    + Root::aUnpack
            //                                    + Root::aMaketurn);
            if (hasSetup) {
                if (hasActions) {
                    info.pushBackNew(new InfoItem(tx.translateString("Setup & Actions..."), String_t(), RootAction, 0));
                } else {
                    info.pushBackNew(new InfoItem(tx.translateString("Setup..."), String_t(), RootAction, 0));
                }
            } else {
                if (hasActions) {
                    info.pushBackNew(new InfoItem(tx.translateString("Actions..."), String_t(), RootAction, 0));
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
      m_optionButton(_("Ins - Add Account"), util::Key_Insert, root),
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
    m_info.setRenderFlag(m_info.UseBackgroundColorScheme, true);
}

int
client::screens::BrowserScreen::run(gfx::ColorScheme<util::SkinColor::Color>& parentColors)
{
    ui::widgets::TransparentWindow window(parentColors, ui::layout::VBox::instance5);

    ui::Group buttons(ui::layout::HBox::instance5);
    ui::widgets::Button btnExit(_("Exit"), util::Key_Escape, m_root);
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
client::screens::BrowserScreen::onRootAction(size_t index)
{
    // FIXME: clean this up
    struct Querier : public util::Request<game::browser::Session> {
     public:
        virtual void handle(game::browser::Session& s)
            {
                if (game::browser::Browser* b = s.browser().get()) {
                    afl::base::Ptr<game::Root> root = b->getSelectedRoot();
                    if (root.get() != 0) {
                        m_actions = getActions(*root);
                    }
                }
            }
        Actions_t m_actions;
    };
    Downlink link(m_root);
    Querier q;
    link.call(m_sender, q);

    ui::widgets::StringListbox list(m_root.provider(), m_root.colorScheme());
    for (size_t i = 0; i < countof(ACTION_INFO); ++i) {
        if (q.m_actions.contains(ACTION_INFO[i].action)) {
            list.addItem(ACTION_INFO[i].action, _(ACTION_INFO[i].name) + String_t("..."));
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
        client::dialogs::doFolderConfigDialog(m_root, m_sender, afl::string::Translator::getSystemInstance());
        break;
     case xSweep:
        break;
     case xUnpack:
        break;
     case xMaketurn:
        break;
    }
}

bool
client::screens::BrowserScreen::preparePlayAction(size_t /*index*/)
{
    if (checkLocalSetup(m_root, m_sender)) {
        ImageLoader loader(m_root);
        loader.loadImage("gamedirsetup");
        loader.wait();

        afl::base::Ptr<gfx::Canvas> pix = m_root.provider().getImage("gamedirsetup");

        const int WIDTH = 600; /* FIXME */
        ui::widgets::RichListbox box(m_root.provider(), m_root.colorScheme());
        box.setPreferredWidth(WIDTH);
        box.setRenderFlag(box.UseBackgroundColorScheme, true);
        box.addItem(util::rich::Parser::parseXml(_("<big>Automatic</big>\n"
                                                   "PCC2 will automatically assign a directory within your profile directory. "
                                                   "If unsure, choose this.")), makeSubImage(pix, 0, 0, 72, 64), true);
        box.addItem(util::rich::Parser::parseXml(_("<big>Manual</big>\n"
                                                   "Manually assign a directory. Use if you want to have full control.")), makeSubImage(pix, 0, 64, 72, 64), true);
        box.addItem(util::rich::Parser::parseXml(_("<big>None</big>\n"
                                                   "Do not assign a directory. The game will be opened for viewing only, and no changes can be saved.")), makeSubImage(pix, 0, 128, 72, 64), true);

        ui::Window window(_("Game Directory Setup"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
        ui::rich::StaticText intro(util::rich::Parser::parseXml(_("<font color=\"static\">This game does not yet have an associated game directory. "
                                                                  "PCC2 needs a directory on your computer to store configuration and history data. "
                                                                  "Please choose how the directory should be assigned.</font>")),
                                   WIDTH, m_root.provider());
        window.add(intro);
        window.add(box);

        ui::widgets::StandardDialogButtons btns(m_root);
        window.add(btns);
        window.pack();

        ui::EventLoop loop(m_root);
        btns.addStop(loop);
        box.requestFocus();

        m_root.centerWidget(window);
        m_root.add(window);
        if (!loop.run()) {
            return false;
        }
        m_root.remove(window);
        switch (box.getCurrentItem()) {
         case 0:
            // Auto
            setLocalDirectoryAutomatically(m_root, m_sender);
            break;

         case 1: {
            // Manual - FIXME
            String_t s;
            while (1) {
                if (!client::dialogs::doDirectorySelectionDialog(m_root, m_sender, s)) {
                    return false;
                }
                if (verifyLocalDirectory(m_root, m_sender, s)) {
                    break;
                }
            }
            setLocalDirectory(m_root, m_sender, s);
            break;
         }
         case 2:
            // None
            setLocalDirectoryNone(m_root, m_sender);
            break;
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
