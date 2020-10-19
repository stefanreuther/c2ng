/**
  *  \file client/map/minefieldmissionoverlay.cpp
  */

#include "client/map/minefieldmissionoverlay.hpp"
#include "afl/string/format.hpp"
#include "client/map/callback.hpp"
#include "client/map/renderer.hpp"
#include "game/game.hpp"
#include "game/map/minefield.hpp"
#include "game/map/minefieldmission.hpp"
#include "game/map/ship.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "util/translation.hpp"

using afl::string::Format;

client::map::MinefieldMissionOverlay::MinefieldMissionOverlay(ui::Root& root)
    : Overlay(),
      m_data(),
      m_root(root),
      m_reply(root.engine().dispatcher(), *this)
{ }

client::map::MinefieldMissionOverlay::~MinefieldMissionOverlay()
{ }

void
client::map::MinefieldMissionOverlay::setEffects(game::map::MinefieldEffects_t data)
{
    bool oldEmpty = m_data.empty();
    m_data = data;

    // Minor optimisation: do not redraw if old and new are empty
    if (!oldEmpty || !m_data.empty()) {
        requestRedraw();
    }
}

void
client::map::MinefieldMissionOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

void
client::map::MinefieldMissionOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WShipScannerChartWidget::drawPost
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addSize(-1)));

    for (size_t i = 0, n = m_data.size(); i < n; ++i) {
        const game::map::MinefieldEffect& eff = m_data[i];
        int32_t radius = game::map::Minefield::getRadiusFromUnits(eff.newUnits);
        String_t label;
        if (eff.newUnits == 0) {
            // Scoop it, gone
            label = Format(_("gone (%d torp%!1{s%})"), eff.numTorps);
        } else if (eff.id == 0 || eff.radiusChange == 0) {
            // Lay, new minefield (otherwise, Id would be known)
            // -or- Action does not change size
            label = Format(_("%d ly"), radius);
        } else if (eff.radiusChange > 0) {
            // Lay
            label = Format(_("%d ly (+%d)"), radius, eff.radiusChange);
        } else {
            // Scoop
            label = Format(_("%d ly (%d torp%!1{s%})"), radius, eff.numTorps);
        }

        // Circle
        gfx::Point center = ren.scale(eff.center);
        ctx.setColor(eff.isEndangered ? ui::Color_Red : ui::Color_Yellow);
        if (radius > 0) {
            drawCircle(ctx, center, ren.scale(radius));
        }

        // Label
        ctx.setTextAlign(1, 0);
        outText(ctx, center + gfx::Point(0, 10), label);

        // FIXME: missing feature: limit check for laying
        //     // Limit check
        //     int limit = (md.type
        //                  ? config.MaximumWebMinefieldRadius(md.owner)
        //                  : config.MaximumMinefieldRadius(md.owner));
        //     if (radius > limit) {
        //         drawCircle(ctx, p, vp.scale(limit));
        //         ctx.setColor(COLOR_DARK);
        //         outText(ctx, p.x, p.y + 10 + font_heights[FONT_SMALL], _("<over limit>"));
        //     }
    }
}

bool
client::map::MinefieldMissionOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return true;
}

bool
client::map::MinefieldMissionOverlay::handleKey(util::Key_t /*key*/, int /*prefix*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::MinefieldMissionOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    return false;
}

void
client::map::MinefieldMissionOverlay::attach(game::proxy::ObjectObserver& oop)
{
    class Reply : public util::Request<MinefieldMissionOverlay> {
     public:
        Reply(game::Session& s, game::map::Object* obj)
            : m_data()
            {
                const game::map::Ship* pShip = dynamic_cast<const game::map::Ship*>(obj);
                const game::Root* pRoot = s.getRoot().get();
                const game::Game* pGame = s.getGame().get();
                const game::spec::ShipList* pShipList = s.getShipList().get();
                if (pShip != 0 && pRoot != 0 && pGame != 0 && pShipList != 0) {
                    game::map::MinefieldMission msn;
                    // FIXME: correct universe?
                    const game::map::Universe& univ = pGame->currentTurn().universe();
                    if (msn.checkLayMission(*pShip, univ, *pRoot, pGame->shipScores(), *pShipList)) {
                        computeMineLayEffect(m_data, msn, *pShip, univ, *pRoot);
                    }
                    if (msn.checkScoopMission(*pShip, *pRoot, pGame->shipScores(), *pShipList)) {
                        computeMineScoopEffect(m_data, msn, *pShip, univ, *pRoot, *pShipList);
                    }
                }
            }

        void handle(MinefieldMissionOverlay& t)
            { t.setEffects(m_data); }

     private:
        game::map::MinefieldEffects_t m_data;
    };

    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(util::RequestSender<MinefieldMissionOverlay> reply)
            : m_reply(reply)
            { }
        virtual void handle(game::Session& s, game::map::Object* obj)
            { m_reply.postNewRequest(new Reply(s, obj)); }
     private:
        util::RequestSender<MinefieldMissionOverlay> m_reply;
    };
    oop.addNewListener(new Listener(m_reply.getSender()));
}
