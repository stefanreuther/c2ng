/**
  *  \file client/map/waypointoverlay.cpp
  *  \brief Class client::map::WaypointOverlay
  */

#include "client/map/waypointoverlay.hpp"
#include "client/map/callback.hpp"
#include "client/map/renderer.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"

using game::map::ShipMovementInfo;

client::map::WaypointOverlay::WaypointOverlay(ui::Root& root)
    : m_root(root),
      m_reply(root.engine().dispatcher(), *this),
      m_infos()
{ }

client::map::WaypointOverlay::~WaypointOverlay()
{ }

void
client::map::WaypointOverlay::setData(const game::map::ShipMovementInfos_t& infos)
{
    if (infos != m_infos) {
        m_infos = infos;
        requestRedraw();
    }
}

void
client::map::WaypointOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

void
client::map::WaypointOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WShipScannerChartWidget::drawPost (part), WShipScannerChartWidget::drawScanner, ship.pas:DrawTowingInfo, ship.pas:DrawChunnelInfo
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());

    for (size_t i = 0, n = m_infos.size(); i < n; ++i) {
        const ShipMovementInfo& e = m_infos[i];
        switch (e.action) {
         case ShipMovementInfo::Movement:
            ctx.setColor(ui::Color_Red);
            ctx.setLinePattern(gfx::SOLID_LINE);
            drawLine(ctx, ren.scale(e.from), ren.scale(e.to));
            break;

         case ShipMovementInfo::Chunnel:
            switch (e.status) {
             case ShipMovementInfo::Success:
                ctx.setColor(ui::Color_BlueGray);
                break;
             case ShipMovementInfo::InitiatorFails:
                ctx.setColor(ui::Color_Red);
                break;
             case ShipMovementInfo::MateFails:
                ctx.setColor(ui::Color_Yellow);
                break;
            }
            ctx.setLinePattern(0xF0);
            drawLine(ctx, ren.scale(e.from), ren.scale(e.to));
            break;

         case ShipMovementInfo::Tow:
            ctx.setColor(ui::Color_Red);
            ctx.setLinePattern(0x55);
            drawLine(ctx, ren.scale(e.from), ren.scale(e.to));
            break;
        }
    }
}

bool
client::map::WaypointOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::WaypointOverlay::handleKey(util::Key_t /*key*/, int /*prefix*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::WaypointOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    return false;
}

void
client::map::WaypointOverlay::attach(game::proxy::ObjectObserver& oop)
{
    class Reply : public util::Request<WaypointOverlay> {
     public:
        Reply(game::Session& s, game::map::Object* obj)
            : m_infos()
            {
                const game::map::Ship* pShip = dynamic_cast<const game::map::Ship*>(obj);
                const game::Root* pRoot = s.getRoot().get();
                const game::Game* pGame = s.getGame().get();
                const game::spec::ShipList* pShipList = s.getShipList().get();
                if (pShip != 0 && pRoot != 0 && pGame != 0 && pShipList != 0) {
                    // FIXME: correct universe?
                    const game::map::Universe& univ = pGame->currentTurn().universe();
                    packShipMovementInfo(m_infos, *pShip, univ, pGame->shipScores(), *pShipList, *pRoot);
                }
            }

        void handle(WaypointOverlay& t)
            { t.setData(m_infos); }

     private:
        game::map::ShipMovementInfos_t m_infos;
    };

    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(util::RequestSender<WaypointOverlay> reply)
            : m_reply(reply)
            { }
        virtual void handle(game::Session& s, game::map::Object* obj)
            { m_reply.postNewRequest(new Reply(s, obj)); }
     private:
        util::RequestSender<WaypointOverlay> m_reply;
    };
    oop.addNewListener(new Listener(m_reply.getSender()));
}
