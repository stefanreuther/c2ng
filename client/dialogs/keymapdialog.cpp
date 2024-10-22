/**
  *  \file client/dialogs/keymapdialog.cpp
  */

#include "client/dialogs/keymapdialog.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/helpdialog.hpp"
#include "client/downlink.hpp"
#include "game/proxy/keymapproxy.hpp"
#include "ui/group.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/statictext.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/window.hpp"
#include "util/keymapinformation.hpp"

using game::proxy::KeymapProxy;
using afl::string::Format;
using util::rich::Text;
using util::KeymapInformation;

namespace {
    class KeymapDialog : private KeymapProxy::Listener {
     public:
        KeymapDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender);
        ~KeymapDialog();

        void setKeymapName(String_t name);
        void run(ui::Root& root);
        bool handleKey(util::Key_t key);
        void onHelp();

     private:
        void requestUpdate();

        virtual void updateKeyList(util::KeySet_t& keys);

        void renderInformation();

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::Session> m_gameSender;

        KeymapProxy m_proxy;
        client::Downlink m_link;

        String_t m_primaryKeymapName;
        String_t m_alternateKeymapName;

        String_t m_currentKeymapName;
        util::KeySet_t m_keys;

        KeymapInformation m_info;
        KeymapInformation::Index_t m_infoIndex;
        bool m_isESC;

        ui::rich::StaticText m_treeText;
        ui::rich::StaticText m_responseText;

        ui::EventLoop m_loop;
    };

    class KeyWidget : public ui::InvisibleWidget {
     public:
        KeyWidget(KeymapDialog& dlg)
            : m_dialog(dlg)
            { }

        virtual bool handleKey(util::Key_t key, int /*prefix*/)
            { return m_dialog.handleKey(key); }
     private:
        KeymapDialog& m_dialog;
    };
}

KeymapDialog::KeymapDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender)
    : Listener(),
      m_root(root),
      m_translator(tx),
      m_gameSender(gameSender),
      m_proxy(gameSender, root.engine().dispatcher()),
      m_link(root, tx),
      m_primaryKeymapName(),
      m_alternateKeymapName(),
      m_currentKeymapName(),
      m_keys(),
      m_info(),
      m_infoIndex(KeymapInformation::nil),
      m_isESC(false),
      m_treeText(Text(), 0, root.provider()),
      m_responseText(Text(), 0, root.provider()),
      m_loop(root)
{
    m_proxy.setListener(*this);
}

KeymapDialog::~KeymapDialog()
{ }

void
KeymapDialog::setKeymapName(String_t name)
{
    m_primaryKeymapName = name;
    m_alternateKeymapName.clear();
    requestUpdate();
}

void
KeymapDialog::run(ui::Root& root)
{
    // Clear rich-text widgets to not mess up layout
    m_treeText.setText(Text());
    m_responseText.setText(Text());

    // Build dialog
    // ex WKeymapDebugger::init
    // VBox
    //   Grid
    //     keymaptree
    //     response
    //   HBox
    //     UISpacer
    //     "Close"

    gfx::Point cellSize = root.provider().getFont(gfx::FontRequest())->getCellSize();
    gfx::Point textSize = cellSize.scaledBy(20, 12);

    afl::base::Deleter h;
    ui::Window& win(h.addNew(new ui::Window(m_translator("Keymap Debugger"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5)));
    win.add(h.addNew(new KeyWidget(*this)));

    // Text
    ui::layout::Grid& g1Layout = h.addNew(new ui::layout::Grid(2));
    ui::Group& g1 = h.addNew(new ui::Group(g1Layout));
    g1Layout.setForcedCellSize(textSize.getX(), textSize.getY());
    g1.add(m_treeText);
    g1.add(m_responseText);
    win.add(g1);

    ui::widgets::Button& btnClose = h.addNew(new ui::widgets::Button(m_translator("Close"), 0, root));
    ui::Group& g2 = h.addNew(new ui::Group(ui::layout::HBox::instance5));
    g2.add(h.addNew(new ui::Spacer()));
    g2.add(btnClose);

    ui::widgets::Button& btnHelp = h.addNew(new ui::widgets::Button(m_translator("Help"), 0, root));
    btnHelp.sig_fire.add(this, &KeymapDialog::onHelp);
    g2.add(btnHelp);
    win.add(g2);
    win.pack();
    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));

    m_responseText.setText(m_translator("Press the key for which you want information."));
    renderInformation();

    // Do it
    root.centerWidget(win);
    root.add(win);
    m_loop.run();
}

bool
KeymapDialog::handleKey(util::Key_t key)
{
    // ex WKeymapDebugger::handleEvent
    // Shift-Escape always stops
    if (key == util::KeyMod_Shift + util::Key_Escape || (m_isESC && key == util::Key_Escape)) {
        m_loop.stop(0);
        return true;
    }

    // Only non-modifier keys
    if (util::classifyKey(key) == util::ModifierKey) {
        return false;
    }

    // Attempt to look up other key
    KeymapProxy::Info info;
    m_proxy.getKey(m_link, key, info);

    Text message = Text(Format(m_translator("Key %s:\n"), util::formatKey(key))).withStyle(util::rich::StyleAttribute::Bold);
    bool used;
    switch (info.result) {
     case KeymapProxy::Unassigned:
        // Not bound at all
        message += m_translator("This key is not bound.\n");
        break;

     case KeymapProxy::Cancelled:
        // Key is bound to 0 in a keymap
        message += Format(m_translator("This key is unbound by %s.\n"), info.keymapName);
        break;

     case KeymapProxy::Internal:
        // Internal
        message += Format(m_translator("Bound in keymap %s.\n"), info.keymapName);
        message += m_translator("This key is handled internally.\n");
        break;

     case KeymapProxy::Normal:
        // Normal binding
        message += Format(m_translator("Bound in keymap %s.\n"), info.keymapName);
        message += Format(m_translator("Command:\n  %s\n"), info.command);
        break;
    }

    if (info.result == KeymapProxy::Unassigned) {
        used = false;
        m_infoIndex = KeymapInformation::nil;
    } else {
        used = true;
        m_infoIndex = m_info.find(info.keymapName);
    }

    if (!info.origin.empty()) {
        message += Format(m_translator("Command provided by %s\n"), info.origin);
    }

    // Process result
    m_isESC = false;
    if (key == util::Key_Escape) {
        m_isESC = true;
        if (used) {
            // ESC handled by keymap; give users advice how to proceed
            message += m_translator("Press Shift-ESC to close this window.\n");
        } else if (!m_alternateKeymapName.empty()) {
            // Pretend key was used to cancel secondary keymap
            used = true;
        } else {
            m_loop.stop(0);
        }
    }

    if (key == util::Key_Quit) {
        message += m_translator("Press Shift-ESC to close this window.\n");
    }

    // If key was used, switch to alternate keymap.
    // (This means if there is no alternate keymap, we switch back to primary.)
    if (used) {
        m_alternateKeymapName = info.alternateKeymapName;
        requestUpdate();
    }

    m_responseText.setText(message);
    m_responseText.requestRedraw();
    renderInformation();
    return true;
}

void
KeymapDialog::onHelp()
{
    // ex WKeymapDebugger::onHelp
    client::dialogs::doHelpDialog(m_root, m_translator, m_gameSender, "pcc2:keymap");
}

void
KeymapDialog::requestUpdate()
{
    String_t newName = m_alternateKeymapName.empty() ? m_primaryKeymapName : m_alternateKeymapName;
    if (newName != m_currentKeymapName) {
        m_currentKeymapName = newName;
        m_keys.clear();
        m_proxy.setKeymapName(newName);

        m_info.clear();
        m_infoIndex = KeymapInformation::nil;
        m_proxy.getDescription(m_link, m_info);

        renderInformation();
    }
}

void
KeymapDialog::updateKeyList(util::KeySet_t& keys)
{
    m_keys.swap(keys);
}

void
KeymapDialog::renderInformation()
{
    // ex formatKeymapTree (part)
    Text t;
    for (size_t i = 0, n = m_info.size(); i < n; ++i) {
        size_t level;
        String_t name;
        if (m_info.get(i, level, name)) {
            t += String_t(level*2, ' ');
            if (i == m_infoIndex) {
                t += Text(name).withStyle(util::rich::StyleAttribute::Bold);
            } else {
                t += name;
            }
            t += "\n";
        }
    }
    m_treeText.setText(t);
    m_treeText.requestRedraw();
}


void
client::dialogs::doKeymapDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, String_t keymapName)
{
    KeymapDialog dlg(root, tx, gameSender);
    dlg.setKeymapName(keymapName);
    dlg.run(root);
}
