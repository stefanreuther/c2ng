/**
  *  \file client/tiles/visualscanshipinfotile.cpp
  */

#include "client/tiles/visualscanshipinfotile.hpp"
#include "afl/string/format.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "gfx/context.hpp"
#include "util/skincolor.hpp"

using afl::string::Format;
using game::map::Ship;
using game::map::Object;
using client::tiles::VisualScanShipInfoTile;

namespace {
    void prepareContent(game::Session& session, Object* obj, VisualScanShipInfoTile::Content& result)
    {
        // ex WVisualScanShipInfoTile::drawData
        const Ship* pShip = dynamic_cast<Ship*>(obj);
        if (pShip == 0) {
            return;
        }

        const game::spec::ShipList* pShipList = session.getShipList().get();
        if (pShipList == 0) {
            return;
        }

        const game::Root* pRoot = session.getRoot().get();
        if (pRoot == 0) {
            return;
        }

        const game::config::UserConfiguration& pref = pRoot->userConfiguration();
        afl::string::Translator& tx = session.translator();

        // Line 1:
        //   Mass: x kt        Speed: warp X
        //   Mass: unknown     Not moving
        int mass;
        if (pShip->getMass(*pShipList).get(mass)) {
            result.text[VisualScanShipInfoTile::ShipMass] = Format(tx("Mass: %d kt"), pref.formatNumber(mass));
        } else {
            result.text[VisualScanShipInfoTile::ShipMass] = tx("Mass: unknown");
        }

        // Line 1a:
        int warp;
        if (pShip->getWarpFactor().get(warp)) {
            if (warp == 0) {
                result.text[VisualScanShipInfoTile::Speed] = tx("Not moving");
            } else {
                result.text[VisualScanShipInfoTile::Speed] = Format(tx("Speed: warp %d"), warp);
            }
        } else {
            // Leave empty
        }

        // Line 2:
        //   Waypoint: foo
        if (pShip->getShipKind() == Ship::CurrentShip) {
            game::map::Point pt;
            if (pShip->getWaypoint().get(pt)) {
                if (game::Game* pGame = session.getGame().get()) {
                    result.text[VisualScanShipInfoTile::Waypoint] =
                        Format(tx("Waypoint: %s"), pGame->viewpointTurn().universe().findLocationName(pt, game::map::Universe::NameGravity, pGame->mapConfiguration(), pRoot->hostConfiguration(), pRoot->hostVersion(), tx));
                }
            }
        }

        // Line 3:
        //   Next turn: foo
        // FIXME: port this
        // GPoint pt = s->getPredictedPos();
        // if (s->getShipKind() == GShip::CurrentShip && (pt.x != 0 || pt.y != 0)) {
        //     if (pt == s->getWaypoint()) {
        //         text = tx("Next turn: not moved");
        //     } else {
        //         text = format(tx("Next turn: %s"),
        //                       getCurrentUniverse()->getLocationName(pt, locs_WW));
        //     }
        // }

        // Line 4:
        //   Damage: %d%%
        int damage;
        if (pShip->getDamage().get(damage)) {
            result.text[VisualScanShipInfoTile::Damage] = Format(tx("Damage: %d%%").c_str(), damage);
        }
    }
}

client::tiles::VisualScanShipInfoTile::VisualScanShipInfoTile(ui::Root& root)
    : m_root(root),
      m_content(),
      m_reply(root.engine().dispatcher(), *this)
{ }

client::tiles::VisualScanShipInfoTile::~VisualScanShipInfoTile()
{ }

void
client::tiles::VisualScanShipInfoTile::draw(gfx::Canvas& can)
{
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
    ctx.setSolidBackground();
    ctx.setColor(util::SkinColor::Static);

    gfx::Rectangle area = getExtent();
    const int lineHeight = ctx.getFont()->getCellSize().getY();

    // First line is two-in-one
    gfx::Rectangle firstLine = area.splitY(lineHeight);
    outTextF(ctx, firstLine.splitX(firstLine.getWidth()/2), m_content.text[ShipMass]);
    outTextF(ctx, firstLine,                                m_content.text[Speed]);

    // Remaining lines
    for (int i = 1; i < NUM_LINES; ++i) {
        outTextF(ctx, area.splitY(lineHeight), m_content.text[i]);
    }
}

void
client::tiles::VisualScanShipInfoTile::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::tiles::VisualScanShipInfoTile::handlePositionChange()
{ }

ui::layout::Info
client::tiles::VisualScanShipInfoTile::getLayoutInfo() const
{
    // ex WVisualScanShipInfoTile::WVisualScanShipInfoTile
    return m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(20, NUM_LINES);
}

bool
client::tiles::VisualScanShipInfoTile::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::tiles::VisualScanShipInfoTile::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::tiles::VisualScanShipInfoTile::setContent(const Content& content)
{
    m_content = content;
    requestRedraw();
}

void
client::tiles::VisualScanShipInfoTile::attach(game::proxy::ObjectObserver& oop)
{
    class Updater : public util::Request<VisualScanShipInfoTile> {
     public:
        Updater(const Content& content)
            : m_content(content)
            { }
        virtual void handle(VisualScanShipInfoTile& tile)
            { tile.setContent(m_content); }
     private:
        Content m_content;
    };

    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(util::RequestSender<VisualScanShipInfoTile> reply)
            : m_reply(reply)
            { }
        virtual void handle(game::Session& session, game::map::Object* obj)
            {
                Content result;
                prepareContent(session, obj, result);
                m_reply.postNewRequest(new Updater(result));
            }
     private:
        util::RequestSender<VisualScanShipInfoTile> m_reply;
    };

    oop.addNewListener(new Listener(m_reply.getSender()));
}
