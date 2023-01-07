/**
  *  \file client/dialogs/newaccount.cpp
  *  \brief Account Creation dialog
  */

#include "client/dialogs/newaccount.hpp"
#include "afl/base/deleter.hpp"
#include "afl/base/observable.hpp"
#include "client/downlink.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/radiobutton.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

using ui::dialogs::MessageBox;
using ui::widgets::FocusIterator;
using ui::widgets::InputLine;
using ui::widgets::RadioButton;
using ui::widgets::StaticText;
using util::SkinColor;

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

        bool run(ui::Widget* pHelp)
            {
                afl::string::Translator& tx = m_translator;
                afl::base::Deleter h;
                ui::Window win(tx("Add Account"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                win.add(h.addNew(new StaticText(tx("User name:"), SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                win.add(m_userInput);
                win.add(h.addNew(new StaticText(tx("Type:"), SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                win.add(m_typePlanetsCentral);
                win.add(m_typeNu);
                win.add(h.addNew(new StaticText(tx("Address (empty for default):"), SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                win.add(m_hostInput);

                FocusIterator& it = h.addNew(new FocusIterator(FocusIterator::Vertical | FocusIterator::Tab));
                it.add(m_userInput);
                it.add(m_typePlanetsCentral);
                it.add(m_typeNu);
                it.add(m_hostInput);
                win.add(it);

                ui::widgets::StandardDialogButtons& btn = h.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
                btn.ok().sig_fire.add(this, &NewAccountDialog::onOK);
                btn.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
                if (pHelp != 0) {
                    btn.addHelp(*pHelp);
                    win.add(*pHelp);
                }

                win.add(btn);
                win.add(h.addNew(new ui::widgets::Quit(m_root, m_loop)));

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

        void submit(game::proxy::BrowserProxy& proxy)
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
        InputLine m_userInput;
        RadioButton m_typePlanetsCentral;
        RadioButton m_typeNu;
        InputLine m_hostInput;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        ui::EventLoop m_loop;
    };
}

bool
client::dialogs::doNewAccountDialog(game::proxy::BrowserProxy& proxy, ui::Widget* pHelp, ui::Root& root, afl::string::Translator& tx)
{
    NewAccountDialog dlg(root, tx);
    if (dlg.run(pHelp)) {
        dlg.submit(proxy);
        return true;
    } else {
        return false;
    }
}
