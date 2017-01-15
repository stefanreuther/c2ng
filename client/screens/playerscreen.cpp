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
#include "ui/layout/hbox.hpp"
#include "ui/group.hpp"
#include "ui/widgets/imagebutton.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/layout/flow.hpp"

namespace {

    ui::widgets::AbstractButton& createImageButton(afl::base::Deleter& del, ui::Root& root, ui::LayoutableGroup& group, String_t text, util::Key_t key, String_t image)
    {
        // Create container group
        ui::widgets::FrameGroup& frame = del.addNew(new ui::widgets::FrameGroup(ui::layout::HBox::instance0, root.colorScheme(), ui::widgets::FrameGroup::LoweredFrame));

        // Create button
        ui::widgets::ImageButton& result = del.addNew(new ui::widgets::ImageButton(image, key, root, gfx::Point(110, 110)));
        result.setText(text);

        // Connect and return
        frame.add(result);
        group.add(frame);
        return result;
    }

    ui::widgets::AbstractButton& createActionButton(afl::base::Deleter& del, ui::Root& root, ui::LayoutableGroup& group, String_t text, util::Key_t key)
    {
        ui::widgets::Button& btn = del.addNew(new ui::widgets::Button(text, key, root));
        group.add(btn);
        return btn;
    }

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

        void run(client::si::InputState& in, client::si::OutputState& out, bool first)
            {
                // Player screen
                //   HBox
                //     VBox
                //       FrameGroup + ImageButton
                //       Spacer
                //     VBox
                //       DocView
                //       Spacer
                //       Buttons...
                afl::string::Translator& tx = m_session.translator();
                afl::base::Deleter del;
                ui::Root& root = m_session.root();

                // FIXME: this should be a container different widget...
                ui::LayoutableGroup& panel = del.addNew(new ui::Window("!Player screen", root.provider(), root.colorScheme(), ui::BLUE_BLACK_WINDOW, ui::layout::HBox::instance5));

                // Keymap handler
                client::widgets::KeymapWidget& keys = del.addNew(new client::widgets::KeymapWidget(m_session.gameSender(), root.engine().dispatcher(), *this));

                // Left group containing list of image buttons
                ui::LayoutableGroup& leftGroup = del.addNew(new ui::Group(ui::layout::VBox::instance5));
                keys.addButton(createImageButton(del, root, leftGroup, tx.translateString("F1 - Starships"), util::Key_F1, "menu.ship"));
                keys.addButton(createImageButton(del, root, leftGroup, tx.translateString("F2 - Planets"),   util::Key_F2, "menu.planet"));
                keys.addButton(createImageButton(del, root, leftGroup, tx.translateString("F3 - Starbases"), util::Key_F3, "menu.base"));
                keys.addButton(createImageButton(del, root, leftGroup, tx.translateString("F4 - Starchart"), util::Key_F4, "menu.chart"));
                leftGroup.add(del.addNew(new ui::Spacer()));
                panel.add(leftGroup);

                // Right group
                ui::LayoutableGroup& rightGroup = del.addNew(new ui::Group(ui::layout::VBox::instance5));
                rightGroup.add(m_docView);
                rightGroup.add(del.addNew(new ui::Spacer()));

                // Buttons
                ui::LayoutableGroup& btnGroup = del.addNew(new ui::Group(del.addNew(new ui::layout::Flow(5, true))));

                // PCC2 buttons:
                //   A - Alliances
                //   Ctrl-O - Options
                //   F7 - Search
                //   G - Global
                //   T - Teams
                //   S - Scores
                //   B - Battle Simulator
                //   I - Imperial Stats
                //   W - Write Message
                //   M - Messages
                //   V - Combat Recorder
                //   ESC - Exit
                //   H - Help
                keys.addButton(createActionButton(del, root, btnGroup, tx.translateString("ESC - Exit"), util::Key_Escape));
                rightGroup.add(btnGroup);
                panel.add(rightGroup);

                // Finish and display it
                keys.setKeymapName("RACESCREEN");
                panel.add(keys);
                panel.setExtent(root.getExtent());
                panel.setState(ui::Widget::ModalState, true);
                root.add(panel);

                // Execute a possible inbound process. This will return when the inbound process finished.
                // If the inbound process requests a context change, this will already stop the m_loop.
                continueProcessWait(in.getProcess());

                // Execute initialisation hooks the first time we're on the player screen.
                // If the inbound process already requested a context change, bad things would happen if we start another process here.
                // Therefore, we rather lose the init hooks in this case.
                // (This will not normally happen because if first=true, there will be no inbound process.)
                if (first && !m_loop.isStopped()) {
                    executeCommandWait("C2$RunLoadHook", false, "Turn Initialisation");
                }

                // Run (this will immediately exit if one of the above scripts requested a context change.)
                m_loop.run();
                out = m_outputState;
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
                            info += afl::string::Format("!%d message%!1{s%}", game->currentTurn().inbox().getNumMessages());
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
client::screens::doPlayerScreen(Session& session, client::si::InputState& in, client::si::OutputState& out, bool first)
{
    PlayerScreen(session).run(in, out, first);
}
