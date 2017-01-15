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
#include "ui/invisiblewidget.hpp"
#include "util/messagecollector.hpp"
#include "util/messagenotifier.hpp"
#include "client/widgets/consoleview.hpp"
#include "util/stringparser.hpp"
#include "afl/base/optional.hpp"

namespace {
    const int NLINES = 15;

    class ConsoleController : public ui::InvisibleWidget {
     public:
        ConsoleController(client::widgets::ConsoleView& view,
                          ui::widgets::InputLine& input,
                          client::si::UserSide& user,
                          ui::Root& root)
            : m_scrollback(0),
              m_view(view),
              m_input(input),
              m_collector(user.console()),
              m_notifier(root.engine().dispatcher()),
              m_format()
            {
                m_format.setConfiguration("*@-Debug=dim:"
                                          "script.error=right,red:"
                                          "script.trace=right:"
                                          "*@Error=red:"
                                          "script.input=bold:"
                                          "script.result=bold,right:"
                                          "script.empty=dim,right");
                m_recall.setConfiguration("script.input=input:"
                                          "script.result=result");
                user.mainLog().addListener(m_notifier);
                m_notifier.sig_change.add(this, &ConsoleController::onUpdate);
                onUpdate();
            }

        void onUpdate()
            {
                util::MessageCollector::MessageNumber_t nr = m_collector.getNewestPosition();

                // Skip scrollback
                int n = m_scrollback;
                while (n > 0 && m_collector.readOlderMessage(nr, 0, nr)) {
                    --n;
                }

                // Render
                m_view.clear();
                n = NLINES;
                afl::sys::LogListener::Message msg;
                while (n > 0 && m_collector.readOlderMessage(nr, &msg, nr)) {
                    --n;

                    int align = 0;
                    int bold = 0;
                    util::SkinColor::Color color = util::SkinColor::Static;
                    String_t result;
                    if (m_format.match(msg, result)) {
                        util::StringParser p(result);
                        String_t s;
                        while (!p.parseEnd()) {
                            p.parseDelim(",", s);
                            if (s == "left") {
                                align = 0;
                            } else if (s == "right") {
                                align = 2;
                            } else if (s == "center") {
                                align = 1;
                            } else if (s == "bold") {
                                ++bold;
                            } else if (s == "static") {
                                color = util::SkinColor::Static;
                            } else if (s == "green") {
                                color = util::SkinColor::Green;
                            } else if (s == "yellow") {
                                color = util::SkinColor::Yellow;
                            } else if (s == "red") {
                                color = util::SkinColor::Red;
                            } else if (s == "white") {
                                color = util::SkinColor::White;
                            } else if (s == "blue") {
                                color = util::SkinColor::Blue;
                            } else if (s == "dim") {
                                color = util::SkinColor::Faded;
                            } else {
                                // else what?
                            }
                                
                            if (p.parseEnd()) {
                                break;
                            }
                            p.parseChar(',');
                        }
                    }
                    
                    m_view.addLine(n, msg.m_message, align, bold, color);
                }
                m_view.setScrollbackIndicator(m_scrollback);
            }

        virtual bool handleKey(util::Key_t key, int prefix)
            {
                switch (key) {
                 case util::KeyMod_Shift + util::Key_PgUp:
                    requestActive();
                    m_scrollback += 5;
                    onUpdate();
                    return true;
                 case util::KeyMod_Shift + util::Key_Up:
                    requestActive();
                    m_scrollback += 1;
                    onUpdate();
                    return true;
                 case util::KeyMod_Shift + util::Key_PgDn:
                    requestActive();
                    m_scrollback -= std::min(m_scrollback, 5);
                    onUpdate();
                    return true;
                 case util::KeyMod_Shift + util::Key_Down:
                    requestActive();
                    m_scrollback -= std::min(m_scrollback, 1);
                    onUpdate();
                    return true;
                 case util::KeyMod_Shift + util::Key_End:
                    requestActive();
                    m_scrollback = 0;
                    onUpdate();
                    return true;
                 case util::Key_Up:
                    requestActive();
                    doRecall(-1);
                    return true;
                 case util::Key_Down:
                    requestActive();
                    doRecall(+1);
                    return true;
                 default:
                    return defaultHandleKey(key, prefix);
                }
            }

        void doRecall(int direction)
            {
                // ex WConsoleDialog::doRecall

                // Find current position.
                // If at end, initialize.
                util::MessageCollector::MessageNumber_t pos;
                if (!m_recallPosition.get(pos)) {
                    pos = m_collector.getNewestPosition();
                    m_recallLastInput = m_input.getText();
                }

                while (1) {
                    afl::sys::LogListener::Message msg;
                    bool ok = (direction < 0
                               ? m_collector.readOlderMessage(pos, &msg, pos)
                               : m_collector.readNewerMessage(pos, &msg, pos));
                    if (!ok) {
                        // End reached
                        if (direction < 0) {
                            // Cannot go further
                            break;
                        } else {
                            // End reached
                            m_recallPosition = afl::base::Nothing;
                            m_input.setText(m_recallLastInput);
                            break;
                        }
                    } else {
                        // Found a line. Recallable?
                        String_t mode;
                        if (msg.m_message != m_input.getText() && m_recall.match(msg, mode) && (mode == "input" || mode == "result")) {
                            m_recallPosition = pos;
                            m_input.setText(msg.m_message);
                            break;
                        }
                    }
                }
            }

        void resetRecall()
            {
                m_recallPosition = afl::base::Nothing;
                m_scrollback = 0;
                onUpdate();
            }

     private:
        int m_scrollback;
        client::widgets::ConsoleView& m_view;
        ui::widgets::InputLine& m_input;
        util::MessageCollector& m_collector;
        util::MessageNotifier m_notifier;
        util::MessageMatcher m_format;
        util::MessageMatcher m_recall;
        afl::base::Optional<util::MessageCollector::MessageNumber_t> m_recallPosition;
        String_t m_recallLastInput;
    };


    class ConsoleDialog : private client::si::Control {
     public:
        ConsoleDialog(client::si::UserSide& iface, client::si::Control& parentControl, client::si::OutputState& outputState)
            : Control(iface, parentControl.root(), parentControl.translator()),
              m_parentControl(parentControl),
              m_loop(parentControl.root()),
              m_window(translator().translateString("Console"), parentControl.root().provider(), parentControl.root().colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5),
              m_input(1000, 30, parentControl.root()),
              m_group(ui::layout::HBox::instance5),
              m_spacer(),
              m_okButton(translator().translateString("OK"), util::Key_Return, parentControl.root()),
              m_cancelButton(translator().translateString("Cancel"), util::Key_Escape, parentControl.root()),
              m_consoleView(parentControl.root().provider(), gfx::Point(35, 15)),
              m_consoleController(m_consoleView, m_input, iface, parentControl.root()),
              m_outputState(outputState)
            {
                m_input.setFont(gfx::FontRequest().addSize(1));
                m_okButton.sig_fire.add(this, &ConsoleDialog::onOK);
                m_cancelButton.sig_fire.addNewClosure(m_loop.makeStop(0));
                m_window.add(m_consoleView);
                m_window.add(m_consoleController);
                m_window.add(m_input);
                m_window.add(m_group);
                m_group.add(m_spacer);
                m_group.add(m_okButton);
                m_group.add(m_cancelButton);
                m_input.requestFocus();

                // Greeting
                // FIXME: this is in WConsoleDialog::processPreIdle and thus should not appear if we are blocked by a script that just did UI.PopupConsole
                String_t greeting = translator().translateString("Enter command or expression:");
                afl::sys::LogListener::Message msg;
                util::MessageCollector::MessageNumber_t nr = iface.console().getNewestPosition();
                if (!iface.console().readOlderMessage(nr, &msg, nr) || msg.m_message != greeting) {
                    iface.mainLog().write(afl::sys::LogListener::Info, "console", greeting);
                }
            }

        void onOK()
            {
                // Reset
                m_consoleController.resetRecall();

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
        client::widgets::ConsoleView m_consoleView;
        ConsoleController m_consoleController;
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
