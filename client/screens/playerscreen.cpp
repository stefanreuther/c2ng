/**
  *  \file client/screens/playerscreen.cpp
  */

#include <ctime>
#include "client/screens/playerscreen.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/classicvcrdialog.hpp"
#include "client/dialogs/inboxdialog.hpp"
#include "client/si/control.hpp"
#include "client/si/scripttask.hpp"
#include "client/vcr/classic/playbackscreen.hpp"
#include "client/widgets/keymapwidget.hpp"
#include "client/widgets/messageactionpanel.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "gfx/complex.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/flow.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/root.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/imagebutton.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/panel.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/transparentwindow.hpp"
#include "ui/window.hpp"
#include "util/requestreceiver.hpp"
#include "interpreter/values.hpp"
#include "interpreter/arguments.hpp"

namespace {
    const char*const KEYMAP_NAME = "RACESCREEN";

    ui::widgets::BaseButton& createImageButton(afl::base::Deleter& del, ui::Root& root, ui::LayoutableGroup& group, String_t text, util::Key_t key, String_t image)
    {
        // Create container group
        ui::widgets::FrameGroup& frame = del.addNew(new ui::widgets::FrameGroup(ui::layout::HBox::instance0, root.colorScheme(), ui::LoweredFrame));

        // Create button
        ui::widgets::ImageButton& result = del.addNew(new ui::widgets::ImageButton(image, key, root, gfx::Point(110, 110)));
        result.setText(text);

        // Connect and return
        frame.add(result);
        group.add(frame);
        return result;
    }

    ui::LayoutableGroup& createGroup(afl::base::Deleter& del, String_t name, ui::LayoutableGroup& parent, ui::Root& root, gfx::ColorScheme<util::SkinColor::Color>& colorScheme)
    {
        ui::LayoutableGroup& win = del.addNew(new ui::widgets::TransparentWindow(colorScheme, ui::layout::VBox::instance5));
        win.setState(win.ModalState, false);
        win.add(del.addNew(new ui::widgets::StaticText(name, util::SkinColor::Static, gfx::FontRequest().addSize(1), root.provider())));

        ui::LayoutableGroup& content = del.addNew(new ui::Group(del.addNew(new ui::layout::Flow(4, false))));
        win.add(content);

        parent.add(win);
        return content;
    }

    ui::widgets::BaseButton& createActionButton(afl::base::Deleter& del, ui::Root& root, ui::LayoutableGroup& group, String_t text, util::Key_t key)
    {
        ui::widgets::Button& btn = del.addNew(new ui::widgets::Button(text, key, root));
        group.add(btn);
        return btn;
    }

    class PlayerScreen : private client::si::Control {
     public:
        PlayerScreen(client::si::UserSide& us)
            : Control(us),
              m_loop(us.root()),
              m_docView(gfx::Point(200, 200), 0, us.root().provider()),
              m_receiver(us.root().engine().dispatcher(), *this),
              m_updateTrampoline(us.gameSender().makeTemporary(new TrampolineFromSession(m_receiver.getSender()))),
              m_outputState()
            { }

        ~PlayerScreen()
            { }

        void run(client::si::InputState& in, client::si::OutputState& out, gfx::ColorScheme<util::SkinColor::Color>& colorScheme, bool first)
            {
                // ex WPlayerScreen::init
                // Player screen
                //   HBox
                //     VBox
                //       FrameGroup + ImageButton
                //       Spacer
                //     VBox
                //       DocView
                //       Spacer
                //       Buttons...
                afl::string::Translator& tx = translator();
                afl::base::Deleter del;
                ui::Root& root = interface().root();

                ui::LayoutableGroup& panel = del.addNew(new ui::widgets::Panel(ui::layout::HBox::instance5, 10));
                panel.setColorScheme(colorScheme);

                // Keymap handler
                client::widgets::KeymapWidget& keys = del.addNew(new client::widgets::KeymapWidget(interface().gameSender(), root.engine().dispatcher(), *this));

                // Left group containing list of image buttons
                ui::LayoutableGroup& leftGroup = del.addNew(new ui::Group(ui::layout::VBox::instance5));
                createImageButton(del, root, leftGroup, tx("F1 - Starships"), util::Key_F1, "menu.ship").dispatchKeyTo(keys);
                createImageButton(del, root, leftGroup, tx("F2 - Planets"),   util::Key_F2, "menu.planet").dispatchKeyTo(keys);
                createImageButton(del, root, leftGroup, tx("F3 - Starbases"), util::Key_F3, "menu.base").dispatchKeyTo(keys);
                createImageButton(del, root, leftGroup, tx("F4 - Starchart"), util::Key_F4, "menu.chart").dispatchKeyTo(keys);
                leftGroup.add(del.addNew(new ui::Spacer()));
                panel.add(leftGroup);

                // Right group
                ui::LayoutableGroup& rightGroup = del.addNew(new ui::Group(ui::layout::VBox::instance5));
                rightGroup.add(m_docView);

                // Panels
                ui::LayoutableGroup& menuGroup = del.addNew(new ui::Group(del.addNew(new ui::layout::Flow(5, false))));

                ui::LayoutableGroup& diploGroup = createGroup(del, tx("Diplomacy"), menuGroup, root, colorScheme);
                createActionButton(del, root, diploGroup, tx("Alliances"), 'a').dispatchKeyTo(keys);
                createActionButton(del, root, diploGroup, tx("Data Transfer"), 'd').dispatchKeyTo(keys);
                createActionButton(del, root, diploGroup, tx("Teams"), 't').dispatchKeyTo(keys);

                ui::LayoutableGroup& researchGroup = createGroup(del, tx("Research"), menuGroup, root, colorScheme);
                createActionButton(del, root, researchGroup, tx("Search"), util::Key_F7).dispatchKeyTo(keys);
                createActionButton(del, root, researchGroup, tx("Imperial Stats"), 'i').dispatchKeyTo(keys);
                createActionButton(del, root, researchGroup, tx("Scores"), 's').dispatchKeyTo(keys);
                createActionButton(del, root, researchGroup, tx("Battle Simulator"), 'b').dispatchKeyTo(keys);
                createActionButton(del, root, researchGroup, tx("Starship Cost Calculator"), 'd').dispatchKeyTo(keys);
                createActionButton(del, root, researchGroup, tx("Almanac"), 'A').dispatchKeyTo(keys);

                ui::LayoutableGroup& messagesGroup = createGroup(del, tx("Messages"), menuGroup, root, colorScheme);
                createActionButton(del, root, messagesGroup, tx("Inbox"), 'm').dispatchKeyTo(keys);
                createActionButton(del, root, messagesGroup, tx("Write"), 'w').dispatchKeyTo(keys);
                createActionButton(del, root, messagesGroup, tx("Visual Combat Recordings"), 'v').dispatchKeyTo(keys);
                createActionButton(del, root, messagesGroup, tx("View util.dat"), util::KeyMod_Alt + 'u').dispatchKeyTo(keys);

                ui::LayoutableGroup& extraGroup = createGroup(del, tx("Extra"), menuGroup, root, colorScheme);
                createActionButton(del, root, extraGroup, tx("Options"), util::KeyMod_Ctrl + 'o').dispatchKeyTo(keys);
                createActionButton(del, root, extraGroup, tx("Console"), util::KeyMod_Alt + 'c').dispatchKeyTo(keys);
                createActionButton(del, root, extraGroup, tx("Process Manager"), util::KeyMod_Alt + 'p').dispatchKeyTo(keys);
                createActionButton(del, root, extraGroup, tx("Tip of the Day"), 't').dispatchKeyTo(keys);

                ui::LayoutableGroup& fleetGroup = createGroup(del, tx("Fleet"), menuGroup, root, colorScheme);
                createActionButton(del, root, fleetGroup, tx("Global Actions"), 'g').dispatchKeyTo(keys);
                createActionButton(del, root, fleetGroup, tx("Fleets"), util::Key_F10).dispatchKeyTo(keys);
                createActionButton(del, root, fleetGroup, tx("Ship History"), util::Key_F6).dispatchKeyTo(keys);
                createActionButton(del, root, fleetGroup, tx("Selection"), util::KeyMod_Alt + '.').dispatchKeyTo(keys);

                rightGroup.add(menuGroup);
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
                createActionButton(del, root, btnGroup, tx("ESC - Exit"), util::Key_Escape).dispatchKeyTo(keys);
                rightGroup.add(btnGroup);
                panel.add(rightGroup);

                // Publish UI properties
                util::RequestSender<Proprietor> prop(interface().gameSender().makeTemporary(new ProprietorFromSession()));

                // Finish and display it
                keys.setKeymapName(KEYMAP_NAME);
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
                class InitTask : public client::si::ScriptTask {
                 public:
                    virtual void execute(uint32_t pgid, game::Session& session)
                        {
                            // Access
                            interpreter::ProcessList& list = session.processList();

                            // Create a task to run the 'Load' and 'NewTurn' hooks, with high priority
                            {
                                interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
                                bco->addInstruction(interpreter::Opcode::maPush,
                                                    interpreter::Opcode::sNamedShared,
                                                    bco->addName("C2$RUNLOADHOOK"));
                                bco->addInstruction(interpreter::Opcode::maIndirect,
                                                    interpreter::Opcode::miIMCall, 0);
                                interpreter::Process& proc = list.create(session.world(), "Turn Initialisation");
                                proc.pushFrame(bco, false);
                                proc.setPriority(0);
                                list.handlePriorityChange(proc);
                                list.resumeProcess(proc, pgid);
                            }

                            // Revive all auto-tasks
                            list.resumeSuspendedProcesses(pgid);

                            // Create task to show notifications, with low priority
                            {
                                interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
                                bco->addInstruction(interpreter::Opcode::maPush,
                                                    interpreter::Opcode::sNamedShared,
                                                    bco->addName("C2$SHOWINITIALNOTIFICATIONS"));
                                bco->addInstruction(interpreter::Opcode::maIndirect,
                                                    interpreter::Opcode::miIMCall, 0);
                                interpreter::Process& proc = list.create(session.world(), "Turn Initialisation (2)");
                                proc.pushFrame(bco, false);
                                proc.setPriority(99);
                                list.handlePriorityChange(proc);
                                list.resumeProcess(proc, pgid);
                            }
                        }
                };

                if (first && !m_loop.isStopped()) {
                    std::auto_ptr<client::si::ScriptTask> p(new InitTask());
                    executeTaskWait(p);
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

        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
            {
                // ex WPlayerScreen::processEvent
                using client::si::OutputState;
                switch (target) {
                 case OutputState::NoChange:
                 case OutputState::PlayerScreen:
                    interface().continueProcess(link);
                    break;

                 case OutputState::ExitProgram:
                 case OutputState::ExitGame:
                 case OutputState::ShipScreen:
                 case OutputState::PlanetScreen:
                 case OutputState::BaseScreen:
                 case OutputState::ShipTaskScreen:
                 case OutputState::PlanetTaskScreen:
                 case OutputState::BaseTaskScreen:
                 case OutputState::Starchart:
                    interface().detachProcess(link);
                    m_outputState.set(link, target);
                    m_loop.stop(0);
                    break;
                }
            }
        virtual void handlePopupConsole(client::si::RequestLink2 link)
            { defaultHandlePopupConsole(link); }
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link)
            { defaultHandleScanKeyboardMode(link); }
        virtual void handleEndDialog(client::si::RequestLink2 link, int /*code*/)
            { interface().continueProcess(link); }
        virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
            { defaultHandleSetView(link, name, withKeymap); }
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
            { defaultHandleUseKeymap(link, name, prefix); }
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text)
            { defaultHandleOverlayMessage(link, text); }
        virtual game::interface::ContextProvider* createContextProvider()
            { return 0; }

     private:
        ui::EventLoop m_loop;
        ui::rich::DocumentView m_docView;
        util::RequestReceiver<PlayerScreen> m_receiver;

        class Proprietor : public game::interface::UserInterfacePropertyAccessor {
         public:
            Proprietor(game::Session& session)
                : m_session(session)
                { m_session.uiPropertyStack().add(*this); }
            ~Proprietor()
                { m_session.uiPropertyStack().remove(*this); }

            virtual bool get(game::interface::UserInterfaceProperty prop, std::auto_ptr<afl::data::Value>& result)
                {
                    // ex WPlayerScreen::getProperty
                    switch (prop) {
                     case game::interface::iuiScreenNumber:
                        result.reset(interpreter::makeIntegerValue(0));
                        return true;
                     case game::interface::iuiScreenRegistered:
                        return false;
                     case game::interface::iuiIterator:
                     case game::interface::iuiAutoTask:
                        result.reset();
                        return true;
                     case game::interface::iuiSimFlag:
                        result.reset(interpreter::makeBooleanValue(0));
                        return true;
                     case game::interface::iuiScanX:
                     case game::interface::iuiChartX:
                        getCursorLocation(game::map::Point::X, result);
                        return true;
                     case game::interface::iuiScanY:
                     case game::interface::iuiChartY:
                        getCursorLocation(game::map::Point::Y, result);
                        return true;
                     case game::interface::iuiKeymap:
                        result.reset(interpreter::makeStringValue(KEYMAP_NAME));
                        return true;
                    }
                    return false;
                }
            virtual bool set(game::interface::UserInterfaceProperty prop, const afl::data::Value* p)
                {
                    // ex WPlayerScreen::setProperty
                    switch (prop) {
                     case game::interface::iuiScreenNumber:
                     case game::interface::iuiScreenRegistered:
                     case game::interface::iuiIterator:
                     case game::interface::iuiAutoTask:
                     case game::interface::iuiSimFlag:
                     case game::interface::iuiKeymap:
                        return false;
                     case game::interface::iuiScanX:
                     case game::interface::iuiChartX:
                        setCursorLocation(game::map::Point::X, p);
                        return true;
                     case game::interface::iuiScanY:
                     case game::interface::iuiChartY:
                        setCursorLocation(game::map::Point::Y, p);
                        return true;
                    }
                    return false;
                }

         private:
            void getCursorLocation(game::map::Point::Component c, std::auto_ptr<afl::data::Value>& result)
                {
                    game::Game* pGame = m_session.getGame().get();
                    if (pGame == 0) {
                        result.reset();
                        return;
                    }

                    game::map::Point pt;
                    if (!pGame->cursors().location().getPosition(pt)) {
                        result.reset();
                        return;
                    }

                    result.reset(interpreter::makeIntegerValue(pt.get(c)));
                }

            void setCursorLocation(game::map::Point::Component c, const afl::data::Value* p)
                {
                    int32_t value;
                    if (!interpreter::checkIntegerArg(value, p, 0, game::MAX_NUMBER)) {
                        return;
                    }

                    game::Game* pGame = m_session.getGame().get();
                    if (pGame == 0) {
                        // Cannot happen
                        throw interpreter::Error::notAssignable();
                    }

                    // Get old position. If this fails, it leaves the point default-initialized.
                    // This is needed to bootstrap.
                    game::map::Point pt;
                    pGame->cursors().location().getPosition(pt);

                    // Only call set() if this actually is a change, to avoid losing track of an object.
                    if (value != pt.get(c)) {
                        pt.set(c, value);
                        pGame->cursors().location().set(pt);
                    }
                }

            game::Session& m_session;
        };

        class ProprietorFromSession : public afl::base::Closure<Proprietor*(game::Session&)> {
         public:
            virtual Proprietor* call(game::Session& session)
                { return new Proprietor(session); }
        };

        class Trampoline {
         public:
            Trampoline(game::Session& session, util::RequestSender<PlayerScreen> sender)
                : m_session(session),
                  m_sender(sender),
                  conn_root()
                {
                    if (game::Root* root = session.getRoot().get()) {
                        conn_root = root->playerList().sig_change.add(this, &Trampoline::onChange);
                    }
                    onChange();
                }
            void onChange()
                {
                    String_t info;
                    game::Root* root = m_session.getRoot().get();
                    game::Game* game = m_session.getGame().get();
                    if (root != 0 && game != 0) {
                        afl::string::Translator& tx = m_session.translator();
                        if (game::Player* p = root->playerList().get(game->getViewpointPlayer())) {
                            info = p->getName(game::Player::LongName);
                            if (!p->getName(game::Player::UserName).empty()) {
                                info += "\n";
                                info += p->getName(game::Player::UserName);
                            }
                        } else {
                            info = afl::string::Format(tx("Player %d"), game->getViewpointPlayer());
                        }
                        info += "\n";
                        info += afl::string::Format(tx("%d message%!1{s%}"), game->currentTurn().inbox().getNumMessages());
                        info += "\n";
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
            game::Session& m_session;
            util::RequestSender<PlayerScreen> m_sender;
            afl::base::SignalConnection conn_root;
        };

        class TrampolineFromSession : public afl::base::Closure<Trampoline*(game::Session&)> {
         public:
            TrampolineFromSession(const util::RequestSender<PlayerScreen>& sender)
                : m_sender(sender)
                { }
            virtual Trampoline* call(game::Session& session)
                { return new Trampoline(session, m_sender); }
         private:
            util::RequestSender<PlayerScreen> m_sender;
        };

        util::RequestSender<Trampoline> m_updateTrampoline;
        client::si::OutputState m_outputState;
    };
}

void
client::screens::doPlayerScreen(client::si::UserSide& us, client::si::InputState& in, client::si::OutputState& out, gfx::ColorScheme<util::SkinColor::Color>& colorScheme, bool first)
{
    PlayerScreen(us).run(in, out, colorScheme, first);
}
