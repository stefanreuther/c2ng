/**
  *  \file client/tiles/visualscanhullinfotile.cpp
  */

#include "client/tiles/visualscanhullinfotile.hpp"
#include "afl/string/format.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/map/ship.hpp"
#include "game/root.hpp"
#include "gfx/context.hpp"
#include "util/skincolor.hpp"
#include "util/unicodechars.hpp"

using afl::string::Format;
using game::map::Ship;
using game::map::Object;
using client::tiles::VisualScanHullInfoTile;

namespace {
    void prepareContent(game::Session& session, Object* obj, VisualScanHullInfoTile::Content& result)
    {
        // ex WVisualScanHullInfoTile::draw
        // Fetch preconditions
        const Ship* pShip = dynamic_cast<Ship*>(obj);
        if (pShip == 0) {
            return;
        }

        const game::spec::ShipList* pShipList = session.getShipList().get();
        if (pShipList == 0) {
            return;
        }

        const game::spec::Hull* pHull = pShipList->hulls().get(pShip->getHull().orElse(0));
        if (pHull == 0) {
            return;
        }

        const game::Root* pRoot = session.getRoot().get();
        if (pRoot == 0) {
            return;
        }
        const game::config::UserConfiguration& pref = pRoot->userConfiguration();

        afl::string::Translator& tx = session.translator();

        // Line 1: "Hull mass: nn kt"
        result.text[VisualScanHullInfoTile::HullMass] = Format(tx.translateString("Hull mass: %d kt").c_str(), pref.formatNumber(pHull->getMass()));

        // Line 2:
        //   Cargo: a/b kt
        //   Max cargo: x kt
        if (pShip->getShipKind() == Ship::CurrentShip) {
            result.text[VisualScanHullInfoTile::Cargo] = Format(tx.translateString("Cargo: %d/%d kt").c_str(),
                                                                pref.formatNumber(pHull->getMaxCargo() - pShip->getFreeCargo(*pShipList).orElse(0)),
                                                                pref.formatNumber(pHull->getMaxCargo()));
        } else {
            result.text[VisualScanHullInfoTile::Cargo] = Format(tx.translateString("Max Cargo: %d kt").c_str(), pref.formatNumber(pHull->getMaxCargo()));
        }

        // Line 3:
        //   Fuel: a/b kt
        //   Max fuel: x kt
        if (pShip->getShipKind() == Ship::CurrentShip) {
            result.text[VisualScanHullInfoTile::Fuel] = Format(tx.translateString("Fuel: %d/%d kt").c_str(),
                                                               pref.formatNumber(pShip->getCargo(game::Element::Neutronium).orElse(0)),
                                                               pref.formatNumber(pHull->getMaxFuel()));
        } else {
            result.text[VisualScanHullInfoTile::Fuel] = Format(tx.translateString("Max Fuel: %d kt").c_str(),
                                                               pref.formatNumber(pHull->getMaxFuel()));
        }

        // Line 4:
        //   3xBeamTyp
        //   Max Beams: x
        //   No beams
        // FIXME: handle history
        if (pShip->getShipKind() == Ship::CurrentShip) {
            int numBeams = pShip->getNumBeams().orElse(0);
            const game::spec::Component* pBeam = pShipList->beams().get(pShip->getBeamType().orElse(0));
            if (numBeams != 0 && pBeam != 0) {
                result.text[VisualScanHullInfoTile::Beams] = Format("%d" UTF_TIMES "%s", numBeams, pBeam->getName(pShipList->componentNamer()));
            } else {
                result.text[VisualScanHullInfoTile::Beams] = tx.translateString("No beams");
            }
        } else {
            if (pHull->getMaxBeams() != 0) {
                result.text[VisualScanHullInfoTile::Beams] = Format(tx.translateString("Max Beams: %d").c_str(), pHull->getMaxBeams());
            } else {
                result.text[VisualScanHullInfoTile::Beams] = tx.translateString("No beams");
            }
        }

        // Line 5:
        //   2xTorpTyp
        //   Max torps: x
        //   No torps
        //   Fighter bays: x
        //   No fighter bays
        if (pShip->getShipKind() == Ship::CurrentShip) {
            const int numLaunchers = pShip->getNumLaunchers().orElse(0);
            const int numBays = pShip->getNumBays().orElse(0);
            const game::spec::Component*const pLauncher = pShipList->launchers().get(pShip->getTorpedoType().orElse(0));
            if (numLaunchers != 0 && pLauncher != 0) {
                result.text[VisualScanHullInfoTile::Secondary] = Format("%d" UTF_TIMES "%s", numLaunchers, pLauncher->getName(pShipList->componentNamer()));
            } else if (numBays > 0) {
                result.text[VisualScanHullInfoTile::Secondary] = Format(tx.translateString("Fighter bays: %d").c_str(), numBays);
            } else if (pHull->getNumBays() != 0) {
                result.text[VisualScanHullInfoTile::Secondary] = tx.translateString("No fighter bays");
            } else if (pHull->getMaxLaunchers() != 0) {
                result.text[VisualScanHullInfoTile::Secondary] = tx.translateString("No torps");
            } else {
                // leave empty
            }
        } else {
            if (pHull->getNumBays() != 0) {
                result.text[VisualScanHullInfoTile::Secondary] = Format(tx.translateString("Fighter bays: %d").c_str(), pHull->getNumBays());
            } else if (pHull->getMaxLaunchers() != 0) {
                result.text[VisualScanHullInfoTile::Secondary] = Format(tx.translateString("Max torps: %d").c_str(), pHull->getMaxLaunchers());
            } else {
                // leave empty
            }
        }

        // Line 6:
        //   FCode: foo
        if (pShip->getShipKind() == Ship::CurrentShip) {
            result.text[VisualScanHullInfoTile::FriendlyCode] = Format(tx.translateString("FCode: %s").c_str(), pShip->getFriendlyCode().orElse(String_t()));
        }
    }
}

client::tiles::VisualScanHullInfoTile::VisualScanHullInfoTile(ui::Root& root)
    : m_root(root),
      m_content(),
      m_reply(root.engine().dispatcher(), *this)
{ }

client::tiles::VisualScanHullInfoTile::~VisualScanHullInfoTile()
{ }

void
client::tiles::VisualScanHullInfoTile::draw(gfx::Canvas& can)
{
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
    ctx.setSolidBackground();
    ctx.setColor(util::SkinColor::Static);

    gfx::Rectangle area = getExtent();
    const int lineHeight = ctx.getFont()->getCellSize().getY();
    for (int i = 0; i < NUM_LINES; ++i) {
        outTextF(ctx, area.splitY(lineHeight), m_content.text[i]);
    }
}

void
client::tiles::VisualScanHullInfoTile::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::tiles::VisualScanHullInfoTile::handlePositionChange()
{ }

ui::layout::Info
client::tiles::VisualScanHullInfoTile::getLayoutInfo() const
{
    // ex WVisualScanHullInfoTile::WVisualScanHullInfoTile
    return m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(10, NUM_LINES);
}

bool
client::tiles::VisualScanHullInfoTile::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::tiles::VisualScanHullInfoTile::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::tiles::VisualScanHullInfoTile::setContent(const Content& content)
{
    m_content = content;
    requestRedraw();
}

void
client::tiles::VisualScanHullInfoTile::attach(game::proxy::ObjectObserver& oop)
{
    class Updater : public util::Request<VisualScanHullInfoTile> {
     public:
        Updater(const Content& content)
            : m_content(content)
            { }
        virtual void handle(VisualScanHullInfoTile& tile)
            { tile.setContent(m_content); }
     private:
        Content m_content;
    };

    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(util::RequestSender<VisualScanHullInfoTile> reply)
            : m_reply(reply)
            { }
        virtual void handle(game::Session& session, game::map::Object* obj)
            {
                Content result;
                prepareContent(session, obj, result);
                m_reply.postNewRequest(new Updater(result));
            }
     private:
        util::RequestSender<VisualScanHullInfoTile> m_reply;
    };

    oop.addNewListener(new Listener(m_reply.getSender()));
}
