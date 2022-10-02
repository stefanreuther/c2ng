/**
  *  \file client/screens/controlscreen.cpp
  */

#include "client/screens/controlscreen.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/refcounted.hpp"
#include "client/map/scanneroverlay.hpp"
#include "client/map/shiptaskoverlay.hpp"
#include "client/map/waypointoverlay.hpp"
#include "client/map/widget.hpp"
#include "client/si/control.hpp"
#include "client/si/genericwidgetvalue.hpp"
#include "client/si/widgetcommand.hpp"
#include "client/si/widgetwrapper.hpp"
#include "client/tiles/selectionheadertile.hpp"
#include "client/tiles/shipscreenheadertile.hpp"
#include "client/tiles/tilefactory.hpp"
#include "client/widgets/scanresult.hpp"
#include "game/interface/contextprovider.hpp"
#include "game/interface/iteratorcontext.hpp"
#include "game/interface/planetcontext.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/interface/taskeditorcontext.hpp"
#include "game/interface/userinterfacepropertyaccessor.hpp"
#include "game/map/objectcursorfactory.hpp"
#include "game/proxy/cursorobserverproxy.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/proxy/taskeditorproxy.hpp"
#include "game/turn.hpp"
#include "gfx/complex.hpp"
#include "interpreter/contextreceiver.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/values.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/keyforwarder.hpp"

using interpreter::makeBooleanValue;
using interpreter::makeIntegerValue;
using game::map::Point;

namespace {
    class ScreenCursorFactory : public game::map::ObjectCursorFactory {
     public:
        ScreenCursorFactory(afl::base::Ref<client::screens::ControlScreen::State> state)
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
        afl::base::Ref<client::screens::ControlScreen::State> m_state;
    };


    /*
     *  Color Scheme to draw that awesome shade on control screens
     */

    class ControlScreenColorScheme : public gfx::ColorScheme<util::SkinColor::Color> {
     public:
        ControlScreenColorScheme(gfx::ResourceProvider& provider, String_t imageName, ui::Widget& owningWidget, const ui::ColorSet& colors, ui::ColorScheme& uiColorScheme);

        virtual gfx::Color_t getColor(util::SkinColor::Color index);
        virtual void drawBackground(gfx::Canvas& can, const gfx::Rectangle& area);

     private:
        gfx::ResourceProvider& m_provider;
        String_t m_imageName;
        ui::Widget& m_owningWidget;
        const ui::ColorSet& m_colors;
        ui::ColorScheme& m_uiColorScheme;

        afl::base::Ptr<gfx::Canvas> m_image;
        bool m_imageFinal;
        afl::base::SignalConnection conn_imageChange;

        void onImageChange();
        void requestImage();
    };


    bool isHyperdriving(game::Session& session, const game::map::Object* mo)
    {
        const game::map::Ship* pShip = dynamic_cast<const game::map::Ship*>(mo);
        const game::Game* pGame = session.getGame().get();
        const game::spec::ShipList* pShipList = session.getShipList().get();
        const game::Root* pRoot = session.getRoot().get();

        return pGame != 0
            && pShipList != 0
            && pRoot != 0
            && pShip != 0
            && pShip->isHyperdriving(pGame->shipScores(), *pShipList, pRoot->hostConfiguration());
    }

    game::map::Object* getFleetLeader(game::Session& session, game::map::Object* mo)
    {
        game::map::Ship* sh = dynamic_cast<game::map::Ship*>(mo);
        game::Game* pGame = session.getGame().get();
        if (sh != 0 && pGame != 0 && sh->getFleetNumber() != 0) {
            if (game::Turn* pTurn = pGame->getViewpointTurn().get()) {
                if (game::map::Ship* leader = pTurn->universe().ships().get(sh->getFleetNumber())) {
                    return leader;
                }
            }
        }
        return mo;
    }

}

/*
 *  ControlScreenColorScheme
 */

ControlScreenColorScheme::ControlScreenColorScheme(gfx::ResourceProvider& provider,
                                                   String_t imageName,
                                                   ui::Widget& owningWidget,
                                                   const ui::ColorSet& colors,
                                                   ui::ColorScheme& uiColorScheme)
    : m_provider(provider),
      m_imageName(imageName),
      m_owningWidget(owningWidget),
      m_colors(colors),
      m_uiColorScheme(uiColorScheme),
      m_image(),
      m_imageFinal(false),
      conn_imageChange()
{
    requestImage();
}

gfx::Color_t
ControlScreenColorScheme::getColor(util::SkinColor::Color index)
{
    if (size_t(index) < util::SkinColor::NUM_COLORS) {
        return m_uiColorScheme.getColor(m_colors[index]);
    } else {
        return m_uiColorScheme.getColor(0);
    }
}

void
ControlScreenColorScheme::drawBackground(gfx::Canvas& can, const gfx::Rectangle& area)
{
    // ex WControlScreen::Skin::drawBackground
    // Draw solid
    gfx::Context<uint8_t> ctx(can, m_uiColorScheme);
    drawSolidBar(ctx, area, ui::Color_Grayscale+5);

    // Draw image
    if (m_image.get() != 0) {
        // Widget size defines anchor point
        gfx::Rectangle widgetSize = m_owningWidget.getExtent();

        // Area the image can fill, in screen coordinates
        gfx::Rectangle pixArea(widgetSize.getTopLeft(), m_image->getSize());

        // Area we want to fill with image
        gfx::Rectangle fillArea(area);
        fillArea.intersect(pixArea);
        fillArea.moveBy(gfx::Point() - widgetSize.getTopLeft());

        // Draw pixmap
        if (fillArea.exists()) {
            can.blit(widgetSize.getTopLeft(), *m_image, fillArea);
        }
    }
}

void
ControlScreenColorScheme::onImageChange()
{
    requestImage();
    if (m_imageFinal) {
        m_owningWidget.requestRedraw();
    }
}

void
ControlScreenColorScheme::requestImage()
{
    // Try to obtain image
    m_image = m_provider.getImage(m_imageName, &m_imageFinal);
    if (m_imageFinal) {
        conn_imageChange.disconnect();
    } else {
        if (!conn_imageChange.isConnected()) {
            conn_imageChange = m_provider.sig_imageChange.add(this, &ControlScreenColorScheme::onImageChange);
        }
    }
}


/*
 *  Control Screen Definitions
 */

const client::screens::ControlScreen::Definition client::screens::ControlScreen::ShipScreen = {
    client::si::OutputState::ShipScreen,
    ScreenHistory::Ship,
    interpreter::Process::pkDefault,
    true,
    "SHIPSCREEN",
    "SHIPSCREEN",
};
const client::screens::ControlScreen::Definition client::screens::ControlScreen::PlanetScreen = {
    client::si::OutputState::PlanetScreen,
    ScreenHistory::Planet,
    interpreter::Process::pkDefault,
    false,
    "PLANETSCREEN",
    "PLANETSCREEN",
};
const client::screens::ControlScreen::Definition client::screens::ControlScreen::BaseScreen = {
    client::si::OutputState::BaseScreen,
    ScreenHistory::Starbase,
    interpreter::Process::pkDefault,
    false,
    "BASESCREEN",
    "BASESCREEN",
};
const client::screens::ControlScreen::Definition client::screens::ControlScreen::HistoryScreen = {
    client::si::OutputState::HistoryScreen,
    ScreenHistory::HistoryShip,
    interpreter::Process::pkDefault,
    true,
    "HISTORYSCREEN",
    "HISTORYSCREEN",
};
const client::screens::ControlScreen::Definition client::screens::ControlScreen::FleetScreen = {
    client::si::OutputState::FleetScreen,
    ScreenHistory::Fleet,
    interpreter::Process::pkDefault,
    true,
    "FLEETSCREEN",
    "FLEETSCREEN",
};
const client::screens::ControlScreen::Definition client::screens::ControlScreen::ShipTaskScreen = {
    client::si::OutputState::ShipTaskScreen,
    ScreenHistory::ShipTask,
    interpreter::Process::pkShipTask,
    true,
    "SHIPTASKSCREEN",
    "SHIPTASKSCREEN",
};
const client::screens::ControlScreen::Definition client::screens::ControlScreen::PlanetTaskScreen = {
    client::si::OutputState::PlanetTaskScreen,
    ScreenHistory::PlanetTask,
    interpreter::Process::pkPlanetTask,
    false,
    "PLANETTASKSCREEN",
    "PLANETTASKSCREEN",
};
const client::screens::ControlScreen::Definition client::screens::ControlScreen::BaseTaskScreen = {
    client::si::OutputState::BaseTaskScreen,
    ScreenHistory::StarbaseTask,
    interpreter::Process::pkBaseTask,
    false,
    "BASETASKSCREEN",
    "BASETASKSCREEN",
};


/*
 *  State
 */

game::map::ObjectCursor*
client::screens::ControlScreen::State::getCursor(game::Session& session) const
{
    if (game::Game* g = session.getGame().get()) {
        int effScreenNumber = (screenNumber == game::map::Cursors::FleetScreen ? game::map::Cursors::ShipScreen : screenNumber);
        return g->cursors().getCursorByNumber(effScreenNumber);
    } else {
        return 0;
    }
}

game::map::Object*
client::screens::ControlScreen::State::getObject(game::Session& session) const
{
    if (game::map::ObjectCursor* c = getCursor(session)) {
        return c->getCurrentObject();
    } else {
        return 0;
    }
}


/*
 *  Context Provider
 */

class client::screens::ControlScreen::ContextProvider : public game::interface::ContextProvider {
 public:
    ContextProvider(afl::base::Ref<State> state)
        : m_state(state)
        { }
    virtual void createContext(game::Session& session, interpreter::ContextReceiver& recv)
        {
            // FIXME: make a function
            game::map::Object* obj = m_state->getObject(session);
            if (interpreter::Context* ctx = game::interface::createObjectContext(obj, session)) {
                recv.pushNewContext(ctx);
            }
        }
 private:
    afl::base::Ref<State> m_state;
};


/*
 *  Updater
 */

class client::screens::ControlScreen::Updater : public game::proxy::ObjectListener {
 public:
    Updater(util::RequestSender<ControlScreen> reply, bool isFleet)
        : m_reply(reply),
          m_lastObject(0),
          m_lastPosition(),
          m_lastHyp(),
          m_isFleet(isFleet)
        { }
    virtual void handle(game::Session& session, game::map::Object* obj)
        {
            game::map::Object* mo = obj;

            // If this is a fleet, we want to look at the fleet leader instead
            if (m_isFleet) {
                mo = getFleetLeader(session, mo);
            }

            Point pt;
            bool hasPosition = mo != 0 && mo->getPosition(pt);
            bool isHyp = isHyperdriving(session, mo);

            if (mo != 0 && (mo != m_lastObject || pt != m_lastPosition)) {
                // Normal update: new object chosen (or position changed).
                // Update everything.
                Point target;
                game::map::Ship* pShip = dynamic_cast<game::map::Ship*>(mo);
                if (pShip == 0 || !pShip->getWaypoint().get(target)) {
                    target = pt;
                }

                class Req : public util::Request<ControlScreen> {
                 public:
                    Req(bool hasPosition, Point pt, Point target, game::Id_t id, bool isHyperdriving)
                        : m_hasPosition(hasPosition), m_isHyperdriving(isHyperdriving), m_point(pt), m_target(target), m_id(id)
                        { }
                    virtual void handle(ControlScreen& cs)
                        {
                            // Note that this will configure m_movementOverlay.setLockOrigin() for the actual ship's value,
                            // and later update through the TaskEditorProxy again to the task's value.
                            cs.setId(m_id);
                            cs.setPositions(m_point, m_target, m_isHyperdriving);
                        }
                 private:
                    bool m_hasPosition;
                    bool m_isHyperdriving;
                    Point m_point;
                    Point m_target;
                    game::Id_t m_id;
                };

                m_reply.postNewRequest(new Req(hasPosition, pt, target, mo->getId(), isHyp));
                m_lastPosition = pt;
                m_lastObject = mo;
                m_lastHyp = isHyp;
            } else if (isHyp != m_lastHyp) {
                // Only hyperdrive changed; only update that.
                class Req : public util::Request<ControlScreen> {
                 public:
                    Req(bool isHyperdriving)
                        : m_isHyperdriving(isHyperdriving)
                        { }
                    virtual void handle(ControlScreen& cs)
                        {
                            // Do not update for Ship Task Editor. In that case, the "hyperdriving" flag is controlled by the TaskEditor.
                            if (cs.m_taskEditorProxy.get() == 0) {
                                cs.setIsHyperdriving(m_isHyperdriving);
                            }
                        }
                 private:
                    bool m_isHyperdriving;
                };
                m_reply.postNewRequest(new Req(isHyp));
                m_lastHyp = isHyp;
            }
        }
 private:
    util::RequestSender<ControlScreen> m_reply;
    game::map::Object* m_lastObject;
    Point m_lastPosition;
    bool m_lastHyp;
    bool m_isFleet;
};


/*
 *  Proprietor
 *  (A proprietor is someone who has properties, right?)
 *
 *  This class provides user interface properties to scripts.
 *  It lives on the script side.
 */

class client::screens::ControlScreen::Proprietor : public game::interface::UserInterfacePropertyAccessor
{
 public:
    Proprietor(game::Session& session, afl::base::Ref<State> state, util::RequestSender<ControlScreen> reply)
        : m_session(session),
          m_state(state),
          m_reply(reply),
          m_scanPosition()
        { m_session.uiPropertyStack().add(*this); }
    ~Proprietor()
        { m_session.uiPropertyStack().remove(*this); }

    virtual bool get(game::interface::UserInterfaceProperty prop, std::auto_ptr<afl::data::Value>& result);
    virtual bool set(game::interface::UserInterfaceProperty prop, const afl::data::Value* p);

    void setScannerPosition(afl::base::Optional<Point> p)
        { m_scanPosition = p; }

 private:
    game::Session& m_session;
    afl::base::Ref<State> m_state;
    util::RequestSender<ControlScreen> m_reply;

    afl::base::Optional<Point> m_scanPosition;
};

bool
client::screens::ControlScreen::Proprietor::get(game::interface::UserInterfaceProperty prop, std::auto_ptr<afl::data::Value>& result)
{
    // ex WControlScreen::getProperty
    switch (prop) {
     case game::interface::iuiScreenRegistered:
        // Not exported to script world
        result.reset(makeBooleanValue(true));
        return true;

     case game::interface::iuiScreenNumber:
        // UI.Screen: from state
        result.reset(makeIntegerValue(m_state->screenNumber));
        return true;

     case game::interface::iuiAutoTask:
        // UI.AutoTask
        result.reset();
        if (game::map::Object* obj = m_state->getObject(m_session)) {
            result.reset(game::interface::TaskEditorContext::create(m_session, m_state->taskType, obj->getId()));
        }
        return true;

     case game::interface::iuiIterator:
        // UI.Iterator: created from state
        if (m_session.getGame().get() != 0) {
            result.reset(game::interface::makeIteratorValue(m_session, m_state->screenNumber));
        } else {
            result.reset();
        }
        return true;

     case game::interface::iuiSimFlag:
        // System.Sim: we are not simulating
        result.reset(interpreter::makeBooleanValue(0));
        return true;

     case game::interface::iuiScanX:
     case game::interface::iuiScanY:
        // UI.X/Y: scanner position, provided by UI
        if (const Point* pt = m_scanPosition.get()) {
            if (prop == game::interface::iuiScanX) {
                result.reset(makeIntegerValue(pt->getX()));
            } else {
                result.reset(makeIntegerValue(pt->getY()));
            }
        } else {
            result.reset();
        }
        return true;

     case game::interface::iuiChartX:
     case game::interface::iuiChartY:
        // Chart.X/Y: object position, provided by game
        result.reset();
        if (game::map::Object* obj = m_state->getObject(m_session)) {
            Point pt;
            if (obj->getPosition(pt)) {
                if (prop == game::interface::iuiChartX) {
                    result.reset(makeIntegerValue(pt.getX()));
                } else {
                    result.reset(makeIntegerValue(pt.getY()));
                }
            }
        }
        return true;

     case game::interface::iuiKeymap:
        // UI.Keymap: name of keymap
        // @change In PCC2, this is the keymap object
        result.reset(interpreter::makeStringValue(m_state->keymapName));
        return true;
    }
    return false;
}

bool
client::screens::ControlScreen::Proprietor::set(game::interface::UserInterfaceProperty prop, const afl::data::Value* p)
{
    // ex WControlScreen::setProperty
    // Do it
    switch (prop) {
     case game::interface::iuiScanX:
     case game::interface::iuiScanY:
        if (const Point* pt = m_scanPosition.get()) {
            int32_t iv;
            if (interpreter::checkIntegerArg(iv, p, 0, 10000)) {
                // Remember new position
                Point newPos = (prop == game::interface::iuiScanX
                                ? Point(iv, pt->getY())
                                : Point(pt->getX(), iv));
                m_scanPosition = newPos;

                // Update UI
                m_reply.postRequest(&ControlScreen::onScannerMove, newPos);
            }
            return true;
        } else {
            // If we have no scanner position, we don't have a scanner.
            // We don't need to allow scripts to assign it component-wise.
            throw interpreter::Error::notAssignable();
        }

     default:
        // FIXME: reconsider: PCC2 threw directly (but didn't have a stack).
        return false;
    }
}


class client::screens::ControlScreen::ProprietorFromSession : public afl::base::Closure<Proprietor*(game::Session&)> {
 public:
    ProprietorFromSession(const afl::base::Ref<State>& state, const util::RequestSender<ControlScreen>& reply)
        : m_state(state), m_reply(reply)
        { }
    virtual Proprietor* call(game::Session& session)
        { return new Proprietor(session, m_state, m_reply); }
 private:
    afl::base::Ref<State> m_state;
    util::RequestSender<ControlScreen> m_reply;
};


/*
 *  Control Screen
 */

client::screens::ControlScreen::ControlScreen(client::si::UserSide& us, int nr, const Definition& def)
    : Control(us),
      m_number(nr),
      m_definition(def),
      m_state(*new State(nr, def.target, def.taskType, def.keymapName)),
      m_deleter(),
      m_loop(us.root()),
      m_outputState(),
      m_panel(ui::layout::HBox::instance5, 2),
      m_mapWidget(interface().gameSender(), root(), gfx::Point(300, 300)),
      m_scannerOverlay(root().colorScheme()),
      m_movementOverlay(root().engine().dispatcher(), interface().gameSender(), m_mapWidget, us.translator()),
      m_minefieldOverlay(root(), us.translator()),
      m_scanResult(root(), interface().gameSender(), translator()),
      m_keymapWidget(interface().gameSender(), root().engine().dispatcher(), *this),
      m_center(),
      m_id(),
      m_taskEditorProxy(),
      m_taskKind(interpreter::Process::pkDefault),
      m_fleetProxy(),
      m_historyAdaptor(),
      m_reply(us.root().engine().dispatcher(), *this),
      m_proprietor(us.gameSender().makeTemporary(new ProprietorFromSession(m_state, m_reply.getSender())))
{
    m_mapWidget.setMode(client::map::Widget::ScannerMode);
}

client::screens::ControlScreen&
client::screens::ControlScreen::withTaskEditor(interpreter::Process::ProcessKind kind)
{
    m_taskEditorProxy.reset(new game::proxy::TaskEditorProxy(interface().gameSender(), root().engine().dispatcher()));
    m_taskKind = kind;
    return *this;
}

client::screens::ControlScreen&
client::screens::ControlScreen::withFleetProxy()
{
    m_fleetProxy.reset(new game::proxy::FleetProxy(interface().gameSender(), root().engine().dispatcher()));
    return *this;
}

client::screens::ControlScreen&
client::screens::ControlScreen::withHistoryAdaptor()
{
    m_historyAdaptor.reset(new client::tiles::HistoryAdaptor(interface().gameSender(), root().engine().dispatcher()));
    return *this;
}

void
client::screens::ControlScreen::run(client::si::InputState& in, client::si::OutputState& out)
{
    // Set up common state
    afl::base::Deleter deleter;
    ui::Root& root = interface().root();

    // Build it
    ControlScreenColorScheme panelColors(root.provider(), "bg.cscreen", m_panel, ui::DARK_COLOR_SET, root.colorScheme());
    m_panel.setColorScheme(panelColors);
    game::proxy::CursorObserverProxy oop(interface().gameSender(), std::auto_ptr<game::map::ObjectCursorFactory>(new ScreenCursorFactory(m_state)));

    ui::Group tileGroup(ui::layout::VBox::instance5);
    client::tiles::TileFactory(interface(), *this, oop)
        .withTaskEditorProxy(m_taskEditorProxy.get())
        .withFleetProxy(m_fleetProxy.get())
        .withHistoryAdaptor(m_historyAdaptor.get())
        .createLayout(tileGroup, m_definition.layoutName, deleter);
    tileGroup.add(deleter.addNew(new ui::Spacer()));
    m_panel.add(tileGroup);

    m_minefieldOverlay.attach(oop);

    m_keymapWidget.setKeymapName(m_definition.keymapName);

    ui::Group mapGroup(ui::layout::VBox::instance5);
    mapGroup.add(m_mapWidget);
    mapGroup.add(m_scanResult);

    ui::PrefixArgument prefix(root);
    ui::widgets::KeyForwarder forwarder(*this);

    m_panel.add(forwarder);
    m_panel.add(prefix);
    m_panel.add(mapGroup);
    m_panel.setExtent(root.getExtent());
    m_panel.setState(ui::Widget::ModalState, true);
    root.add(m_panel);

    if (m_historyAdaptor.get() != 0) {
        m_historyAdaptor->sig_turnChange.add(this, &ControlScreen::onHistoryTurnChange);
    } else {
        oop.addNewListener(new Updater(m_reply.getSender(), m_fleetProxy.get() != 0));
    }

    m_mapWidget.addOverlay(m_scannerOverlay);
    m_mapWidget.addOverlay(m_movementOverlay);
    m_mapWidget.addOverlay(m_minefieldOverlay);

    // FIXME: only for ship/fleet
    {
        client::map::WaypointOverlay& wo = m_deleter.addNew(new client::map::WaypointOverlay(root, m_fleetProxy.get() != 0));
        m_mapWidget.addOverlay(wo);
        wo.attach(oop);
    }

    if (m_taskEditorProxy.get() != 0 && m_taskKind == interpreter::Process::pkShipTask) {
        client::map::ShipTaskOverlay& ov = m_deleter.addNew(new client::map::ShipTaskOverlay(root));
        m_mapWidget.addOverlay(ov);
        m_taskEditorProxy->sig_shipChange.add(&ov, &client::map::ShipTaskOverlay::setStatus);
        m_taskEditorProxy->sig_shipChange.add(this, &ControlScreen::onTaskEditorShipChange);
    }
    if (m_fleetProxy.get() != 0) {
        m_fleetProxy->sig_change.add(this, &ControlScreen::onFleetChange);
    }

    m_movementOverlay.sig_move.add(this, &ControlScreen::onScannerMove);
    m_movementOverlay.sig_doubleClick.add(this, &ControlScreen::onDoubleClick);

    continueProcessWait(in.getProcess());
    m_loop.run();

    out = m_outputState;
}

void
client::screens::ControlScreen::handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
{
    // ex WControlScreen::processEvent
    using client::si::OutputState;
    switch (target) {
     case OutputState::NoChange:
        interface().continueProcess(link);
        break;

     case OutputState::ShipScreen:
     case OutputState::PlanetScreen:
     case OutputState::BaseScreen:
     case OutputState::HistoryScreen:
     case OutputState::FleetScreen:
     case OutputState::ShipTaskScreen:
     case OutputState::PlanetTaskScreen:
     case OutputState::BaseTaskScreen:
        if (target == m_state->ownTarget) {
            interface().continueProcess(link);
        } else {
            interface().detachProcess(link);
            m_outputState.set(link, target);
            m_loop.stop(0);
        }
        break;

     case OutputState::ExitProgram:
     case OutputState::ExitGame:
     case OutputState::PlayerScreen:
     case OutputState::Starchart:
        interface().detachProcess(link);
        m_outputState.set(link, target);
        m_loop.stop(0);
        break;
    }
}

void
client::screens::ControlScreen::handlePopupConsole(client::si::RequestLink2 link)
{
    defaultHandlePopupConsole(link);
}

void
client::screens::ControlScreen::handleScanKeyboardMode(client::si::RequestLink2 link)
{
    m_movementOverlay.doKeyboardMode(m_mapWidget.renderer());
    interface().continueProcess(link);
}

void
client::screens::ControlScreen::handleEndDialog(client::si::RequestLink2 link, int /*code*/)
{
    // This is not a dialog.
    interface().continueProcess(link);
}

void
client::screens::ControlScreen::handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetView(link, name, withKeymap);
}

void
client::screens::ControlScreen::handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
{
    defaultHandleUseKeymap(link, name, prefix);
}

void
client::screens::ControlScreen::handleOverlayMessage(client::si::RequestLink2 link, String_t text)
{
    defaultHandleOverlayMessage(link, text);
}

game::interface::ContextProvider*
client::screens::ControlScreen::createContextProvider()
{
    return new ContextProvider(m_state);
}

bool
client::screens::ControlScreen::handleKey(util::Key_t key, int prefix)
{
    // Tile buttons will generate key events to be handled by the user keymap, but also to be handled by us.
    // We need a single target for key events, so we route them from here to m_keymapWidget manually.

    // Handle user keys
    if (m_keymapWidget.handleKey(key, prefix)) {
        return true;
    }

    // Tabbing for history
    // ex WHistoryScreen::handleEvent
    if (m_historyAdaptor.get() != 0) {
        if ((key & ~(util::KeyMod_Shift | util::KeyMod_Ctrl)) == util::Key_Tab) {
            if (const game::map::ShipLocationInfo* p = m_historyAdaptor->getCurrentTurnInformation()) {
                if (const game::map::Point* pt = p->position.get()) {
                    game::proxy::HistoryShipProxy::Mode mode =
                        (key & util::KeyMod_Shift) != 0
                        ? game::proxy::HistoryShipProxy::Previous
                        : game::proxy::HistoryShipProxy::Next;
                    bool marked = (key & util::KeyMod_Ctrl) != 0;
                    m_historyAdaptor->proxy().browseAt(*pt, mode, marked);
                }
            }
        }
        return true;
    }

    return false;
}

void
client::screens::ControlScreen::setId(game::Id_t id)
{
    // ex WControlScreen::onCurrentChanged (sort-of)
    m_id = id;
    interface().history().push(ScreenHistory::Reference(m_definition.historyType, id, 0));
    if (m_taskEditorProxy.get() != 0) {
        m_taskEditorProxy->selectTask(id, m_taskKind, true);
    }
}

void
client::screens::ControlScreen::setPositions(game::map::Point origin, game::map::Point target, bool isHyperdriving)
{
    m_center = origin;
    m_mapWidget.setCenter(origin);
    setIsHyperdriving(isHyperdriving);
    setTarget(target);
}

void
client::screens::ControlScreen::setTarget(game::map::Point target)
{
    class SetProperties : public util::Request<Proprietor> {
     public:
        SetProperties(Point pt)
            : m_point(pt)
            { }
        virtual void handle(Proprietor& prop)
            { prop.setScannerPosition(m_point); }
     private:
        Point m_point;
    };

    m_scanResult.setPositions(m_center, target);
    m_scannerOverlay.setPositions(m_center, target);
    m_movementOverlay.setPosition(target);
    m_proprietor.postNewRequest(new SetProperties(target));
}

void
client::screens::ControlScreen::setIsHyperdriving(bool isHyperdriving)
{
    m_movementOverlay.setLockOrigin(m_center, isHyperdriving, getShipId());
}

void
client::screens::ControlScreen::clearPositions()
{
    // FIXME: invalidate mapWidget - how?
    m_scanResult.clearPositions();
    m_scannerOverlay.clearPositions();
    m_movementOverlay.clearPosition();
}

void
client::screens::ControlScreen::onScannerMove(game::map::Point target)
{
    setTarget(target);
}

void
client::screens::ControlScreen::onDoubleClick(game::map::Point /*target*/)
{
    // ex WControlScreen::onChartDblclick
    /* Check with current modifiers. If none found, check again
       without shift, then without all modifiers.

       For regular events, we automatically discount shift
       (xref ui/window.cc:simplifyEvent). */
    util::Key_t mods = root().engine().getKeyboardModifierState();
    if (!m_panel.handleKey(util::Key_DoubleClick | mods, 0)) {
        if (!m_panel.handleKey(util::Key_DoubleClick | (mods & ~util::KeyMod_Shift), 0)) {
            m_panel.handleKey(util::Key_DoubleClick, 0);
        }
    }
}

void
client::screens::ControlScreen::onTaskEditorShipChange(const game::proxy::TaskEditorProxy::ShipStatus& st)
{
    game::map::Point finalPos = st.positions.empty() ? st.startPosition : st.positions.back();
    bool isHyperdriving = st.isHyperdriving;

    m_movementOverlay.setLockOrigin(finalPos, isHyperdriving, getShipId());
}

void
client::screens::ControlScreen::onFleetChange()
{
    if (m_fleetProxy.get() != 0) {
        if (m_fleetProxy->getSelectedFleetMember() == 0 && m_fleetProxy->getFleetMemberList().empty()) {
            m_outputState.set(client::si::RequestLink2(), client::si::OutputState::PlayerScreen);
            m_loop.stop(0);
        }
    }
}

void
client::screens::ControlScreen::onHistoryTurnChange()
{
    if (m_historyAdaptor.get() != 0) {
        if (const game::map::ShipLocationInfo* p = m_historyAdaptor->getCurrentTurnInformation()) {
            if (const game::map::Point* pos = p->position.get()) {
                setPositions(*pos, *pos, false);
            } else {
                clearPositions();
            }
        }
        m_mapWidget.setShipTrailId(m_historyAdaptor->getShipId());
    }
}

game::Id_t
client::screens::ControlScreen::getShipId() const
{
    return m_definition.isShip ? m_id : 0;
}
