/**
  *  \file client/map/distanceoverlay.cpp
  *  \brief Class client::map::DistanceOverlay
  */

#include "client/map/distanceoverlay.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/helpdialog.hpp"
#include "client/map/location.hpp"
#include "client/map/screen.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/fleetmember.hpp"
#include "game/map/shippredictor.hpp"
#include "game/turn.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "util/math.hpp"

using afl::string::Format;

namespace {
    /* Given distance and speed, compute estimated time. */
    int estimateTime(double d, int warp)
    {
        int32_t r = util::roundToInt(d);
        int32_t way = warp*warp;

        return (r + way-1) / way;
    }
}


client::map::DistanceOverlay::DistanceOverlay(Screen& parent, Location& loc, game::map::Point origin, game::Id_t shipId)
    : Overlay(),
      m_parent(parent),
      m_location(loc),
      m_origin(origin),
      m_shipId(shipId),
      m_status(),
      m_first(true),
      m_state(Requesting),
      m_reply(parent.root().engine().dispatcher(), *this),
      conn_positionChange(loc.sig_positionChange.add(this, &DistanceOverlay::onPositionChange))
{
    requestStatus();
}

client::map::DistanceOverlay::~DistanceOverlay()
{ }

void
client::map::DistanceOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

void
client::map::DistanceOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WDistanceChartMode::drawOverlays
    gfx::Context<uint8_t> ctx(can, m_parent.root().colorScheme());
    ctx.useFont(*m_parent.root().provider().getFont(gfx::FontRequest()));
    ctx.setColor(ui::Color_White);

    const int line = ctx.getFont()->getLineHeight();
    const int indent = ctx.getFont()->getEmWidth();

    gfx::Point pos = ren.getExtent().getTopLeft();

    // Distance
    outText(ctx, pos, m_status.distanceInfo);
    pos.addY(line);

    // Speeds, angle
    outText(ctx, pos, m_status.flightInfo);
    pos.addY(line);

    // Ship info
    if (m_status.mode != DistanceMode && !m_status.shipName.empty()) {
        outText(ctx, pos, m_status.shipName + ":");
        pos.addY(line);
        pos.addX(indent);

        ctx.setColor(m_status.shipColor);
        outText(ctx, pos, m_status.shipInfo);
        pos.addY(line);

        ctx.setColor(m_status.fuelColor);
        outText(ctx, pos, m_status.fuelInfo);
        pos.addY(line);

        if (m_status.mode == WaypointMode) {
            ctx.setColor(ui::Color_White);
            outText(ctx, pos, m_parent.translator()("Press [W] to change this ship's waypoint."));
        }
    }
}

bool
client::map::DistanceOverlay::drawCursor(gfx::Canvas& can, const Renderer& ren)
{
    // ex WDistanceChartMode::drawCursor
    gfx::Context<uint8_t> ctx(can, m_parent.root().colorScheme());
    ctx.setColor(ui::Color_White);

    drawLine(ctx, ren.scale(m_origin), ren.scale(m_location.getPosition()));
    return false;
}

bool
client::map::DistanceOverlay::handleKey(util::Key_t key, int prefix, const Renderer& /*ren*/)
{
    // ex WDistanceChartMode::handleEvent
    switch (key) {
     case 'x':
        // Swap ends
        swapEnds();
        return true;

     case 'f':
        // Use ship for fuel consumption
        if (m_location.getFocusedObject().getType() == game::Reference::Ship) {
            int newId = m_location.getFocusedObject().getId();
            if (newId != m_shipId) {
                m_shipId = newId;
                maybeRequestStatus();
            }
        }
        return true;

     case 'w':
     case util::KeyMod_Ctrl + 'w':
        // Set ship waypoint
        if (m_status.mode == WaypointMode) {
            setWaypoint();
        }
        return true;

     case util::Key_Backspace:
        // Back to beginning and exit (which means back to the object we came from)
        m_location.setPosition(m_origin);
        if (m_status.mode == WaypointMode || m_status.mode == ForeignMode) {
            m_location.setFocusedObject(game::Reference(game::Reference::Ship, m_shipId));
        }
        m_parent.removeOverlay(this);
        return true;

     case util::Key_Escape:
        // When coming from an object, go back there; otherwise, stay where we are
        if (m_status.mode == WaypointMode || m_status.mode == ForeignMode) {
            m_location.setPosition(m_origin);
            m_location.setFocusedObject(game::Reference(game::Reference::Ship, m_shipId));
        }
        m_parent.removeOverlay(this);
        return true;

     case 'd':
     case util::Key_Delete:
        m_parent.removeOverlay(this);
        return true;

     case util::Key_Quit:
        m_parent.root().postKeyEvent(key, prefix);
        m_parent.removeOverlay(this);
        return true;

     case 'h':
     case util::KeyMod_Alt + 'h':
        client::dialogs::doHelpDialog(m_parent.root(), m_parent.translator(), m_parent.gameSender(), "pcc2:distance");
        return true;

     default:
        return false;
    }
}

bool
client::map::DistanceOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    return false;
}

void
client::map::DistanceOverlay::onPositionChange(game::map::Point /*pt*/)
{
    // ex WDistanceChartMode::onMove
    // FIXME: handle map seam crossing
    //     origin += loc.getWrapAdjust();
    //     WSimpleChartMode::onMove(loc);
    maybeRequestStatus();
}

void
client::map::DistanceOverlay::maybeRequestStatus()
{
    if (m_state == Idle) {
        m_state = Requesting;
        requestStatus();
    } else {
        m_state = Retriggered;
    }
}

void
client::map::DistanceOverlay::setStatus(Status st)
{
    m_status = st;
    if (m_first) {
        // When invoked from a ship, go to its waypoint
        m_first = false;
        if (st.mode == WaypointMode && st.shipWaypoint != m_location.getPosition()) {
            m_location.setPosition(st.shipWaypoint);
        }
    }

    if (m_state == Requesting) {
        m_state = Idle;
    } else {
        m_state = Requesting;
        requestStatus();
    }

    requestRedraw();
}

void
client::map::DistanceOverlay::requestStatus()
{
    class Task : public util::Request<game::Session> {
     public:
        Task(util::RequestSender<DistanceOverlay> reply, game::map::Point origin, game::map::Point target, game::Id_t shipId)
            : m_reply(reply), m_origin(origin), m_target(target), m_shipId(shipId)
            { }
        virtual void handle(game::Session& session)
            {
                Status st;
                buildStatus(st, session, m_origin, m_target, m_shipId);
                m_reply.postRequest(&DistanceOverlay::setStatus, st);
            }
     private:
        util::RequestSender<DistanceOverlay> m_reply;
        game::map::Point m_origin;
        game::map::Point m_target;
        game::Id_t m_shipId;
    };
    m_parent.gameSender().postNewRequest(new Task(m_reply.getSender(), m_origin, m_location.getPosition(), m_shipId));
}

inline void
client::map::DistanceOverlay::swapEnds()
{
    game::map::Point p = m_origin;
    m_origin = m_location.getPosition();
    m_location.setPosition(p);
}

void
client::map::DistanceOverlay::setWaypoint()
{
    class Task : public util::Request<game::Session> {
     public:
        Task(game::Id_t shipId, game::map::Point waypoint)
            : m_shipId(shipId), m_waypoint(waypoint)
            { }

        virtual void handle(game::Session& session)
            {
                game::Game& g = game::actions::mustHaveGame(session);
                game::Root& r = game::actions::mustHaveRoot(session);
                game::spec::ShipList& sl = game::actions::mustHaveShipList(session);
                if (game::Turn* t = g.getViewpointTurn().get()) {
                    game::map::Ship& sh = game::actions::mustExist(t->universe().ships().get(m_shipId));
                    game::actions::mustBePlayed(sh);

                    // FIXME: shouldn't call this if FleetMember will refuse (bug also in PCC2)
                    game::map::FleetMember(t->universe(), sh, g.mapConfiguration()).setWaypoint(m_waypoint, r.hostConfiguration(), sl);
                    session.notifyListeners();
                }
            }

     private:
        game::Id_t m_shipId;
        game::map::Point m_waypoint;
    };
    m_parent.gameSender().postNewRequest(new Task(m_shipId, m_location.getPosition()));
}

void
client::map::DistanceOverlay::buildStatus(Status& out, game::Session& session, game::map::Point origin, game::map::Point target, game::Id_t shipId)
{
    // Obtain references
    if (session.getGame().get() == 0 || session.getRoot().get() == 0 || session.getShipList().get() == 0) {
        return;
    }
    const game::Game& g = *session.getGame();
    const game::Root& r = *session.getRoot();
    const game::spec::ShipList& sl = *session.getShipList();
    if (g.getViewpointTurn().get() == 0) {
        return;
    }
    const game::Turn& t = *g.getViewpointTurn();
    afl::string::Translator& tx = session.translator();
    util::NumberFormatter fmt = r.userConfiguration().getNumberFormatter();

    // Distance
    int dx = target.getX() - origin.getX();
    int dy = target.getY() - origin.getY();
    double dist = util::getDistanceFromDX(dx, dy);
    out.distanceInfo = Format(tx("Distance from first point: %.1f ly"), dist);

    // Flight info
    out.flightInfo = Format(tx("Warp/Time: 6/%d 7/%d 8/%d 9/%d"), estimateTime(dist, 6), estimateTime(dist, 7), estimateTime(dist, 8), estimateTime(dist, 9));
    if (dist > 0) {
        out.flightInfo += Format(", %d" "\xC2\xB0", util::roundToInt(util::getHeadingDeg(dx, dy)));
    }

    // Mode
    // ex WDistanceChartMode::getMode
    const game::map::Ship* sh = t.universe().ships().get(shipId);
    game::map::Point shipPos;
    if (sh == 0 || !sh->isVisible() || !sh->getPosition().get(shipPos)) {
        // Ship doesn't exist or isn't visible
        out.mode = DistanceMode;
    } else {
        if (shipPos == origin) {
            if (sh->isPlayable(game::map::Object::Playable)) {
                out.mode = WaypointMode;
                out.shipWaypoint = sh->getWaypoint().orElse(shipPos);
            } else {
                out.mode = ForeignMode;
            }
        } else {
            out.mode = OtherMode;
        }
    }

    // Prediction for playable ships
    if (sh != 0 && sh->isPlayable(game::map::Object::Playable)) {
        // Name
        out.shipName = sh->getName(game::LongName, tx, session.interface());

        // Prediction
        game::map::ShipPredictor pred(t.universe(), shipId, g.shipScores(), sl, g.mapConfiguration(), r.hostConfiguration(), r.hostVersion(), r.registrationKey());
        pred.setPosition(origin);
        pred.setWaypoint(target);
        pred.addTowee();
        pred.computeMovement();

        // Speed/time
        int speed = sh->getWarpFactor().orElse(0);
        if (speed == 0) {
            out.shipInfo = tx("not moving");
            out.shipColor = ui::Color_Red;
        } else if (pred.isAtTurnLimit()) {
            out.shipInfo = tx("too long");
            out.shipColor = ui::Color_Yellow;
        } else {
            out.shipInfo = Format(tx("%d turn%!1{s%} at warp %d"), pred.getNumTurns(), speed);
            out.shipColor = ui::Color_White;
        }

        // Fuel usage
        int32_t availableFuel = sh->getCargo(game::Element::Neutronium).orElse(0);
        if (speed == 0) {
            out.fuelInfo = Format(tx("%d kt fuel aboard"), fmt.formatNumber(availableFuel));
            out.fuelColor = ui::Color_White;
        } else {
            int32_t fuelUsed = pred.getMovementFuelUsed(); // FIXME: deal with other usage categories?
            out.fuelInfo = Format(tx("%d of %d kt fuel used"), fmt.formatNumber(fuelUsed), fmt.formatNumber(availableFuel));
            out.fuelColor = fuelUsed > availableFuel ? ui::Color_Red : ui::Color_White;
        }
    }
}
