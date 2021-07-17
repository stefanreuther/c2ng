/**
  *  \file client/vcr/flak/flatrenderer.cpp
  *  \brief Class client::vcr::flak::FlatRenderer
  */

#include <cmath>
#include "client/vcr/flak/flatrenderer.hpp"
#include "gfx/clipfilter.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"

using game::vcr::flak::VisualisationState;

namespace {
    const int SHIP_RADIUS = 5;
    const int FIGHTER_RADIUS = 2;
    const int TORP_RADIUS = 3;
    const int SMOKE_RADIUS = 2;

    class Scaler {
     public:
        Scaler(const gfx::Rectangle& area, float range)
            : m_center(area.getCenter()), m_width(area.getWidth()), m_height(area.getHeight()), m_range(range*2)
            { }

        int scaleX(int32_t x) const
            { return int(float(m_center.getX()) + float(x) * float(m_width) / m_range); }

        int scaleY(int32_t y) const
            { return int(float(m_center.getY()) + float(y) * float(m_height) / m_range); }

        gfx::Point scale(const game::vcr::flak::Position& pos) const
            {
                // ex FlakFlatVisualizer::getScreenPosition
                return gfx::Point(scaleX(pos.x), scaleY(pos.y));
            }

     private:
        gfx::Point m_center;
        int m_width;
        int m_height;
        float m_range;
    };

    void drawGrid(gfx::Context<uint8_t>& ctx, const Scaler& scaler, int32_t size)
    {
        // ex FlakFlatBackgroundSprite::draw (sort-of)
        const int32_t DIST = 10000;
        const int dim = std::max(1, (size + 3000) / DIST);

        ctx.setColor(ui::Color_Dark);
        ctx.setLinePattern(gfx::DOTTED_LINE);
        int x1 = scaler.scaleX(-DIST*dim);
        int x2 = scaler.scaleX( DIST*dim);
        int y1 = scaler.scaleY(-DIST*dim);
        int y2 = scaler.scaleY( DIST*dim);

        for (int i = -dim; i <= dim; ++i) {
            int y = scaler.scaleY(DIST*i), x = scaler.scaleX(DIST*i);
            drawHLine(ctx, x1, y, x2);
            drawVLine(ctx, x, y1, y2);
        }
        ctx.setLinePattern(gfx::SOLID_LINE);
    }

    void drawTorpedo(gfx::BaseContext& ctx, gfx::Point pos)
    {
        drawHLine(ctx, pos.getX() - TORP_RADIUS, pos.getY(), pos.getX() + TORP_RADIUS);
        drawVLine(ctx, pos.getX(), pos.getY() - TORP_RADIUS, pos.getY() + TORP_RADIUS);
    }

    void drawBeams(gfx::Context<uint8_t>& ctx, const Scaler& scaler, afl::base::Memory<const VisualisationState::Beam> beams)
    {
        struct BeamDef {
            uint8_t color;
            uint8_t from;
            uint8_t to;
        };
        static const BeamDef BEAMS[] = {
            { ui::Color_Red, 0, 1 },
            { ui::Color_Red, 0, 2 },
            { ui::Color_DarkRed, 1, 3 },
            { ui::Color_Fire+5,  2, 3 },
        };
        const int NUM_BEAMS = sizeof(BEAMS)/sizeof(BEAMS[0]);

        while (const VisualisationState::Beam* p = beams.eat()) {
            if (p->age < NUM_BEAMS) {
                const BeamDef& d = BEAMS[p->age];
                ctx.setColor(d.color);

                gfx::Point a = scaler.scale(p->from);
                gfx::Point b = scaler.scale(p->to);
                drawLine(ctx,
                         gfx::Point(a.getX() + (b.getX() - a.getX()) * d.from / 3,
                                    a.getY() + (b.getY() - a.getY()) * d.from / 3),
                         gfx::Point(a.getX() + (b.getX() - a.getX()) * d.to / 3,
                                    a.getY() + (b.getY() - a.getY()) * d.to / 3));
            }
        }
    }

    void drawSmoke(gfx::Context<uint8_t>& ctx, const Scaler& scaler, afl::base::Memory<const VisualisationState::Smoke> smoke)
    {
        static const uint8_t COLORS[] = {
            ui::Color_White,
            ui::Color_White,
            ui::Color_Yellow,
            ui::Color_Red,
            ui::Color_Red,
            ui::Color_DarkRed,
        };
        const int NUM_COLORS = sizeof(COLORS)/sizeof(COLORS[0]);

        while (const VisualisationState::Smoke* p = smoke.eat()) {
            if (p->age < NUM_COLORS) {
                ctx.setColor(COLORS[p->age]);
                drawCircle(ctx, scaler.scale(p->pos), SMOKE_RADIUS);
            }
        }
    }
}

client::vcr::flak::FlatRenderer::FlatRenderer(ui::Root& root,
                                              game::vcr::flak::VisualisationState& state,
                                              game::vcr::flak::VisualisationSettings& settings)
    : m_root(root),
      m_state(state),
      m_settings(settings)
{ }

void
client::vcr::flak::FlatRenderer::init()
{ }

void
client::vcr::flak::FlatRenderer::draw(gfx::Canvas& can, const gfx::Rectangle& area, bool grid)
{
    Scaler scaler(area, m_settings.getCameraDistance());
    gfx::ClipFilter filter(can, area);
    gfx::Context<uint8_t> ctx(filter, m_root.colorScheme());

    // Clear everything
    drawSolidBar(ctx, area, ui::Color_Black);

    // Grid
    if (grid) {
        drawGrid(ctx, scaler, m_state.getGridSize());
    }

    // Fleets
    afl::base::Memory<const VisualisationState::Fleet> fleets = m_state.fleets();
    ctx.setColor(ui::Color_White);
    while (const VisualisationState::Fleet* f = fleets.eat()) {
        if (f->isAlive) {
            drawCircle(ctx, gfx::Point(scaler.scaleX(f->x), scaler.scaleY(f->y)), SHIP_RADIUS);
        }
    }

    // Objects
    afl::base::Memory<const VisualisationState::Object> objects = m_state.objects();
    while (const VisualisationState::Object* obj = objects.eat()) {
        // ex FlakSimpleObjectSprite::draw
        switch (obj->type) {
         case VisualisationState::NoObject:
            break;

         case VisualisationState::FighterObject:
            ctx.setColor(ui::Color_Blue);
            drawCircle(ctx, scaler.scale(obj->pos), FIGHTER_RADIUS);
            break;

         case VisualisationState::TorpedoObject:
            ctx.setColor(ui::Color_Red);
            drawTorpedo(ctx, scaler.scale(obj->pos));
            break;
        }
    }

    // Beams
    drawBeams(ctx, scaler, m_state.beams());

    // Explosions
    drawSmoke(ctx, scaler, m_state.smoke());
}
