/**
  *  \file client/map/renderer.cpp
  */

#include "client/map/renderer.hpp"
#include "client/marker.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"

namespace {
    const int SCRingRadius           = 3;      ///< Radius of planet ring, in ly.
    const int SCMaxRingRadius        = 6;      ///< Max.\ radius of scaled planet ring, in pixels.
    const int SCCrossRadius          = 6;      ///< Size of starbase cross, in ly.
    const int SCMaxCrossRadius       = 12;     ///< Max.\ radius of scaled SB cross, in pixels.
    const int SCObjCrossRadius       = 5;      ///< Size of cross in center of a circular object (mine/UFO), in ly.
    const int SCMaxObjCrossRadius    = 10;     ///< Max.\ size of scaled cross in center of a circular object, in pixels.
    const int SCWPCrossRadius        = 10;     ///< Size of cross at end of waypoint, in ly.
    const int SCMaxWPCrossRadius     = 20;     ///< Max.\ size of scaled cross at end of waypoint, in pixels.

    const int SCMaxIconHeight = 50;
    const int SCMaxIconWidth  = 300;

    void drawCross(gfx::Context& ctx, gfx::Point pt, int size)
    {
        // ex GChartViewport::drawCross (part)
        drawHLine(ctx, pt.getX() - size, pt.getY(), pt.getX() + size);
        drawVLine(ctx, pt.getX(), pt.getY() - size, pt.getY() + size);
    }

    uint8_t getShipColor(game::TeamSettings::Relation rel)
    {
        switch (rel) {
         case game::TeamSettings::ThisPlayer:   return ui::Color_Green;
         case game::TeamSettings::AlliedPlayer: return ui::Color_Yellow;
         case game::TeamSettings::EnemyPlayer:  return ui::Color_Red;
        }
        return ui::Color_Pink;
    }

    uint8_t getMinefieldColor(game::TeamSettings::Relation rel)
    {
        switch (rel) {
         case game::TeamSettings::ThisPlayer:   return ui::Color_GreenScale+4;
         case game::TeamSettings::AlliedPlayer: return ui::Color_DarkYellowScale+4;
         case game::TeamSettings::EnemyPlayer:  return ui::Color_Dark;
        }
        return ui::Color_DarkPink;
    }
}

class client::map::Renderer::Listener : public game::map::RendererListener {
 public:
    Listener(gfx::Context& ctx, const Renderer& parent)
        : m_context(ctx),
          m_parent(parent)
        { }
    virtual void drawGridLine(game::map::Point a, game::map::Point b)
        {
            gfx::Point ax = m_parent.scale(a);
            gfx::Point bx = m_parent.scale(b);

            // FIXME: align the pattern!
            m_context.setLinePattern(0x55);
            m_context.setColor(ui::Color_Dark);
            drawLine(m_context, ax, bx);
        }

    virtual void drawBorderLine(game::map::Point a, game::map::Point b)
        {
            gfx::Point ax = m_parent.scale(a);
            gfx::Point bx = m_parent.scale(b);

            // FIXME: align the pattern!
            m_context.setLinePattern(0x27);
            m_context.setColor(ui::Color_Dark);
            drawLine(m_context, ax, bx);
        }

    virtual void drawSelection(game::map::Point p)
        {
            m_context.setColor(ui::Color_Yellow);
            client::drawSelection(m_context, m_parent.scale(p), m_parent.m_zoomMultiplier, m_parent.m_zoomDivider);
        }

    virtual void drawPlanet(game::map::Point p, int /*id*/, int flags)
        {
            // ex GChartViewport::drawPlanetMarker
            gfx::Point ptx = m_parent.scale(p);

            // Determine ship ring color
            bool shipGuessed;
            int shipRingColor;
            switch (flags & (ripOwnShips | ripEnemyShips | ripAlliedShips)) {
             case 0:
                shipGuessed = true;
                switch (flags & (ripGuessedAlliedShips | ripGuessedEnemyShips)) {
                 case 0:
                    shipRingColor = 0;
                    break;
                 case ripGuessedAlliedShips:
                    shipRingColor = ui::Color_Yellow;  // ?
                    break;
                 case ripGuessedEnemyShips:
                    shipRingColor = ui::Color_Red;
                    break;
                 default:
                    shipRingColor = ui::Color_Yellow;
                    break;
                }
                break;

             case ripOwnShips:
                shipGuessed = false;
                shipRingColor = ui::Color_Green;
                break;

             case ripEnemyShips:
                shipGuessed = false;
                shipRingColor = ui::Color_Red;
                break;

             case ripAlliedShips:
             case ripAlliedShips + ripOwnShips:
                shipGuessed = false;
                shipRingColor = ui::Color_Yellow;  // ?
                break;

             default:
                shipGuessed = false;
                shipRingColor = ui::Color_Yellow;
            }

            // Determine planet ring color
            int planetRingColor;
            bool planetDotted;
            if ((flags & ripOwnPlanet) != 0) {
                planetRingColor = ui::Color_Blue;
                planetDotted = false;
            } else if ((flags & ripAlliedPlanet) != 0) {
                planetRingColor = ui::Color_DarkYellowScale + 7;
                planetDotted = false;
            } else if ((flags & ripEnemyPlanet) != 0) {
                planetRingColor = ui::Color_BlueBlack;
                planetDotted = false;
            } else if ((flags & ripUnowned) != 0) {
                planetRingColor = ui::Color_BlueBlack;
                planetDotted = false;
            } else {
                planetRingColor = 0;
                planetDotted = false;
            }

            // Draw starbase marker
            m_context.setColor(planetRingColor);
            m_context.setLinePattern(0xFF);
            if ((flags & ripHasBase) != 0) {
                int sbsize = std::min(m_parent.scale(SCCrossRadius), SCMaxCrossRadius);
                drawHLine(m_context, ptx.getX() - sbsize, ptx.getY(), ptx.getX() + sbsize);
                drawVLine(m_context, ptx.getX(), ptx.getY() - sbsize, ptx.getY() + sbsize);
            }

            // Draw planet ring
            int textIncrement = 1;   // ex incy
            int r = std::min(m_parent.scale(SCRingRadius), m_parent.scale(SCMaxRingRadius));
            if (shipRingColor == 0 && r == 0) {
                // In small zoom levels, the radius comes out as 0, meaning no planet ring.
                // If we don't have a ship ring that highlights the planet, force the planet ring to be visible.
                r = 1;
            }
            if (r != 0 && planetRingColor != 0) {
                if (planetDotted) {
                    drawDottedCircle(m_context, ptx, r);
                } else {
                    drawCircle(m_context, ptx, r);
                }
                textIncrement = r+1;
            }

            // Draw planet dot
            m_context.setColor(ui::Color_White);
            drawPixel(m_context, ptx);

            // Draw ship ring
            if (shipRingColor != 0) {
                m_context.setColor(shipRingColor);
                ++r;
                if (shipGuessed) {
                    drawDottedCircle(m_context, ptx, r);
                } else {
                    drawCircle(m_context, ptx, r);
                }
                textIncrement = r+1;
            }

            // ctx.setColor(COLOR_GRAY);
            // if ((show & GChartOptions::co_Labels) != 0) {
            //     outText(ctx, pt.x, pt.y + incy, getLabel(PlanetLabel, pl.getId()));
            (void) textIncrement;
            // }
        }

    virtual void drawShip(game::map::Point p, int /*id*/, Relation_t rel)
        {
            gfx::Point pt = m_parent.scale(p);
            m_context.setColor(getShipColor(rel));
//     if ((show & GChartOptions::co_ShipDots) != 0 && sh.getOrbitPlanetId() == 0) {
            drawPixel(m_context, pt);
//         can.drawPixel(pt, ctx.getRawColor(), GFX_ALPHA_OPAQUE);
//     } else {
//         drawShipIcon(...); FIXME
//     }
        }

    virtual void drawFleetLeader(game::map::Point p, int /*id*/, Relation_t rel)
        {
            gfx::Point pt = m_parent.scale(p);
            m_context.setColor(getShipColor(rel));
            drawPixel(m_context, pt + gfx::Point(-1, -1));
            drawPixel(m_context, pt + gfx::Point(+1, -1));
            drawPixel(m_context, pt + gfx::Point(-1, +1));
            drawPixel(m_context, pt + gfx::Point(+1, +1));
        }

    virtual void drawMinefield(game::map::Point p, int /*id*/, int r, bool /*isWeb*/, Relation_t rel)
        {
            // Determine style
            m_context.setLinePattern(0xFF);
            m_context.setColor(getMinefieldColor(rel));

            // Draw
            // FIXME: handle filled/hollow status
            // drawObject(ctx, pt, scale(pmf.getRadius()), fill & GChartOptions::co_Mine);
            gfx::Point pt = m_parent.scale(p);
            drawCircle(m_context, pt, m_parent.scale(r));
            drawCross(m_context, pt, m_parent.getCrossSize());
        }


 private:
    gfx::Context& m_context;
    const Renderer& m_parent;
};



client::map::Renderer::Renderer()
    : m_area(),
      m_renderList(),
      m_zoomMultiplier(1),
      m_zoomDivider(1),
      m_center()
{ }

client::map::Renderer::~Renderer()
{ }

void
client::map::Renderer::setExtent(const gfx::Rectangle& area)
{
    m_area = area;
}

void
client::map::Renderer::setCenter(game::map::Point center)
{
    m_center = center;
}

void
client::map::Renderer::setRenderList(afl::base::Ptr<game::map::RenderList> renderList)
{
    m_renderList = renderList;
}

void
client::map::Renderer::draw(gfx::Canvas& can, ui::ColorScheme& colorScheme) const
{
    gfx::Context ctx(can);
    ctx.useColorScheme(colorScheme);
    if (m_renderList.get() != 0) {
        Listener painter(ctx, *this);
        m_renderList->replay(painter);
    }
}

void
client::map::Renderer::getPreferredWorldRange(game::map::Point& min, game::map::Point& max) const
{
    // Get minimum world range
    getMinimumWorldRange(min, max);

    // Allow +200 for scrolling
    min -= game::map::Point(200, 200);
    max += game::map::Point(200, 200);
}

void
client::map::Renderer::getMinimumWorldRange(game::map::Point& min, game::map::Point& max) const
{
    // Get half size. Add some fuzz factor.
    int fuzz  = 5;
    int halfX = (m_area.getWidth()/2  + SCMaxIconWidth)  * m_zoomDivider / m_zoomMultiplier + fuzz;
    int halfY = (m_area.getHeight()/2 + SCMaxIconHeight) * m_zoomDivider / m_zoomMultiplier + fuzz;

    // Generate output
    min = game::map::Point(m_center.getX() - halfX, m_center.getY() - halfY);
    max = game::map::Point(m_center.getX() + halfX, m_center.getY() + halfY);
}

gfx::Point
client::map::Renderer::scale(game::map::Point pt) const
{
    gfx::Point screenCenter = m_area.getCenter();
    return gfx::Point(screenCenter.getX() + (pt.getX() - m_center.getX()) * m_zoomMultiplier / m_zoomDivider,
                      screenCenter.getY() - (pt.getY() - m_center.getY()) * m_zoomMultiplier / m_zoomDivider);
}

int
client::map::Renderer::scale(int r) const
{
    return r * m_zoomMultiplier / m_zoomDivider;
}

int
client::map::Renderer::getCrossSize() const
{
    // ex GChartViewport::drawCross (part)
    int n = scale(SCObjCrossRadius);
    if (n <= 0) {
        return 1;
    } else if (n > SCMaxObjCrossRadius) {
        return SCMaxObjCrossRadius;
    } else {
        return n;
    }
}
