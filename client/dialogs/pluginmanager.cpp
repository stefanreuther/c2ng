/**
  *  \file client/dialogs/pluginmanager.cpp
  *  \brief Class client::dialogs::PluginManager
  */

#include "client/dialogs/pluginmanager.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/sessionfileselectiondialog.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/plugininfo.hpp"
#include "client/widgets/pluginlist.hpp"
#include "game/proxy/pluginmanagerproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/filenamepattern.hpp"
#include "util/rich/parser.hpp"

using afl::string::Format;
using client::widgets::PluginInfo;
using client::widgets::PluginList;
using client::widgets::HelpWidget;
using game::proxy::PluginManagerProxy;
using ui::dialogs::MessageBox;
using ui::widgets::Button;
using ui::widgets::FrameGroup;
using ui::widgets::ScrollbarContainer;
using ui::widgets::StaticText;
using util::KeyString;
using util::rich::Parser;
using util::rich::StyleAttribute;
using util::rich::Text;

namespace {
    /* Dialog class */
    class Dialog {
     public:
        Dialog(client::dialogs::PluginManager& parent);

        void run();
        bool isRestart() const;

     private:
        void onList(const PluginManagerProxy::Infos_t& list);
        void onDetails(const PluginManagerProxy::Details_t& d);
        void onMove();

        void onAdd();
        void onRemove();

        void doAdd(game::proxy::WaitIndicator& ind, const String_t& fileName);
        void doRemove(String_t id);

        void showError(const String_t& errorMessage, const String_t& conclusio, const Text& defaultMessage);

        client::dialogs::PluginManager& m_parent;
        PluginManagerProxy m_proxy;

        PluginList m_list;
        PluginInfo m_info;
        ui::EventLoop m_loop;
        bool m_restart;
    };
}

/*
 *  Dialog
 */

Dialog::Dialog(client::dialogs::PluginManager& parent)
    : m_parent(parent),
      m_proxy(parent.gameSender(), parent.root().engine().dispatcher()),
      m_list(parent.root(), parent.translator()),
      m_info(parent.root(), parent.translator()),
      m_loop(parent.root()),
      m_restart(false)
{
    m_proxy.sig_list.add(this, &Dialog::onList);
    m_proxy.sig_details.add(this, &Dialog::onDetails);
    m_list.sig_change.add(this, &Dialog::onMove);
}

void
Dialog::run()
{
    // ex WPluginDialog::init()
    // Window (VBox)
    //   HBox
    //     VBox
    //       "Plugins:"
    //       FrameGroup / ScrollbarContainer / PluginList
    //     PluginInfo
    //   HBox [Add, Delete <> Help, Close]
    afl::string::Translator& tx = m_parent.translator();
    ui::Root& root = m_parent.root();

    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(tx("Plugins"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    ui::Group& g1  = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Group& g11 = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::Group& g2  = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g11.add(del.addNew(new StaticText(tx("Plugins:"), util::SkinColor::Static, "+", root.provider())));
    g11.add(FrameGroup::wrapWidget(del, root.colorScheme(), ui::LoweredFrame, del.addNew(new ScrollbarContainer(m_list, root))));
    g1.add(g11);
    g1.add(m_info);
    win.add(g1);

    // Buttons
    Button& btnAdd    = del.addNew(new Button(tx("Add"), util::Key_Insert, root));
    Button& btnRemove = del.addNew(new Button(tx("Remove"), util::Key_Delete, root));
    Button& btnHelp   = del.addNew(new Button(tx("Help"), 'h', root));
    Button& btnClose  = del.addNew(new Button(tx("Close"), util::Key_Escape, root));
    g2.add(btnAdd);
    g2.add(btnRemove);
    g2.add(del.addNew(new ui::Spacer()));
    g2.add(btnHelp);
    g2.add(btnClose);
    win.add(g2);

    // Events
    ui::Widget& help = del.addNew(new HelpWidget(root, tx, m_parent.gameSender(), "pcc2:plugins"));
    win.add(help);
    btnHelp.dispatchKeyTo(help);
    win.add(del.addNew(new ui::widgets::Quit(root, m_loop)));

    btnAdd.sig_fire.add(this, &Dialog::onAdd);
    btnRemove.sig_fire.add(this, &Dialog::onRemove);
    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));

    // Start it
    m_list.requestFocus();
    m_proxy.requestList();

    win.pack();
    root.centerWidget(win);
    root.add(win);
    m_loop.run();
}

bool
Dialog::isRestart() const
{
    return m_restart;
}

/** Proxy: update list content. */
void
Dialog::onList(const PluginManagerProxy::Infos_t& list)
{
    // setContent will trigger onMove.
    m_list.setContent(list);
}

/** Proxy: update plugin details. */
void
Dialog::onDetails(const PluginManagerProxy::Details_t& d)
{
    m_info.setContent(d);
}

/** List: handle movement. Request new details. */
void
Dialog::onMove()
{
    // m_info will internally schedule redraw to avoid flicker.
    // requestDetails will trigger onDetails().
    m_info.setLoading();
    if (const PluginList::Info_t* p = m_list.getCurrentPlugin()) {
        m_proxy.requestDetails(p->id);
    }
}

/** User action: add new plugin. */
void
Dialog::onAdd()
{
    // WPluginDialog::onAdd()
    afl::string::Translator& tx = m_parent.translator();
    client::Downlink link(m_parent.root(), tx);

    // Ask for file
    HelpWidget help(m_parent.root(), tx, m_parent.gameSender(), "pcc2:plugins");
    client::dialogs::SessionFileSelectionDialog fc(m_parent.root(), tx, m_parent.gameSender(), tx("Install Plugin"));
    fc.setPattern(util::FileNamePattern::getAllFilesWithExtensionPattern("c2p"));
    fc.addPattern(util::FileNamePattern::getAllFilesWithExtensionPattern("q"));
    fc.addPattern(util::FileNamePattern::getAllFilesWithExtensionPattern("res"));
    fc.addPattern(util::FileNamePattern::getAllFilesWithExtensionPattern("zip"));
    fc.addPattern(util::FileNamePattern::getAllFilesWithExtensionPattern("c2z"));
    fc.setDefaultExtension("c2p");
    fc.setHelpWidget(help);

    if (fc.runDefault(link)) {
        doAdd(link, fc.getResult());
        m_proxy.requestList();
        m_proxy.cancelInstallation();
    }
}

/** User action: remove current plugin. */
void
Dialog::onRemove()
{
    // WPluginDialog::onRemove()
    if (const PluginList::Info_t* p = m_list.getCurrentPlugin()) {
        doRemove(p->id);
        m_proxy.cancelInstallation();
        m_proxy.requestList();
    }
}

/** Implementation of the "add" action. */
void
Dialog::doAdd(game::proxy::WaitIndicator& ind, const String_t& fileName)
{
    // Initialize
    afl::string::Translator& tx = m_parent.translator();
    PluginManagerProxy::InstallInfo info = m_proxy.prepareInstall(ind, fileName);
    if (!info.isValid) {
        showError(info.errorMessage,
                  tx("Plugin not installed."),
                  Parser::parseXml(tx("File cannot be installed.\n"
                                      "<small>The file you have chosen cannot be installed as a plugin. "
                                      "A plugin normally comes as a <tt>*.c2p</tt> or <tt>*.c2z</tt> file.</small>")));
        return;
    }

    // Check ambiguities
    switch (info.ambiguity) {
     case util::plugin::Installer::NoPlugin:
        break;

     case util::plugin::Installer::OnePlugin: {
        Text text(tx("The file you have selected is not a plugin file."));
        text += "\n\n";
        text += Text(Format(tx("It can be converted into a plugin. "
                               "However, there is a plugin definition file (%s) next to it. "
                               "If %s is part of that, it is strongly recommended to install the *.c2p file instead."),
                            info.altTitle,
                            info.fileTitle))
            .withStyle(StyleAttribute::Small);
        text += "\n\n";
        text += tx("Do you want to proceed with the original file, or use the *.c2p file?");

        enum { Proceed, Redirect, Cancel };
        KeyString kProceed(tx("Proceed"));
        KeyString kRedirect(tx("Use *.c2p"));
        MessageBox box(text, tx("Install Plugin"), m_parent.root());
        box.addButton(Proceed, kProceed);
        box.addButton(Redirect, kRedirect);
        box.addButton(Cancel, tx("Cancel"), util::Key_Escape);
        box.addKey(Proceed, ' ');
        switch (box.run()) {
         case Proceed:
            // ok
            break;

         case Redirect:
            // use alternative
            info = m_proxy.prepareInstall(ind, info.altName);
            if (!info.isValid) {
                // oh crap, didn't work
                showError(info.errorMessage, tx("Plugin not installed."),
                          Parser::parseXml(tx("File cannot be installed.\n"
                                              "<small>There was trouble processing the alternative file.</small>")));
                return;
            }
            break;

         case Cancel:
            // abort
            return;
        }
        break;
     }

     case util::plugin::Installer::MultiplePlugins: {
        Text text(tx("The file you have selected is not a plugin file."));
        text += "\n\n";
        text += Text(Format(tx("It can be converted into a plugin. "
                               "However, there are multiple plugin definition files (*.c2p) next to it. "
                               "If %s is part of one of these, it is strongly recommended to install the *.c2p file instead."),
                            info.fileTitle))
            .withStyle(StyleAttribute::Small);
        text += "\n\n";
        text += tx("Do you want to proceed anyway?");

        enum { Proceed, Cancel };
        KeyString kProceed(tx("Proceed"));
        MessageBox box(text, tx("Install Plugin"), m_parent.root());
        box.addButton(Proceed, kProceed);
        box.addButton(Cancel, tx("Cancel"), util::Key_Escape);
        box.addKey(Proceed, ' ');
        if (box.run() != Proceed) {
            return;
        }
        break;
     }
    }

    // Check conflicts
    if (const String_t* p = info.conflicts.get()) {
        showError(*p, String_t(), tx("Unable to install plugin."));
        return;
    }

    // Confirm
    {
        String_t msg = info.isUpdate
            ? tx("Do you want to update plugin \"%s\" (%s)?)")
            : tx("Do you want to install plugin \"%s\" (%s)?)");
        Text text(Format(msg, info.pluginName, info.pluginId));
        if (!info.pluginDescription.empty()) {
            text += "\n\n";
            text += Text(info.pluginDescription).withStyle(StyleAttribute::Small);
        }
        if (!MessageBox(text, tx("Install Plugin"), m_parent.root()).doYesNoDialog(tx)) {
            return;
        }
    }

    // If this is an update, unload the existing plugin
    if (info.isUpdate) {
        m_parent.unloadPlugin(info.pluginId);
        m_restart = true;
    }

    // Install the plugin
    PluginManagerProxy::InstallResult result = m_proxy.doInstall(ind);
    if (!result.isValid) {
        showError(info.errorMessage, tx("Plugin not installed."), tx("Unable to install plugin."));
        m_proxy.requestList();
        return;
    }

    // Load it
    m_parent.loadPlugin(result.pluginId);
}

/** Implementation of the "remove" action. */
void
Dialog::doRemove(String_t id)
{
    afl::string::Translator& tx = m_parent.translator();

    // Check preconditions
    client::Downlink link(m_parent.root(), tx);
    PluginManagerProxy::RemoveResult preResult = m_proxy.prepareRemove(link, id);
    if (!preResult.isValid) {
        showError(preResult.errorMessage, String_t(), tx("Unable to uninstall plugin."));
        return;
    }

    // Ask user
    if (!MessageBox(Format(tx("Do you want to remove plugin '%s'?"), id), tx("Uninstall Plugin"), m_parent.root()) .doYesNoDialog(tx)) {
        return;
    }

    // Do it
    m_parent.unloadPlugin(id);
    m_restart = true;
    PluginManagerProxy::RemoveResult result = m_proxy.doRemove(link, id);
    if (!result.isValid) {
        showError(result.errorMessage, String_t(), tx("Plugin could not be completely uninstalled."));
        return;
    }
}

/** Show an error message.
    @param errorMessage   Error message provided by proxy (can be empty)
    @param conclusio      Additional text to add to error message
    @param defaultMessage Message to use when error message is empty */
void
Dialog::showError(const String_t& errorMessage, const String_t& conclusio, const Text& defaultMessage)
{
    Text text;
    if (errorMessage.empty()) {
        text = defaultMessage;
    } else {
        text = errorMessage;
        if (!conclusio.empty()) {
            text += "\n";
            text += conclusio;
        }
    }

    MessageBox(text, m_parent.translator()("Plugins"), m_parent.root()).doOkDialog(m_parent.translator());
}


/*
 *  PluginManager
 */

client::dialogs::PluginManager::PluginManager(ui::Root& root,
                                              util::RequestSender<game::Session> gameSender,
                                              afl::string::Translator& tx)
    : m_root(root),
      m_gameSender(gameSender),
      m_translator(tx)
{ }

void
client::dialogs::PluginManager::run()
{
    Dialog dlg(*this);
    dlg.run();
    if (dlg.isRestart()) {
        MessageBox(m_translator("You should restart PCC2 for the changes to take effect."), m_translator("Plugins"), m_root)
            .doOkDialog(m_translator);
    }
}

ui::Root&
client::dialogs::PluginManager::root()
{
    return m_root;
}

const util::RequestSender<game::Session>&
client::dialogs::PluginManager::gameSender()
{
    return m_gameSender;
}

afl::string::Translator&
client::dialogs::PluginManager::translator()
{
    return m_translator;
}
