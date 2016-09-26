/**
  *  \file client/screens/controlscreen.cpp
  */

#include "client/screens/controlscreen.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/interface/planetcontext.hpp"
#include "client/si/contextprovider.hpp"
#include "client/objectcursorfactory.hpp"
#include "util/slaveobject.hpp"
#include "game/interface/userinterfacepropertyaccessor.hpp"
#include "interpreter/values.hpp"
#include "game/interface/iteratorcontext.hpp"
#include "client/si/control.hpp"
#include "ui/window.hpp"
#include "ui/layout/vbox.hpp"
#include "client/map/widget.hpp"
#include "client/widgets/keymapwidget.hpp"
#include "client/objectobserverproxy.hpp"
#include "client/tiles/selectionheadertile.hpp"
#include "client/objectlistener.hpp"

namespace {

    struct ScreenState {
        int screenNumber;
        client::si::OutputState::Target ownTarget;
    };

    class ScreenCursorFactory : public client::ObjectCursorFactory {
     public:
        ScreenCursorFactory(afl::base::Ptr<ScreenState> state)
            : m_state(state)
            { }
        virtual game::map::ObjectCursor* getCursor(game::Session& session)
            {
                // Keep game alive
                m_game = session.getGame();

                // Get the cursor
                if (game::Game* g = m_game.get()) {
                    return g->cursors().getCursorByNumber(m_state->screenNumber);
                } else {
                    return 0;
                }
            }
     private:
        afl::base::Ptr<game::Game> m_game;
        afl::base::Ptr<ScreenState> m_state;
    };

    class ScreenContextProvider : public client::si::ContextProvider {
     public:
        ScreenContextProvider(afl::base::Ptr<ScreenState> state)
            : m_state(state)
            { }
        virtual void createContext(game::Session& session, interpreter::Process& proc)
            {
                game::Game* g = session.getGame().get();
                game::Root* r = session.getRoot().get();
                game::spec::ShipList* sl = session.getShipList().get();
                if (g != 0 && r != 0 && sl != 0 && m_state.get() != 0) {
                    game::map::ObjectCursor* c = g->cursors().getCursorByNumber(m_state->screenNumber);
                    if (c != 0) {
                        game::map::Object* obj = c->getCurrentObject();
                        if (dynamic_cast<game::map::Ship*>(obj) != 0) {
                            proc.pushNewContext(new game::interface::ShipContext(obj->getId(),
                                                                                 session,
                                                                                 session.getRoot(),
                                                                                 session.getGame(),
                                                                                 session.getShipList()));
                        } else if (dynamic_cast<game::map::Planet*>(obj) != 0) {
                            proc.pushNewContext(new game::interface::PlanetContext(obj->getId(),
                                                                                   session,
                                                                                   session.getRoot(),
                                                                                   session.getGame()));
                        } else {
                            // FIXME?
                        }
                    }
                }
            }
     private:
        afl::base::Ptr<ScreenState> m_state;
    };

    class ScreenUserInterfaceProperties : public util::SlaveObject<game::Session>,
                                          public game::interface::UserInterfacePropertyAccessor
    {
     public:
        ScreenUserInterfaceProperties(afl::base::Ptr<ScreenState> state)
            : m_pSession(0),
              m_state(state)
            { }
        virtual void init(game::Session& master)
            {
                master.uiPropertyStack().add(*this);
                m_pSession = &master;
            }
        virtual void done(game::Session& master)
            {
                master.uiPropertyStack().remove(*this);
            }

        virtual bool get(game::interface::UserInterfaceProperty prop, std::auto_ptr<afl::data::Value>& result)
            {
                switch (prop) {
                 case game::interface::iuiScreenNumber:
                    result.reset(interpreter::makeIntegerValue(1));
                    return true;
                 case game::interface::iuiIterator:
                    if (m_pSession != 0 && m_pSession->getGame().get() != 0) {
                        result.reset(game::interface::makeIteratorValue(m_pSession->getGame(), m_state->screenNumber));
                    } else {
                        result.reset();
                    }
                    return true;
                 case game::interface::iuiSimFlag:
                    result.reset(interpreter::makeBooleanValue(0));
                    return true;
                 case game::interface::iuiScanX:
                 case game::interface::iuiScanY:
                 case game::interface::iuiChartX:
                 case game::interface::iuiChartY:
                    result.reset();
                    return true;
                }
                return false;
            }
        virtual bool set(game::interface::UserInterfaceProperty /*prop*/, afl::data::Value* /*p*/)
            { return false; }
     private:
        game::Session* m_pSession;
        afl::base::Ptr<ScreenState> m_state;
    };


    class ScreenControl : public client::si::Control {
     public:
        ScreenControl(client::Session& session, ui::EventLoop& loop, afl::base::Ptr<ScreenState> state, client::si::OutputState& out)
            : Control(session.interface(), session.root(), session.translator()),
              m_session(session),
              m_loop(loop),
              m_outputState(out),
              m_state(state)
            { }

        ~ScreenControl()
            { }

        virtual void handleStateChange(client::si::UserSide& us, client::si::RequestLink2 link, client::si::OutputState::Target target)
            {
                using client::si::OutputState;
                switch (target) {
                 case OutputState::NoChange:
                    us.continueProcess(link);
                    break;

                 case OutputState::ShipScreen: // FIXME: check whether it's me
                 case OutputState::PlanetScreen: // FIXME: check whether it's me
                 case OutputState::BaseScreen: // FIXME: check whether it's me
                    if (target == m_state->ownTarget) {
                        us.continueProcess(link);
                    } else {
                        us.detachProcess(link);
                        m_outputState.set(link, target);
                        m_loop.stop(0);
                    }
                    break;

                 case OutputState::ExitProgram:
                 case OutputState::ExitGame:
                 case OutputState::PlayerScreen:
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
                // This is not a dialog.
                ui.continueProcess(link);
            }

        virtual client::si::ContextProvider* createContextProvider()
            { return new ScreenContextProvider(m_state); }

     private:
        client::Session& m_session;
        ui::EventLoop& m_loop;
        client::si::OutputState& m_outputState;
        afl::base::Ptr<ScreenState> m_state;
    };

    class ScreenMapUpdater : public client::ObjectListener {
     public:
        ScreenMapUpdater(util::RequestSender<client::map::Widget> sender)
            : m_sender(sender)
            { }
        virtual void handle(game::Session&, game::map::Object* obj)
            {
                if (game::map::MapObject* mo = dynamic_cast<game::map::MapObject*>(obj)) {
                    game::map::Point pt;
                    if (mo->getPosition(pt)) {
                        class Req : public util::Request<client::map::Widget> {
                         public:
                            Req(game::map::Point pt)
                                : m_point(pt)
                                { }
                            virtual void handle(client::map::Widget& w)
                                { w.setCenter(m_point); }
                         private:
                            game::map::Point m_point;
                        };
                        m_sender.postNewRequest(new Req(pt));
                    }
                }
            }
     private:
        util::RequestSender<client::map::Widget> m_sender;
    };
}


client::screens::ControlScreen::ControlScreen(Session& session, int nr, client::si::OutputState::Target me)
    : m_session(session),
      m_number(nr),
      m_me(me)
{ }

void
client::screens::ControlScreen::run(client::si::InputState& in, client::si::OutputState& out)
{
    // Set up common state
    afl::base::Ptr<ScreenState> state(new ScreenState());
    state->screenNumber = m_number;
    state->ownTarget = m_me;

    // An event loop
    ui::Root& root = m_session.root();
    ui::EventLoop loop(root);

    // Control
    ScreenControl ctl(m_session, loop, state, out);

    // Build it
    ui::Window panel("!Control SCreen", root.provider(), ui::BLUE_BLACK_WINDOW, ui::layout::VBox::instance5);
    client::widgets::KeymapWidget keys(m_session.gameSender(), root.engine().dispatcher(), ctl);
    client::map::Widget map(ctl.interface().gameSender(), root, gfx::Point(300, 300));
    keys.setKeymapName("SHIPSCREEN"/*FIXME*/);

    client::ObjectObserverProxy oop(m_session.gameSender(), std::auto_ptr<client::ObjectCursorFactory>(new ScreenCursorFactory(state)));
    client::tiles::SelectionHeaderTile sht(root, keys);
    sht.attach(oop);

    panel.add(keys);
    panel.add(sht);
    panel.add(map);
    panel.setExtent(root.getExtent());
    panel.setState(ui::Widget::ModalState, true);
    root.add(panel);

    util::SlaveRequestSender<game::Session,ScreenUserInterfaceProperties> dialogUIP(m_session.gameSender(), new ScreenUserInterfaceProperties(state));
    util::RequestReceiver<client::map::Widget> mapReceiver(root.engine().dispatcher(), map);
    oop.addNewListener(new ScreenMapUpdater(mapReceiver.getSender()));
                
    ctl.continueProcessWait(in.getProcess());
    loop.run();
}
