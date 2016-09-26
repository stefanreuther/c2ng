/**
  *  \file client/screens/playerscreen.cpp
  */

#include "client/screens/playerscreen.hpp"
#include "client/si/control.hpp"
#include "ui/widgets/button.hpp"
#include "ui/root.hpp"
#include "ui/spacer.hpp"
#include "ui/window.hpp"
#include "ui/rich/documentview.hpp"
#include "util/requestreceiver.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/vbox.hpp"
#include "client/widgets/keymapwidget.hpp"
#include "afl/string/format.hpp"
#include "game/root.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"

namespace {
    class PlayerScreen : private client::si::Control {
     public:
        PlayerScreen(client::Session& session)
            : Control(session.interface(), session.root(), session.translator()),
              m_session(session),
              m_loop(session.root()),
              m_docView(gfx::Point(200, 200), 0, session.root().provider()),
              m_receiver(session.dispatcher(), *this),
              m_slave(session.gameSender(), new Trampoline(m_receiver.getSender())),
              m_outputState()
            { }

        ~PlayerScreen()
            { }

        void run(client::si::InputState& in, client::si::OutputState& out)
            {
                // Build a panel
                ui::Root& root = m_session.root();
                ui::Window panel("!Player screen", root.provider(), ui::BLUE_BLACK_WINDOW, ui::layout::VBox::instance5);
                ui::Spacer spacer;
                ui::widgets::Button btnTest("!Test", 't', root.provider(), root.colorScheme());
                ui::widgets::Button btnList("!List processes on console", 'l', root.provider(), root.colorScheme());
                ui::widgets::Button btnClose("!ESC - Close", util::Key_Escape, root.provider(), root.colorScheme());
                ui::widgets::Button btnF1("!F1 - Ships", util::Key_F1, root.provider(), root.colorScheme());
                ui::widgets::Button btnF2("!F2 - Planets", util::Key_F2, root.provider(), root.colorScheme());
                ui::widgets::Button btnF3("!F3 - Starbases", util::Key_F3, root.provider(), root.colorScheme());
                client::widgets::KeymapWidget keys(m_session.gameSender(), root.engine().dispatcher(), *this);
                keys.setKeymapName("RACESCREEN");
                panel.add(m_docView);
                panel.add(spacer);
                panel.add(btnTest);
                panel.add(btnList);
                panel.add(btnF1);
                panel.add(btnF2);
                panel.add(btnF3);
                panel.add(btnClose);
                panel.add(keys);

                keys.addButton(btnClose);
                btnList.sig_fire.add(this, &PlayerScreen::onList);
                btnTest.sig_fire.add(this, &PlayerScreen::onTest);
                keys.addButton(btnF1);
                keys.addButton(btnF2);
                keys.addButton(btnF3);
                panel.setExtent(root.getExtent());
                panel.setState(ui::Widget::ModalState, true);
                root.add(panel);

                continueProcessWait(in.getProcess());
                m_loop.run();
                out = m_outputState;
            }

        void onTest()
            { }

        void onList()
            {
                class Job : public util::Request<game::Session> {
                 public:
                    void handle(game::Session& s)
                        {
                            const interpreter::ProcessList::Vector_t& list = s.world().processList().getProcessList();
                            afl::sys::LogListener& log = s.log();
                            log.write(log.Info, "main.ps", "  PID  PGID      STATE  NAME");
                            for (size_t i = 0, n = list.size(); i < n; ++i) {
                                const char* state = "?";
                                interpreter::Process& p = *list[i];
                                switch (p.getState()) {
                                 case interpreter::Process::Suspended:  state = "Suspended"; break;
                                 case interpreter::Process::Frozen:     state = "Frozen"; break;
                                 case interpreter::Process::Runnable:   state = "Runnable"; break;
                                 case interpreter::Process::Running:    state = "Running"; break;
                                 case interpreter::Process::Waiting:    state = "Waiting"; break;
                                 case interpreter::Process::Ended:      state = "Ended"; break;
                                 case interpreter::Process::Terminated: state = "Terminated"; break;
                                 case interpreter::Process::Failed:     state = "Failed"; break;
                                }

                                // Write a brief message:
                                //   run 17@33 Running 'UI.Foo'
                                log.write(log.Info, "main.ps", afl::string::Format("%5d %5d %10s '%s'")
                                          << p.getProcessId()
                                          << p.getProcessGroupId()
                                          << state
                                          << p.getName());
                            }
                        }
                };
                m_session.gameSender().postNewRequest(new Job());
            }

        void setInfo(String_t s)
            {
                ui::rich::Document& doc = m_docView.getDocument();
                doc.clear();
                doc.add(s);
                doc.finish();
                m_docView.handleDocumentUpdate();
            }

        virtual void handleStateChange(client::si::UserSide& us, client::si::RequestLink2 link, client::si::OutputState::Target target)
            {
                using client::si::OutputState;
                switch (target) {
                 case OutputState::NoChange:
                 case OutputState::PlayerScreen:
                    us.continueProcess(link);
                    break;

                 case OutputState::ExitProgram:
                 case OutputState::ExitGame:
                 case OutputState::ShipScreen:
                 case OutputState::PlanetScreen:
                 case OutputState::BaseScreen:
                    us.detachProcess(link);
                    m_outputState.set(link, target);
                    m_loop.stop(0);
                    break;
                }
            }
        virtual void handlePopupConsole(client::si::UserSide& ui, client::si::RequestLink2 link)
            { defaultHandlePopupConsole(ui, link); }
        virtual void handleEndDialog(client::si::UserSide& ui, client::si::RequestLink2 link, int /*code*/)
            {
                ui.continueProcess(link);
            }

        virtual client::si::ContextProvider* createContextProvider()
            { return 0; }

     private:
        client::Session& m_session;
        ui::EventLoop m_loop;
        ui::rich::DocumentView m_docView;
        util::RequestReceiver<PlayerScreen> m_receiver;

        class Trampoline : public util::SlaveObject<game::Session> {
         public:
            Trampoline(util::RequestSender<PlayerScreen> sender)
                : m_sender(sender),
                  conn_root(),
                  m_pSession(0)
                { }
            void init(game::Session& session)
                {
                    if (game::Root* root = session.getRoot().get()) {
                        conn_root = root->playerList().sig_change.add(this, &Trampoline::onChange);
                    }
                    m_pSession = &session; // FIXME: this is a hack
                    onChange();
                }
            void done(game::Session& /*session*/)
                {
                    conn_root.disconnect();
                }
            void onChange()
                {
                    String_t info;
                    if (m_pSession != 0) {
                        game::Root* root = m_pSession->getRoot().get();
                        game::Game* game = m_pSession->getGame().get();
                        if (root != 0 && game != 0) {
                            if (game::Player* p = root->playerList().get(game->getViewpointPlayer())) {
                                info = p->getName(game::Player::LongName);
                                if (!p->getName(game::Player::UserName).empty()) {
                                    info += "\n";
                                    info += p->getName(game::Player::UserName);
                                }
                            } else {
                                info = afl::string::Format("!Player %d", game->getViewpointPlayer());
                            }
                            info += "\n";
                            info += afl::string::Format("%d message%!1{s%}", game->currentTurn().inbox().getNumMessages());
                            info += "\n";
                        }
                    }
                    m_sender.postNewRequest(new UpdateTask(info));
                }
         private:
            class UpdateTask : public util::Request<PlayerScreen> {
             public:
                UpdateTask(String_t s)
                    : m_string(s)
                    { }
                void handle(PlayerScreen& ps)
                    { ps.setInfo(m_string); }
             private:
                String_t m_string;
            };
            util::RequestSender<PlayerScreen> m_sender;
            afl::base::SignalConnection conn_root;
            game::Session* m_pSession;
        };
        util::SlaveRequestSender<game::Session,Trampoline> m_slave;
        client::si::OutputState m_outputState;
    };
}

void
client::screens::doPlayerScreen(Session& session, client::si::InputState& in, client::si::OutputState& out)
{
    PlayerScreen(session).run(in, out);
}
