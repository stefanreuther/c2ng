/**
  *  \file client/screens/controlscreen.cpp
  */

#include "client/screens/controlscreen.hpp"
#include "afl/base/refcounted.hpp"
#include "client/map/widget.hpp"
#include "client/objectcursorfactory.hpp"
#include "client/proxy/objectlistener.hpp"
#include "client/si/contextprovider.hpp"
#include "client/si/contextreceiver.hpp"
#include "client/si/control.hpp"
#include "client/si/genericwidgetvalue.hpp"
#include "client/si/widgetcommand.hpp"
#include "client/si/widgetwrapper.hpp"
#include "client/tiles/selectionheadertile.hpp"
#include "client/tiles/shipscreenheadertile.hpp"
#include "client/tiles/tilefactory.hpp"
#include "client/widgets/keymapwidget.hpp"
#include "game/interface/iteratorcontext.hpp"
#include "game/interface/planetcontext.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/interface/userinterfacepropertyaccessor.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/values.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/skincolorscheme.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/panel.hpp"
#include "util/slaveobject.hpp"
#include "client/proxy/cursorobserverproxy.hpp"

namespace {

    /* FIXME: as of 20180827, this object is shared between threads but not modified.
       Can we replace it by a by-value, not by-reference object? */
    struct ScreenState : public afl::base::RefCounted {
        int screenNumber;
        client::si::OutputState::Target ownTarget;

        game::map::ObjectCursor* getCursor(game::Session& session) const
            {
                if (game::Game* g = session.getGame().get()) {
                    return g->cursors().getCursorByNumber(screenNumber);
                } else {
                    return 0;
                }
            }

        game::map::Object* getObject(game::Session& session) const
            {
                if (game::map::ObjectCursor* c = getCursor(session)) {
                    return c->getCurrentObject();
                } else {
                    return 0;
                }
            }
    };

    class ScreenCursorFactory : public client::ObjectCursorFactory {
     public:
        ScreenCursorFactory(afl::base::Ref<ScreenState> state)
            : m_state(state)
            { }
        virtual game::map::ObjectCursor* getCursor(game::Session& session)
            {
                // Keep game alive
                m_game = session.getGame();

                // Get the cursor
                return m_state->getCursor(session);
            }
     private:
        afl::base::Ptr<game::Game> m_game;
        afl::base::Ref<ScreenState> m_state;
    };

    class ScreenContextProvider : public client::si::ContextProvider {
     public:
        ScreenContextProvider(afl::base::Ref<ScreenState> state)
            : m_state(state)
            { }
        virtual void createContext(game::Session& session, client::si::ContextReceiver& recv)
            {
                game::map::Object* obj = m_state->getObject(session);
                if (dynamic_cast<game::map::Ship*>(obj) != 0) {
                    if (interpreter::Context* ctx = game::interface::ShipContext::create(obj->getId(), session)) {
                        recv.addNewContext(ctx);
                    }
                } else if (dynamic_cast<game::map::Planet*>(obj) != 0) {
                    if (interpreter::Context* ctx = game::interface::PlanetContext::create(obj->getId(), session)) {
                        recv.addNewContext(ctx);
                    }
                } else {
                    // FIXME?
                }
            }
     private:
        afl::base::Ref<ScreenState> m_state;
    };

    class ScreenUserInterfaceProperties : public util::SlaveObject<game::Session>,
                                          public game::interface::UserInterfacePropertyAccessor
    {
     public:
        ScreenUserInterfaceProperties(afl::base::Ref<ScreenState> state)
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
                m_pSession = 0;
            }

        virtual bool get(game::interface::UserInterfaceProperty prop, std::auto_ptr<afl::data::Value>& result)
            {
                switch (prop) {
                 case game::interface::iuiScreenNumber:
                    result.reset(interpreter::makeIntegerValue(m_state->screenNumber));
                    return true;
                 case game::interface::iuiIterator:
                    if (m_pSession != 0 && m_pSession->getGame().get() != 0) {
                        result.reset(game::interface::makeIteratorValue(m_pSession->getGame(), m_state->screenNumber, false));
                    } else {
                        result.reset();
                    }
                    return true;
                 case game::interface::iuiSimFlag:
                    result.reset(interpreter::makeBooleanValue(0));
                    return true;
                 case game::interface::iuiScanX:
                 case game::interface::iuiScanY:
                    // FIXME: implement this! iuiScanX, iuiScanY
                    result.reset();
                    return true;
                 case game::interface::iuiChartX:
                 case game::interface::iuiChartY:
                    result.reset();
                    if (m_pSession != 0) {
                        if (game::map::MapObject* obj = dynamic_cast<game::map::MapObject*>(m_state->getObject(*m_pSession))) {
                            game::map::Point pt;
                            if (obj->getPosition(pt)) {
                                if (prop == game::interface::iuiChartX) {
                                    result.reset(interpreter::makeIntegerValue(pt.getX()));
                                } else {
                                    result.reset(interpreter::makeIntegerValue(pt.getY()));
                                }
                            }
                        }
                    }
                    return true;
                }
                return false;
            }
        virtual bool set(game::interface::UserInterfaceProperty /*prop*/, afl::data::Value* /*p*/)
            { return false; }
     private:
        game::Session* m_pSession;
        afl::base::Ref<ScreenState> m_state;
    };


    class ScreenControl : public client::si::Control {
     public:
        ScreenControl(client::Session& session, ui::EventLoop& loop, afl::base::Ref<ScreenState> state, client::si::OutputState& out)
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
        afl::base::Ref<ScreenState> m_state;
    };

    class ScreenMapUpdater : public client::proxy::ObjectListener {
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

const client::screens::ControlScreen::Definition client::screens::ControlScreen::ShipScreen = {
    client::si::OutputState::ShipScreen,
    "SHIPSCREEN",
    "SHIPSCREEN",
};
const client::screens::ControlScreen::Definition client::screens::ControlScreen::PlanetScreen = {
    client::si::OutputState::PlanetScreen,
    "PLANETSCREEN",
    "PLANETSCREEN",
};
const client::screens::ControlScreen::Definition client::screens::ControlScreen::BaseScreen = {
    client::si::OutputState::BaseScreen,
    "BASESCREEN",
    "BASESCREEN",
};

client::screens::ControlScreen::ControlScreen(Session& session, int nr, const Definition& def)
    : m_session(session),
      m_number(nr),
      m_definition(def)
{ }

void
client::screens::ControlScreen::run(client::si::InputState& in, client::si::OutputState& out)
{
    // Set up common state
    afl::base::Deleter deleter;
    afl::base::Ref<ScreenState> state(*new ScreenState());
    state->screenNumber = m_number;
    state->ownTarget = m_definition.target;

    // An event loop
    ui::Root& root = m_session.root();
    ui::EventLoop loop(root);

    // Control
    ScreenControl ctl(m_session, loop, state, out);

    // Build it
    ui::widgets::Panel panel(ui::layout::HBox::instance5, 2);
    ui::SkinColorScheme panelColors(ui::DARK_COLOR_SET, root.colorScheme());
    panel.setColorScheme(panelColors);
    client::widgets::KeymapWidget keys(m_session.gameSender(), root.engine().dispatcher(), ctl);
    client::proxy::CursorObserverProxy oop(m_session.gameSender(), std::auto_ptr<client::ObjectCursorFactory>(new ScreenCursorFactory(state)));

    ui::Group tileGroup(ui::layout::VBox::instance5);
    client::tiles::TileFactory(root, m_session.interface(), keys, oop).createLayout(tileGroup, m_definition.layoutName, deleter);
    tileGroup.add(deleter.addNew(new ui::Spacer()));
    panel.add(tileGroup);

    client::map::Widget map(ctl.interface().gameSender(), root, gfx::Point(300, 300));
    keys.setKeymapName(m_definition.keymapName);

    ui::PrefixArgument prefix(root);

    panel.add(keys);
    panel.add(prefix);
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
