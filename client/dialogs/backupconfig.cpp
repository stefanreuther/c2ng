/**
  *  \file client/dialogs/backupconfig.cpp
  *  \brief Backup Configuration Dialog
  */

#include "client/dialogs/backupconfig.hpp"
#include "afl/base/deleter.hpp"
#include "client/widgets/helpwidget.hpp"
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
#include "ui/window.hpp"
#include "ui/widgets/framegroup.hpp"

namespace {
    enum {
        NoBackup,
        DefaultBackup,
        CustomBackup
    };

    class Dialog {
     public:
        Dialog(ui::Root& root, afl::string::Translator& tx)
            : m_root(root), m_translator(tx),
              m_mode(NoBackup),
              m_input(1000, 20, root)
            {
                m_mode.sig_change.add(this, &Dialog::onMove);
            }

        bool run(util::RequestSender<game::Session>& gameSender)
            {
                // ex WConfigBackupEditorWindow::init
                // VBox
                //   RadioButton "disabled"
                //   RadioButton "enabled"
                //   RadioButton "custom"
                //   HBox [spacer, InputLine]
                //   StandardDialogButtons
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(m_translator("Backup Setting"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

                // RadioButtons
                ui::Widget& r1 = del.addNew(new ui::widgets::RadioButton(m_root, 'd', m_translator("disabled (no backup)"), m_mode, NoBackup));
                ui::Widget& r2 = del.addNew(new ui::widgets::RadioButton(m_root, 'e', m_translator("enabled (standard file name)"), m_mode, DefaultBackup));
                ui::Widget& r3 = del.addNew(new ui::widgets::RadioButton(m_root, 'c', m_translator("custom file name"), m_mode, CustomBackup));
                win.add(r1);
                win.add(r2);
                win.add(r3);

                // Input
                ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
                g.add(del.addNew(new ui::Spacer(m_root.provider().getFont("+")->getCellSize().scaledBy(2, 1))));
                g.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, m_input));
                win.add(g);

                // OK/Cancel buttons
                ui::EventLoop loop(m_root);
                ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
                btn.addStop(loop);
                win.add(btn);

                // Help
                ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, gameSender, "pcc2:settings:backup"));
                btn.addHelp(help);
                win.add(help);

                // Focus handling
                ui::widgets::FocusIterator& it = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Tab | ui::widgets::FocusIterator::Vertical));
                it.add(r1);
                it.add(r2);
                it.add(r3);
                it.add(m_input);
                win.add(it);

                // Quit handling
                win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));

                // Initial focus
                switch (m_mode.get()) {
                 case NoBackup:      r1.requestFocus(); break;
                 case DefaultBackup: r2.requestFocus(); break;
                 case CustomBackup:  r3.requestFocus(); break;
                }

                // Run it
                onMove();
                win.pack();
                m_root.centerWidget(win);
                m_root.add(win);
                return (loop.run() != 0);
            }

        void setValue(const String_t& value, const String_t& defaultValue)
            {
                if (value.empty()) {
                    m_mode.set(NoBackup);
                    m_input.setText(defaultValue);
                } else {
                    if (value == defaultValue) {
                        m_mode.set(DefaultBackup);
                    } else {
                        m_mode.set(CustomBackup);
                    }
                    m_input.setText(value);
                }
            }

        String_t getValue(const String_t& defaultValue)
            {
                // WConfigBackupEditorWindow::onOK() [sort-of]
                switch (m_mode.get()) {
                 case NoBackup:
                 default:
                    return String_t();

                 case DefaultBackup:
                    return defaultValue;

                 case CustomBackup:
                    return m_input.getText();
                }
            }

     private:
        void onMove()
            {
                // WConfigBackupEditorWindow::onMove()
                m_input.setState(ui::Widget::DisabledState, m_mode.get() != CustomBackup);
            }

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        afl::base::Observable<int> m_mode;
        ui::widgets::InputLine m_input;
    };
}

bool
client::dialogs::editBackupConfiguration(String_t& value,
                                         const String_t& defaultValue,
                                         ui::Root& root,
                                         util::RequestSender<game::Session> gameSender,
                                         afl::string::Translator& tx)
{
    Dialog dlg(root, tx);
    dlg.setValue(value, defaultValue);
    if (dlg.run(gameSender)) {
        value = dlg.getValue(defaultValue);
        return true;
    } else {
        return false;
    }
}
