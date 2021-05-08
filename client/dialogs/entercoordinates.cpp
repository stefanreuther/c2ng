/**
  *  \file client/dialogs/entercoordinates.cpp
  *  \brief Coordinate input dialog
  */

#include "client/dialogs/entercoordinates.hpp"
#include "client/widgets/helpwidget.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

namespace {
    class EnterCoordinatesDialog {
     public:
        EnterCoordinatesDialog(const game::map::Configuration& config,
                               ui::Root& root,
                               util::RequestSender<game::Session> gameSender,
                               afl::string::Translator& tx)
            : m_config(config),
              m_root(root),
              m_gameSender(gameSender),
              m_translator(tx),
              m_input(100, 8, root),
              m_loop(root),
              m_result()
            { }

        bool run()
            {
                // ex WGoToXYDialog::init
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(m_translator("Go To X/Y"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

                win.add(del.addNew(new ui::widgets::StaticText(m_translator("Enter X,Y coordinates or sector number:"), util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, m_input));

                ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
                ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:gotoxy"));
                btn.addHelp(help);
                btn.ok().sig_fire.add(this, &EnterCoordinatesDialog::onOK);
                btn.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
                win.add(btn);
                win.add(help);
                win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
                win.pack();

                m_root.centerWidget(win);
                m_root.add(win);
                return m_loop.run() != 0;
            }

        void onOK()
            {
                // ex WGoToXYDialog::handleEvent
                if (m_input.getText().empty()) {
                    // ignore
                } else if (m_result.parseCoordinates(m_input.getText()) || m_config.parseSectorNumber(m_input.getText(), m_result)) {
                    // success
                    m_loop.stop(1);
                } else {
                    // failure
                    ui::dialogs::MessageBox(m_translator("Invalid input."), m_translator("Go To X/Y"), m_root)
                        .doOkDialog(m_translator);
                }
            }

        game::map::Point getResult() const
            { return m_result; }

     private:
        const game::map::Configuration& m_config;
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;
        ui::widgets::InputLine m_input;
        ui::EventLoop m_loop;
        game::map::Point m_result;
    };
}

bool
client::dialogs::doEnterCoordinatesDialog(game::map::Point& result,
                                          const game::map::Configuration& config,
                                          ui::Root& root,
                                          util::RequestSender<game::Session> gameSender,
                                          afl::string::Translator& tx)
{
    // ex doGoToXYDialog
    EnterCoordinatesDialog dlg(config, root, gameSender, tx);
    bool ok = dlg.run();
    if (ok) {
        result = dlg.getResult();
    }
    return ok;
}
