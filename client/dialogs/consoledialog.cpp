/**
  *  \file client/dialogs/consoledialog.cpp
  */

#include "client/dialogs/consoledialog.hpp"
#include "afl/base/optional.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/si/contextprovider.hpp"
#include "client/si/contextreceiver.hpp"
#include "client/si/control.hpp"
#include "client/widgets/consoleview.hpp"
#include "game/interface/completionlist.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/messagecollector.hpp"
#include "util/messagenotifier.hpp"
#include "util/stringparser.hpp"
#include "util/translation.hpp"

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
              m_user(user),
              m_root(root),
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
                            p.parseCharacter(',');
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
                 case util::Key_Tab:
                    doCompletion();
                    return true;
                 default:
                    return defaultHandleKey(key, prefix);
                }
            }

        void insertCompletion(const String_t& stem, const String_t& completion)
            {
                if (completion.size() > stem.size()) {
                    m_input.setFlag(ui::widgets::InputLine::TypeErase, false);
                    m_input.insertText(completion.substr(stem.size()));
                }
            }

        void doCompletion()
            {
                client::Downlink link(m_root);
                game::interface::CompletionList result;

                class Query : public util::Request<game::Session> {
                 public:
                    Query(game::interface::CompletionList& result, const String_t& text, std::auto_ptr<client::si::ContextProvider> ctxp)
                        : m_result(result),
                          m_text(text),
                          m_contextProvider(ctxp)
                        { }
                    virtual void handle(game::Session& session)
                        {
                            class Collector : public client::si::ContextReceiver {
                             public:
                                virtual void addNewContext(interpreter::Context* p)
                                    { m_contexts.pushBackNew(p); }

                                afl::container::PtrVector<interpreter::Context>& get()
                                    { return m_contexts; }
                             private:
                                afl::container::PtrVector<interpreter::Context> m_contexts;
                            };
                            Collector c;
                            if (m_contextProvider.get() != 0) {
                                m_contextProvider->createContext(session, c);
                            }
                            buildCompletionList(m_result, m_text, session, false, c.get());
                        }
                 private:
                    game::interface::CompletionList& m_result;
                    String_t m_text;
                    std::auto_ptr<client::si::ContextProvider> m_contextProvider;
                };
                Query q(result, afl::charset::Utf8().substr(m_input.getText(), 0, m_input.getCursorIndex()), std::auto_ptr<client::si::ContextProvider>(m_user.createContextProvider()));
                link.call(m_user.gameSender(), q);

                String_t stem = result.getStem();
                String_t immediate = result.getImmediateCompletion();
                if (immediate.size() > stem.size()) {
                    insertCompletion(stem, immediate);
                } else if (!result.isEmpty()) {
                    ui::widgets::StringListbox list(m_root.provider(), m_root.colorScheme());
                    int32_t i = 0;
                    for (game::interface::CompletionList::Iterator_t it = result.begin(), e = result.end(); it != e; ++it) {
                        list.addItem(i, *it);
                        ++i;
                    }
                    list.sortItemsAlphabetically();
                    if (ui::widgets::doStandardDialog(_("Completions"), list, true, m_root)
                        && list.getCurrentKey(i))
                    {
                        game::interface::CompletionList::Iterator_t it = result.begin(), e = result.end();
                        while (it != e && i != 0) {
                            ++it, --i;
                        }
                        if (it != e) {
                            insertCompletion(stem, *it);
                        }
                    } else {
                    }
                } else {
                    // no completions available
                }

                // No matter what happened, should still clear TypeErase to avoid new input overwriting old one.
                m_input.setFlag(ui::widgets::InputLine::TypeErase, false);
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
        client::si::UserSide& m_user;
        ui::Root& m_root;
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
