/**
  *  \file client/dialogs/consoledialog.cpp
  */

#include "client/dialogs/consoledialog.hpp"
#include "ui/eventloop.hpp"
#include "ui/window.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/button.hpp"
#include "ui/spacer.hpp"
#include "client/si/control.hpp"
#include "util/translation.hpp"
#include "afl/string/format.hpp"

namespace {
    class ConsoleDialog : private client::si::Control {
     public:
        ConsoleDialog(client::si::UserSide& iface, client::si::Control& parentControl, client::si::OutputState& outputState)
            : Control(iface, parentControl.root(), parentControl.translator()),
              m_parentControl(parentControl),
              m_loop(parentControl.root()),
              m_window(translator().translateString("Console"), parentControl.root().provider(), ui::BLUE_WINDOW, ui::layout::VBox::instance5),
              m_input(1000, 40, parentControl.root()),
              m_group(ui::layout::HBox::instance5),
              m_spacer(),
              m_okButton(translator().translateString("OK"), util::Key_Return, parentControl.root().provider(), parentControl.root().colorScheme()),
              m_cancelButton(translator().translateString("Cancel"), util::Key_Escape, parentControl.root().provider(), parentControl.root().colorScheme()),
              m_outputState(outputState)
            {
                m_okButton.sig_fire.add(this, &ConsoleDialog::onOK);
                m_cancelButton.sig_fire.addNewClosure(m_loop.makeStop(0));
                m_window.add(m_input);
                m_window.add(m_group);
                m_group.add(m_spacer);
                m_group.add(m_okButton);
                m_group.add(m_cancelButton);
            }

        void onOK()
            {
                // FIXME: logging etc.
                String_t command = afl::string::strTrim(m_input.getText());
                if (!command.empty()) {
                    executeCommandWait(command, true, afl::string::Format(translator().translateString("Console: %s").c_str(), command));
                }
                m_input.setText(String_t());
            }

        bool run(client::si::InputState& inputState)
            {
                m_window.pack();
                root().centerWidget(m_window);
                root().add(m_window);
                continueProcessWait(inputState.getProcess());
                return m_loop.run() != 0;
            }


        // Control virtuals:
        virtual void handleStateChange(client::si::UserSide& us, client::si::RequestLink2 link, client::si::OutputState::Target target)
            {
                switch (target) {
                 case client::si::OutputState::NoChange:
                    // No change
                    us.continueProcess(link);
                    break;
                 case client::si::OutputState::ExitProgram:
                 case client::si::OutputState::ExitGame:
                 case client::si::OutputState::PlayerScreen:
                 case client::si::OutputState::ShipScreen:
                 case client::si::OutputState::PlanetScreen:
                 case client::si::OutputState::BaseScreen:
                    // Dispatch to parent
                    us.detachProcess(link);
                    m_outputState.set(link, target);
                    m_loop.stop(1);
                    break;
                }
            }
        virtual void handlePopupConsole(client::si::UserSide& us, client::si::RequestLink2 link)
            { us.continueProcess(link); }
        virtual void handleEndDialog(client::si::UserSide& ui, client::si::RequestLink2 link, int /*code*/)
            {
                // FIXME: keep this?
                ui.continueProcess(link);
            }

        virtual client::si::ContextProvider* createContextProvider()
            {
                return m_parentControl.createContextProvider();
            }

     private:
        client::si::Control& m_parentControl;
        ui::EventLoop m_loop;
        ui::Window m_window;
        ui::widgets::InputLine m_input;
        ui::Group m_group;
        ui::Spacer m_spacer;
        ui::widgets::Button m_okButton;
        ui::widgets::Button m_cancelButton;
        client::si::OutputState& m_outputState;
    };
}

bool
client::dialogs::doConsoleDialog(client::si::UserSide& iface,
                                 client::si::Control& parentControl,
                                 client::si::InputState& inputState,
                                 client::si::OutputState& outputState)
{
    ConsoleDialog dialog(iface, parentControl, outputState);
    return dialog.run(inputState);
}
