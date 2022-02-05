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
#include "client/dialogs/helpdialog.hpp"
#include "client/downlink.hpp"
#include "client/imageloader.hpp"
#include "game/playerlist.hpp"
#include "game/root.hpp"
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
using afl::string::Format;
using game::proxy::BrowserProxy;
using ui::dialogs::MessageBox;
using util::rich::StyleAttribute;
using util::rich::Text;

namespace {
    class NewAccountDialog {
     public:
        NewAccountDialog(ui::Root& root, afl::string::Translator& tx)
            : m_typeValue(0),
              m_userInput(1000, 30, root),
              m_typePlanetsCentral(root, 'p', "PlanetsCentral", m_typeValue, 0),
              m_typeNu            (root, 'n', "planets.nu",     m_typeValue, 1),
              m_hostInput(1000, 30, root),
              m_root(root),
              m_translator(tx),
              m_loop(root)
            {
                m_hostInput.setText("");
            }

        bool run()
            {
                afl::string::Translator& tx = m_translator;
                afl::base::Deleter h;
                ui::Window win(tx("Add Account"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                win.add(h.addNew(new ui::widgets::StaticText(tx("User name:"), util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                win.add(m_userInput);
                win.add(h.addNew(new ui::widgets::StaticText(tx("Type:"), util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                win.add(m_typePlanetsCentral);
                win.add(m_typeNu);
                win.add(h.addNew(new ui::widgets::StaticText(tx("Address (empty for default):"), util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                win.add(m_hostInput);

                ui::widgets::FocusIterator& it = h.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical | ui::widgets::FocusIterator::Tab));
                it.add(m_userInput);
                it.add(m_typePlanetsCentral);
                it.add(m_typeNu);
                it.add(m_hostInput);
                win.add(it);

                ui::widgets::Button& btnOK     = h.addNew(new ui::widgets::Button(tx("OK"), util::Key_Return, m_root));
                ui::widgets::Button& btnCancel = h.addNew(new ui::widgets::Button(tx("Cancel"), util::Key_Escape, m_root));
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

        void submit(BrowserProxy& proxy)
            {
                String_t user = m_userInput.getText();
                String_t type = m_typeValue.get() == 0 ? "pcc" : "nu";
                String_t host = m_hostInput.getText();
                if (host.empty()) {
                    host = m_typeValue.get() == 0 ? "planetscentral.com" : "planets.nu";
                }
                client::Downlink link(m_root, m_translator);
                bool ok = proxy.addAccount(link, user, type, host);
                if (!ok) {
                    MessageBox(m_translator("An account with these parameters already exists."), m_translator("Add Account"), m_root)
                        .doOkDialog(m_translator);
                }
            }

     private:
        afl::base::Observable<int> m_typeValue;
        ui::widgets::InputLine m_userInput;
        ui::widgets::RadioButton m_typePlanetsCentral;
        ui::widgets::RadioButton m_typeNu;
        ui::widgets::InputLine m_hostInput;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
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

    bool verifyLocalDirectory(ui::Root& root, afl::string::Translator& tx, util::RequestSender<afl::io::FileSystem> sender, const String_t& dirName)
    {
        enum Result {
            Missing,
            Success,
            NotEmpty,
            NotWritable
        };
        class Verifier : public util::Request<afl::io::FileSystem> {
         public:
            Verifier(const String_t& dirName)
                : m_dirName(dirName),
                  m_result(Missing)
                { }
            virtual void handle(afl::io::FileSystem& fs)
                {
                    try {
                        afl::base::Ref<afl::io::Directory> dir = fs.openDirectory(m_dirName);

                        // Try creating files
                        bool ok = false;
                        for (int i = 0; i < 1000; ++i) {
                            String_t fileName = Format("_%d.tmp", i);
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
            Result getResult() const
                { return m_result; }
         private:
            String_t m_dirName;
            Result m_result;
        };

        client::Downlink link(root, tx);
        Verifier v(dirName);
        link.call(sender, v);

        bool ok = false;
        switch (v.getResult()) {
         case Missing:
            MessageBox(Format(tx("The directory \"%s\" is not accessible and cannot be used.").c_str(), dirName),
                       tx("Game Directory Setup"), root).doOkDialog(tx);
            break;

         case Success:
            ok = true;
            break;

         case NotWritable:
            MessageBox(Format(tx("The directory \"%s\" is not writable and cannot be used.").c_str(), dirName),
                       tx("Game Directory Setup"), root).doOkDialog(tx);
            break;

         case NotEmpty:
            ok = MessageBox(Format(tx("The directory \"%s\" is not empty. Use anyway?").c_str(), dirName),
                            tx("Game Directory Setup"), root).doYesNoDialog(tx);
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

/***************************** BrowserScreen *****************************/

client::screens::BrowserScreen::BrowserScreen(ui::Root& root, afl::string::Translator& tx, game::proxy::BrowserProxy& proxy, util::RequestSender<game::Session> gameSender)
    : m_root(root),
      m_translator(tx),
      m_gameSender(gameSender),
      m_receiver(root.engine().dispatcher(), *this),
      m_proxy(proxy),
      m_list(gfx::Point(20, 20), m_root),
      m_crumbs(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(40, 1), m_root),
      m_info(root.provider(), root.colorScheme()),
      m_optionButton(tx("Ins - Add Account"), util::Key_Insert, root),
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
        NewAccountDialog dlg(m_root, m_translator);
        if (dlg.run()) {
            dlg.submit(m_proxy);

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
        client::dialogs::doFolderConfigDialog(m_root, m_proxy, afl::string::Translator::getSystemInstance());
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
    afl::string::Translator& tx = m_translator;
    if (checkLocalSetup(m_root, tx, m_proxy)) {
        ImageLoader loader(m_root, m_translator);
        loader.loadImage("gamedirsetup");
        loader.wait();

        afl::base::Ptr<gfx::Canvas> pix = m_root.provider().getImage("gamedirsetup");

        const int WIDTH = 600; /* FIXME */
        ui::widgets::RichListbox box(m_root.provider(), m_root.colorScheme());
        box.setPreferredWidth(WIDTH);
        box.setRenderFlag(box.UseBackgroundColorScheme, true);
        box.addItem(util::rich::Parser::parseXml(tx("<big>Automatic</big>\n"
                                                    "PCC2 will automatically assign a directory within your profile directory. "
                                                    "If unsure, choose this.")), makeSubImage(pix, 0, 0, 72, 64), true);
        box.addItem(util::rich::Parser::parseXml(tx("<big>Manual</big>\n"
                                                    "Manually assign a directory. Use if you want to have full control.")), makeSubImage(pix, 0, 64, 72, 64), true);
        box.addItem(util::rich::Parser::parseXml(tx("<big>None</big>\n"
                                                    "Do not assign a directory. The game will be opened for viewing only, and no changes can be saved.")), makeSubImage(pix, 0, 128, 72, 64), true);

        ui::Window window(tx("Game Directory Setup"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
        ui::rich::StaticText intro(util::rich::Parser::parseXml(tx("<font color=\"static\">This game does not yet have an associated game directory. "
                                                                   "PCC2 needs a directory on your computer to store configuration and history data. "
                                                                   "Please choose how the directory should be assigned.</font>")),
                                   WIDTH, m_root.provider());
        window.add(intro);
        window.add(box);

        ui::widgets::StandardDialogButtons btns(m_root, tx);
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
            m_proxy.setLocalDirectoryAutomatically();
            break;

         case 1: {
            // Manual - FIXME
            String_t s;
            while (1) {
                if (!client::dialogs::doDirectorySelectionDialog(m_root, m_translator, m_proxy.fileSystem(), s)) {
                    return false;
                }
                if (verifyLocalDirectory(m_root, m_translator, m_proxy.fileSystem(), s)) {
                    break;
                }
            }
            m_proxy.setLocalDirectoryName(s);
            break;
         }
         case 2:
            // None
            m_proxy.setLocalDirectoryNone();
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
