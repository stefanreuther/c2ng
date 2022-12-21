/**
  *  \file client/map/renderer.cpp
  */

#include <cmath>
#include "client/map/renderer.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/rotate.hpp"
#include "client/marker.hpp"
#include "game/map/drawing.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "util/math.hpp"

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

    /* PCC1 (using 16-bit int) limits zoom levels to 10 to avoid integer overflow for common cases (coordinates <= 3000).
       Given that we (hopefully) use 32-bit int, we could legally allow huge zoom levels even for uncommon cases
       (coordinates up to 10000); limit for now to avoid people doing too stupid things.
       It will probably still have performance impact. */
    const int MAX_ZOOM = sizeof(int) < sizeof(int32_t) ? 10 : 100;

    const uint8_t IONSTORM_FILL[] = { 0x88, 0x00, 0x22, 0x00,
                                      0x88, 0x00, 0x22, 0x00 };
    // const uint8_t IONSTORM_DENSE_FILL[] = { 0x88, 0x44, 0x22, 0x44,
    //                                         0x88, 0x44, 0x22, 0x44 };
    const uint8_t UFO_FILL[] = { 0x88, 0x55, 0x22, 0x55,
                                 0x88, 0x55, 0x22, 0x55 };

    void drawCross(gfx::BaseContext& ctx, gfx::Point pt, int size)
    {
        // ex GChartViewport::drawCross (part), chart.pas:NDrawCross
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

    uint8_t getShipTrailColor(game::TeamSettings::Relation rel, int age)
    {
        int delta = 7 - std::min(7, age >> 1);
        switch (rel) {
         case game::TeamSettings::ThisPlayer:   return static_cast<uint8_t>(ui::Color_GreenScale + delta);
         case game::TeamSettings::AlliedPlayer: return static_cast<uint8_t>(ui::Color_DarkYellowScale + delta);
         case game::TeamSettings::EnemyPlayer:  return static_cast<uint8_t>(ui::Color_Fire + delta);
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


    /* In addition to the internal<->external conversion, we expose a
       simple color number (0..NUM_USER_COLORS, where 0 isn't selectable
       through dialogs) to the user. */
    static const uint8_t user_colors[] = {
        0,
        1,   2,   3,   4,   5,   6,   7,   8,   9,  15,
        97,  99,  101, 103, 105, 107, 109, 111, 113, 115,
        98,  100, 102, 104, 106, 108, 110, 112, 114, 116
    };
    static_assert(countof(user_colors) == game::map::Drawing::NUM_USER_COLORS + 1, "countof user_colors");

    static const uint8_t ufo_colors[] = {
        ui::Color_Black,     ui::Color_DarkBlue,      ui::Color_DarkGreen,    ui::Color_DarkCyan,
        ui::Color_DarkRed,   ui::Color_DarkMagenta,   ui::Color_DarkYellow,   ui::Color_Gray,
        ui::Color_Dark,      ui::Color_BrightBlue,    ui::Color_BrightGreen,  ui::Color_BrightCyan,
        ui::Color_BrightRed, ui::Color_BrightMagenta, ui::Color_BrightYellow, ui::Color_White
    };

    uint8_t getIonStormColor(int voltage)
    {
        if (voltage < 50) {
            return ui::Color_Blue;
        } else if (voltage < 100) {
            return ui::Color_Gray;
        } else if (voltage < 150) {
            return ui::Color_White;
        } else {
            return ui::Color_Red;
        }
    }
}

class client::map::Renderer::Listener : public game::map::RendererListener {
 public:
    Listener(gfx::Context<uint8_t>& ctx, const Renderer& parent)
        : m_context(ctx),
          m_parent(parent)
        { }

    void drawObject(gfx::Point center, int scaledRadius, bool filled)
        {
            // ex chart.pas:DrawCircle
            if (filled) {
                drawFilledCircle(m_context, center, scaledRadius);
            } else {
                drawCircle(m_context, center, scaledRadius);
            }
        }

    void drawMovingObject(gfx::Point center, int scaledRadius, int speed, int heading, bool filled)
        {
            // ex GChartViewport::drawMovingObject, chart.pas:NDrawThing
            drawObject(center, scaledRadius, filled);
            if (speed > 0 && speed <= 20 && heading >= 0) {
                double h = heading * util::PI / 180.0;

                center.addX( util::roundToInt(scaledRadius * std::sin(h)));
                center.addY(-util::roundToInt(scaledRadius * std::cos(h)));

                int way = m_parent.scale(speed * speed);
                int dx = util::roundToInt(way * std::sin(h));
                int dy = util::roundToInt(way * std::cos(h));

                drawArrow(m_context, center, center + gfx::Point(dx, -dy), m_parent.scale(10) < 5 ? 3 : 5);
            }
        }

    void setMineFillStyle(gfx::Point pt, bool isWeb, Relation_t rel)
        {
            // ex chart.pas:SetMineFillStyle
            // Adjust pattern position to avoid that own and foreing minefields hide each other
            if (rel == game::TeamSettings::ThisPlayer) {
                pt.addY(1);
            }

            if (isWeb) {
                m_context.setFillPattern(gfx::FillPattern(gfx::FillPattern::GRAY50).shiftUp((pt.getX() + pt.getY()) & 1));
            } else {
                m_context.setFillPattern(gfx::FillPattern(gfx::FillPattern::GRAY25).shiftUp((15 + 2*(pt.getX() & 1) - pt.getY()) & 3));
            }
        }

    int getLinePatternAligner(game::map::Point a, game::map::Point b) const
        {
            const int dx = std::abs(a.getX() - b.getX());
            const int dy = std::abs(a.getY() - b.getY());
            gfx::Point zero = m_parent.scale(game::map::Point());
            if (dx > dy) {
                return zero.getX();
            } else {
                return zero.getY();
            }
        }

    virtual void drawGridLine(game::map::Point a, game::map::Point b)
        {
            gfx::Point ax = m_parent.scale(a);
            gfx::Point bx = m_parent.scale(b);

            m_context.setLinePattern(afl::bits::rotateRight8(0xAA, getLinePatternAligner(a, b) & 1));
            m_context.setColor(ui::Color_Dark);
            drawLine(m_context, ax, bx);
        }

    virtual void drawBorderLine(game::map::Point a, game::map::Point b)
        {
            gfx::Point ax = m_parent.scale(a);
            gfx::Point bx = m_parent.scale(b);

            m_context.setLinePattern(afl::bits::rotateRight8(0x27, getLinePatternAligner(a, b) & 7));
            m_context.setColor(ui::Color_Dark);
            drawLine(m_context, ax, bx);
        }

    virtual void drawBorderCircle(game::map::Point c, int r)
        {
            gfx::Point cx = m_parent.scale(c);
            int rx = m_parent.scale(r);

            m_context.setLinePattern(0x27);
            m_context.setColor(ui::Color_Dark);
            drawCircle(m_context, cx, rx);
        }

    virtual void drawSelection(game::map::Point p)
        {
            m_context.setColor(ui::Color_Yellow);
            client::drawSelection(m_context, m_parent.scale(p), m_parent.m_zoomMultiplier, m_parent.m_zoomDivider);
        }

    virtual void drawMessageMarker(game::map::Point p)
        {
            m_context.setColor(ui::Color_BrightMagenta);
            client::drawMessageMarker(m_context, m_parent.scale(p), m_parent.m_zoomMultiplier, m_parent.m_zoomDivider);
        }

    virtual void drawPlanet(game::map::Point p, int /*id*/, int flags, String_t label)
        {
            // ex GChartViewport::drawPlanetMarker
            const gfx::Point ptx = m_parent.scale(p);

            // Determine ship ring color
            bool shipGuessed;
            uint8_t shipRingColor;
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
            uint8_t planetRingColor;
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

            // Label
            if (!label.empty()) {
                m_context.setColor(ui::Color_Gray);
                m_context.setTextAlign(gfx::CenterAlign, gfx::TopAlign);
                outText(m_context, ptx + gfx::Point(0, textIncrement), label);
            }
        }

    virtual void drawShip(game::map::Point p, int /*id*/, Relation_t rel, int flags, String_t label)
        {
            const gfx::Point pt = m_parent.scale(p);
            m_context.setColor(getShipColor(rel));
            if ((flags & risShowDot) != 0) {
                drawPixel(m_context, pt);
            }
            if ((flags & risShowIcon) != 0) {
                drawShipIcon(m_context, pt, rel == game::TeamSettings::ThisPlayer, m_parent.scale(10) > 5);
            }
            if ((flags & risFleetLeader) != 0) {
                drawPixel(m_context, pt + gfx::Point(-1, -1));
                drawPixel(m_context, pt + gfx::Point(+1, -1));
                drawPixel(m_context, pt + gfx::Point(-1, +1));
                drawPixel(m_context, pt + gfx::Point(+1, +1));
            }
            if (!label.empty()) {
                int textIncrement = 0;
                if ((flags & risAtPlanet) != 0) {
                    textIncrement += 4;
                }
                if ((flags & risShowDot) == 0) {
                    textIncrement += 1;
                }
                m_context.setTextAlign(gfx::CenterAlign, gfx::TopAlign);
                outText(m_context, pt + gfx::Point(0, textIncrement), label);
            }
        }

    virtual void drawMinefield(game::map::Point p, int /*id*/, int r, bool isWeb, Relation_t rel, bool filled)
        {
            // Determine style
            m_context.setLinePattern(0xFF);
            m_context.setColor(getMinefieldColor(rel));

            // Draw
            gfx::Point pt = m_parent.scale(p);
            setMineFillStyle(m_parent.scale(game::map::Point(0, 0)), isWeb, rel);
            drawObject(pt, m_parent.scale(r), filled);
            drawCross(m_context, pt, m_parent.getCrossSize());
        }

    virtual void drawUfo(game::map::Point p, int /*id*/, int r, int colorCode, int speed, int heading, bool filled)
        {
            // ex GChartViewport::drawUfos (part)
            const gfx::Point center = m_parent.scale(p);

            m_context.setLinePattern(0xFF);
            m_context.setColor(getUfoColor(colorCode));
            m_context.setFillPattern(gfx::FillPattern(UFO_FILL).shiftDown(center.getY() & 3).shiftRight(center.getX() & 3));

            drawMovingObject(center, m_parent.scale(r), speed, heading, filled);
            drawCross(m_context, center, m_parent.getCrossSize());
        }
    virtual void drawUfoConnection(game::map::Point a, game::map::Point b, int colorCode)
        {
            // ex GChartViewport::drawUfos (part)
            m_context.setLinePattern(0xFF);
            m_context.setColor(getUfoColor(colorCode));
            drawLine(m_context, m_parent.scale(a), m_parent.scale(b));
        }
    virtual void drawIonStorm(game::map::Point p, int r, int voltage, int speed, int heading, bool filled)
        {
            // ex GChartViewport::drawIons, chart.pas:NDrawIons (part)
            m_context.setLinePattern(0xFF);
            m_context.setFillPattern(IONSTORM_FILL);
            m_context.setColor(getIonStormColor(voltage));

            drawMovingObject(m_parent.scale(p), m_parent.scale(r), speed, heading, filled);
        }
    virtual void drawUserCircle(game::map::Point pt, int r, int color)
        {
            // ex GChartViewport::drawDrawing
            m_context.setLinePattern(0xFF);
            m_context.setColor(getUserColor(color));
            drawCircle(m_context, m_parent.scale(pt), m_parent.scale(r));
        }
    virtual void drawUserLine(game::map::Point a, game::map::Point b, int color)
        {
            // ex GChartViewport::drawDrawing
            m_context.setLinePattern(0xFF);
            m_context.setColor(getUserColor(color));
            drawLine(m_context, m_parent.scale(a), m_parent.scale(b));
        }
    virtual void drawUserRectangle(game::map::Point a, game::map::Point b, int color)
        {
            // ex GChartViewport::drawDrawing
            m_context.setLinePattern(0xFF);
            m_context.setColor(getUserColor(color));

            gfx::Point aa = m_parent.scale(a);
            gfx::Point bb = m_parent.scale(b);
            drawRectangle(m_context, gfx::Rectangle(std::min(aa.getX(), bb.getX()),
                                                    std::min(aa.getY(), bb.getY()),
                                                    std::abs(aa.getX() - bb.getX()) + 1,
                                                    std::abs(aa.getY() - bb.getY()) + 1));
        }
    virtual void drawUserMarker(game::map::Point pt, int shape, int color, String_t label)
        {
            // ex GChartViewport::drawDrawing
            m_context.setLinePattern(0xFF);
            m_context.setColor(getUserColor(color));
            m_context.setTextAlign(gfx::CenterAlign, gfx::TopAlign);
            gfx::Point origin = m_parent.scale(pt);
            if (m_parent.m_zoomDivider < 2*m_parent.m_zoomMultiplier) {
                /* draw marker */
                if (const Marker* marker = getUserMarker(shape, true)) {
                    drawMarker(m_context, *marker, origin);

                    /* draw text */
                    if (!label.empty()) {
                        outTextF(m_context, origin + gfx::Point(0, getMarkerHeight(*marker)), 600, label);
                    }
                }
            } else {
                if (const Marker* marker = getUserMarker(shape, false)) {
                    drawMarker(m_context, *marker, origin);
                }
            }
        }

    virtual void drawExplosion(game::map::Point p)
        {
            // bool big = divi < 2*mult;
            bool big = m_parent.scale(10) > 5;
            gfx::Point pp = m_parent.scale(p);

            // Red '+'
            m_context.setColor(ui::Color_Red);
            drawMarker(m_context, *getUserMarker(0, big), pp);

            // Yellow 'x'
            m_context.setColor(ui::Color_Yellow);
            drawMarker(m_context, *getUserMarker(2, big), pp);
        }

    virtual void drawShipTrail(game::map::Point a, game::map::Point b, Relation_t rel, int flags, int age)
        {
            m_context.setColor(getShipTrailColor(rel, age));
            m_context.setLinePattern(0xFF);

            gfx::Point ax = m_parent.scale(a);
            gfx::Point bx = m_parent.scale(b);

            // If we are coming from a real position, draw a knob.
            // (No need to special-case going to a position; in that case, the next trail or the ship will be at that place.)
            if ((flags & TrailFromPosition) != 0) {
                drawPixel(m_context, ax + gfx::Point(0, -1));
                drawPixel(m_context, ax + gfx::Point(0, +1));
                drawPixel(m_context, ax + gfx::Point(-1, 0));
                drawPixel(m_context, ax + gfx::Point(+1, 0));
            }

            // Draw line
            drawLine(m_context, ax, bx);
        }

    virtual void drawShipWaypoint(game::map::Point a, game::map::Point b, Relation_t /*rel*/)
        {
            int waypointCrossSize = std::min(SCMaxWPCrossRadius, m_parent.scale(SCWPCrossRadius));
            m_context.setColor(ui::Color_Dark);
            m_context.setLinePattern(0xFF);

            gfx::Point ax = m_parent.scale(a);
            gfx::Point bx = m_parent.scale(b);
            drawLine(m_context, ax, bx);
            drawHLine(m_context, bx.getX()-waypointCrossSize, bx.getY(), bx.getX()+waypointCrossSize);
            drawVLine(m_context, bx.getX(), bx.getY()-waypointCrossSize, bx.getY()+waypointCrossSize);
        }

    virtual void drawShipVector(game::map::Point a, game::map::Point b, Relation_t /*rel*/)
        {
            int arrowHeadSize = m_parent.scale(10) >= 5 ? 5 : 3;
            m_context.setColor(ui::Color_Gray);
            m_context.setLinePattern(0xFF);
            drawArrow(m_context, m_parent.scale(a), m_parent.scale(b), arrowHeadSize);
        }

    virtual void drawWarpWellEdge(game::map::Point a, Edge e)
        {
            m_context.setColor(ui::Color_Shield+4);
            m_context.setLinePattern(0xFF);

            gfx::Point p = m_parent.scale(a);
            int half = m_parent.scale(1)/2;
            switch (e) {
             case North: drawHLine(m_context, p.getX()-half, p.getY()-half, p.getX()+half); break;
             case South: drawHLine(m_context, p.getX()-half, p.getY()+half, p.getX()+half); break;
             case East:  drawVLine(m_context, p.getX()+half, p.getY()-half, p.getY()+half); break;
             case West:  drawVLine(m_context, p.getX()-half, p.getY()-half, p.getY()+half); break;
            }
        }

 private:
    gfx::Context<uint8_t>& m_context;
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

const gfx::Rectangle&
client::map::Renderer::getExtent() const
{
    return m_area;
}

void
client::map::Renderer::draw(gfx::Canvas& can, ui::ColorScheme& colorScheme, gfx::ResourceProvider& provider) const
{
    gfx::Context<uint8_t> ctx(can, colorScheme);
    setFont(ctx, provider);

    if (m_renderList.get() != 0) {
        Listener painter(ctx, *this);
        m_renderList->replay(painter);
    }
}

void
client::map::Renderer::drawDrawing(gfx::Canvas& can, ui::ColorScheme& colorScheme, gfx::ResourceProvider& provider, const game::map::Drawing& d, uint8_t color) const
{
    gfx::Context<uint8_t> ctx(can, colorScheme);
    setFont(ctx, provider);

    Listener painter(ctx, *this);
    switch (d.getType()) {
     case game::map::Drawing::LineDrawing:
        painter.drawUserLine(d.getPos(), d.getPos2(), color);
        break;
     case game::map::Drawing::RectangleDrawing:
        painter.drawUserRectangle(d.getPos(), d.getPos2(), color);
        break;
     case game::map::Drawing::CircleDrawing:
        painter.drawUserCircle(d.getPos(), d.getCircleRadius(), color);
        break;
     case game::map::Drawing::MarkerDrawing:
        painter.drawUserMarker(d.getPos(), d.getMarkerKind(), color, afl::string::strFirst(d.getComment(), "|"));
        break;
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

// /** Convert screen point to game point. */
game::map::Point
client::map::Renderer::unscale(gfx::Point pt) const
{
    // ex GChartViewport::unscaleXY
    gfx::Point screenCenter = m_area.getCenter();
    return game::map::Point(unscale(pt.getX() - screenCenter.getX()) + m_center.getX(),
                            unscale(screenCenter.getY() - pt.getY()) + m_center.getY());
}

// /** Unscale. Convert screen distance into game distance. */
int
client::map::Renderer::unscale(int r) const
{
    // ex GChartViewport::unscale
    if ((-1)/2 == 0 && r < 0) {
        return -(-int32_t(r) * m_zoomDivider) / m_zoomMultiplier;
    } else {
        return int32_t(r) * m_zoomDivider / m_zoomMultiplier;
    }
}

game::map::Point
client::map::Renderer::getCenter() const
{
    return m_center;
}

bool
client::map::Renderer::zoomIn()
{
    // ex chart.pas:ZoomIn
    if (m_zoomDivider > 1) {
        --m_zoomDivider;
        return true;
    } else if (m_zoomMultiplier < MAX_ZOOM) {
        ++m_zoomMultiplier;
        return true;
    } else {
        return false;
    }
}

bool
client::map::Renderer::zoomOut()
{
    // ex chart.pas:ZoomOut
    if (m_zoomMultiplier > 1) {
        --m_zoomMultiplier;
        return true;
    } else if (m_zoomDivider < MAX_ZOOM) {
        ++m_zoomDivider;
        return true;
    } else {
        return false;
    }
}

void
client::map::Renderer::setZoom(int mult, int divi)
{
    m_zoomMultiplier = std::max(1, std::min(MAX_ZOOM, mult));
    m_zoomDivider    = std::max(1, divi);
}

bool
client::map::Renderer::isValidZoomLevel(int mult, int divi) const
{
    return mult > 0 && divi > 0 && mult <= MAX_ZOOM && divi <= MAX_ZOOM;
}

int
client::map::Renderer::getZoomMultiplier() const
{
    return m_zoomMultiplier;
}

int
client::map::Renderer::getZoomDivider() const
{
    return m_zoomDivider;
}

void
client::map::Renderer::setFont(gfx::BaseContext& ctx, gfx::ResourceProvider& provider) const
{
    // Font
    // ex GChartViewport::getFont
    int16_t fontSize;
    if (m_zoomMultiplier > m_zoomDivider) {
        if (m_zoomMultiplier > 2*m_zoomDivider) {
            /* more then 2:1: 22 pt */
            fontSize = +1;
        } else {
            /* 1:1 up to 2:1: 16 pt */
            fontSize = 0;
        }
    } else {
        if (2*m_zoomMultiplier < m_zoomDivider) {
            /* smaller than 1:2 */
            // FIXME: should be 6 or 8 pt font, as in PCC 1.x */
            fontSize = -2;
        } else {
            /* 1:2 up to 1:1: 12 pt */
            fontSize = -1;
        }
    }
    ctx.useFont(*provider.getFont(gfx::FontRequest().setSize(fontSize)));
}

uint8_t
client::map::getUserColor(int color)
{
    // ex GDrawing::getColorFromUserColor
    if (color >= 0 && color < int(countof(user_colors))) {
        return user_colors[color];
    } else {
        return ui::Color_White;
    }
}

uint8_t
client::map::getUfoColor(int color)
{
    // ex GUfo::getColor
    if (color >= 0 && color < int(countof(ufo_colors))) {
        return ufo_colors[color];
    } else {
        return ui::Color_White;
    }
}
