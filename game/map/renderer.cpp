/**
  *  \file game/map/renderer.cpp
  */

#include "game/map/renderer.hpp"
#include "game/map/viewport.hpp"
#include "game/map/universe.hpp"
#include "game/map/configuration.hpp"
#include "game/map/renderlist.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/anyshiptype.hpp"

game::map::Renderer::Renderer(Viewport& viewport)
    : m_viewport(viewport)
{ }

game::map::Renderer::~Renderer()
{ }

void
game::map::Renderer::render(RendererListener& out)
{
    // ex GChartViewport::drawAux
    // FIXME: complete this
    renderGrid(out);
    // if (opt.show & GChartOptions::co_Mine)
    renderMinefields(out);
    // if (opt.show & GChartOptions::co_Ufo)
    //     drawUfos(can, univ, opt.fill);
    // if (opt.show & GChartOptions::co_Ion)
    //     drawIons(can, univ, opt.fill);
    // if (opt.show & GChartOptions::co_Drawings)
    renderDrawings(out);
    // drawShipSelAndVectors(can, univ, opt.show);
    renderPlanets(out);
    renderShips(out);
}

// /** Draw sector/starchart borders.
//     \param can    output canvas
//     \param show   options.show */
void
game::map::Renderer::renderGrid(RendererListener& out)
{
    // ex GChartViewport::drawSectors (sort-of)
    const Configuration& config = m_viewport.universe().config();

//     GfxContext ctx;
//     ctx.useColorScheme(GfxStandardColorScheme::instance);
//     ctx.useCanvas(can);
//     ctx.setColor(COLOR_DARK);

//     if (chart_conf.getMode() != GChartConfiguration::Circular) {
//         // Rectangular wrap or flat universe
    int dx = std::min(config.getSize().getX() / 200, 10);
    int dy = std::min(config.getSize().getY() / 200, 10);
    if (m_viewport.hasOption(Viewport::ShowGrid)) {
//         if (show & GChartOptions::co_Sectors) {
//             for (int img = getFirstImage(); img >= 0; img = getNextImage(img)) {
//                 GPoint center = chart_conf.getSimplePointAlias(chart_conf.getCenter(), img);
        Point center = config.getCenter();

//                 ctx.setLinePattern((origin_screen.y & 1) ? 0x55 : 0xAA);
        for (int i = -dx; i <= dx; ++i) {
            Point a(center.getX() + 100*i, center.getY() - 100*dy);
            Point b(center.getX() + 100*i, center.getY() + 100*dy);
            if (m_viewport.containsLine(a, b)) {
                out.drawGridLine(a, b);
            }
        }

//                 ctx.setLinePattern((origin_screen.x & 1) ? 0x55 : 0xAA);
        for (int i = -dx; i <= dx; ++i) {
            Point a(center.getX() - 100*dx, center.getY() + 100*i);
            Point b(center.getX() + 100*dx, center.getY() + 100*i);
            if (m_viewport.containsLine(a, b)) {
                out.drawGridLine(a, b);
            }
        }
    }

    if (m_viewport.hasOption(Viewport::ShowBorders)) {
//             for (int img = getFirstImage(); img >= 0; img = getNextImage(img)) {
//                 GPoint p1 = chart_conf.getSimplePointAlias(chart_conf.getMinimumCoordinates(), img);
//                 GPoint p2 = chart_conf.getSimplePointAlias(chart_conf.getMaximumCoordinates(), img);
        Point p1 = config.getMinimumCoordinates();
        Point p2 = config.getMaximumCoordinates();

//                 ctx.setLinePattern(ror8(0x27, origin_screen.y));
        out.drawBorderLine(p1, Point(p1.getX(), p2.getY()));
        out.drawBorderLine(Point(p2.getX(), p1.getY()), p2);

//                 ctx.setLinePattern(ror8(0x27, origin_screen.x));
        out.drawBorderLine(p1, Point(p2.getX(), p1.getY()));
        out.drawBorderLine(Point(p1.getX(), p2.getY()), p2);
//             }
    }
//     } else {
//         // Spherical wrap is special
//         const int size = chart_conf.getSize().x;
//         const int dx = std::min(int(size / 100), 10);
//         const int dy = dx;
//         const GPoint center = chart_conf.getCenter();
//         if (show & GChartOptions::co_Sectors) {
//             // Inside lines
//             int32_t sr = square(size);

//             ctx.setLinePattern((origin_screen.y & 1) ? 0x55 : 0xAA);
//             for (int i = -dx; i <= dx; ++i) {
//                 int yc = std::min(int(std::sqrt(double(sr - square(100 * i)))), 1000);
//                 drawVLine(ctx, scaleX(center.x + 100*i), scaleY(center.y - yc), scaleY(center.y + yc));
//             }

//             ctx.setLinePattern((origin_screen.x & 1) ? 0x55 : 0xAA);
//             for (int i = -dy; i <= dy; ++i) {
//                 int xc = std::min(int(std::sqrt(double(sr - square(100 * i)))), 1000);
//                 drawHLine(ctx, scaleX(center.x - xc), scaleY(center.y - 100*i), scaleX(center.x + xc));
//             }
//         }

//         if (show & GChartOptions::co_Borders) {
//             // Border
//             ctx.setLinePattern(0x27);
//             drawCircle(ctx, scaleXY(center), scale(size));
//         }

//         if (show & ~fill & GChartOptions::co_Sectors) {
//             // Vertical outside lines
//             ctx.setLinePattern(0x55);
//             for (int xi = -dx; xi <= dx; ++xi) {
//                 bool drawing = false;
//                 GfxPoint cursor;
//                 for (int yi = -10*dy; yi <= 10*dy; ++yi) {
//                     GPoint pt(100*xi + center.x, 10*yi + center.y);
//                     GPoint pt1;
//                     if (chart_conf.getPointAlias(pt, pt1, 1, false)) {
//                         GfxPoint c = scaleXY(pt1);
//                         if (drawing) {
//                             drawLine(ctx, cursor, c);
//                         }
//                         cursor = c;
//                         drawing = true;
//                     } else {
//                         drawing = false;
//                     }
//                 }
//             }

//             // Horizontal outside lines
//             for (int yi = -dy; yi <= dy; ++yi) {
//                 bool drawing = false;
//                 GfxPoint cursor;
//                 for (int xi = -10*dx; xi <= 10*dx; ++xi) {
//                     GPoint pt(10*xi + center.x, 100*yi + center.y);
//                     GPoint pt1;
//                     if (chart_conf.getPointAlias(pt, pt1, 1, false)) {
//                         GfxPoint c = scaleXY(pt1);
//                         if (drawing) {
//                             drawLine(ctx, cursor, c);
//                         }
//                         cursor = c;
//                         drawing = true;
//                     } else {
//                         drawing = false;
//                     }
//                 }
//             }
//         }
//     }
// }
}

void
game::map::Renderer::renderMinefields(RendererListener& out)
{
    // ex GChartViewport::drawMines
    const MinefieldType& ty = m_viewport.universe().minefields();
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        if (const Minefield* mf = ty.get(i)) {
            Point pt;
            int owner;
            int radius;
            if (mf->getPosition(pt) && mf->getOwner(owner) && mf->getRadius(radius)) {
                out.drawMinefield(pt, mf->getId(), radius, mf->isWeb(), m_viewport.teamSettings().getPlayerRelation(owner));
            }
        }
    }
}

// /** Draw user drawings. Call only if these are actually enabled.
//     \param can  Canvas to draw on
//     \param trn  Turn to get drawings from
//     \param show options.show, to test whether labels should be drawn */
void
game::map::Renderer::renderDrawings(RendererListener& out)
{
    // ex GChartViewport::drawDrawings
    const DrawingContainer& d = m_viewport.universe().drawings();
//     const bool with_text = (show & GChartOptions::co_Labels) != 0;
    for (DrawingContainer::Iterator_t i = d.begin(); i != d.end(); ++i) {
        if (const Drawing* p = *i) {
            if (p->isVisible()
                /* && (drawing_filter == NO_FILTER
//                 || (*i)->getTag() == drawing_filter) */)
            {
                renderDrawing(out, *p);
            }
        }
    }

//     GfxContext ctx;
//     ctx.useCanvas(can);
//     ctx.useColorScheme(GfxStandardColorScheme::instance);

//     GExplosionContainer& e = trn.getExplosions();
//     for (GExplosionContainer::iterator i = e.begin(); i != e.end(); ++i) {
//         bool big = divi < 2*mult;
//         for (int img = getFirstImage(); img >= 0; img = getNextImage(img)) {
//             GfxPoint pos = scaleXY(chart_conf.getSimplePointAlias(i->getPos(), img));
//             /* red '+' */
//             ctx.setColor(COLOR_RED);
//             drawMarker(ctx, getUserMarker(0, big), pos);
//             /* yellow 'x' */
//             ctx.setColor(COLOR_YELLOW);
//             drawMarker(ctx, getUserMarker(2, big), pos);
//         }
//     }
// }
}

void
game::map::Renderer::renderDrawing(RendererListener& out, const Drawing& d)
{
    switch (d.getType()) {
     case Drawing::LineDrawing:
        // for (int img = getFirstImage(); img >= 0; img = getNextImage(img)) {
        //     GfxPoint origin = scaleXY(chart_conf.getSimplePointAlias(d.getPos(),  img));
        //     GfxPoint end    = scaleXY(chart_conf.getSimplePointAlias(d.getPos2(), img));
        out.drawUserLine(d.getPos(), d.getPos2(), d.getColor());
        // }
        break;
     case Drawing::RectangleDrawing:
        // for (int img = getFirstImage(); img >= 0; img = getNextImage(img)) {
        //     GfxPoint origin = scaleXY(chart_conf.getSimplePointAlias(d.getPos(),  img));
        //     GfxPoint end    = scaleXY(chart_conf.getSimplePointAlias(d.getPos2(), img));
        out.drawUserRectangle(d.getPos(), d.getPos2(), d.getColor());
        // }
        break;
     case Drawing::CircleDrawing:
        // for (int img = getFirstImage(); img >= 0; img = getNextImage(img)) {
        //     GfxPoint origin = scaleXY(chart_conf.getSimplePointAlias(d.getPos(), img));
        out.drawUserCircle(d.getPos(), d.getCircleRadius(), d.getColor());
        // }
        break;
     case Drawing::MarkerDrawing:
        // for (int img = getFirstImage(); img >= 0; img = getNextImage(img)) {
        //     GfxPoint origin = scaleXY(chart_conf.getSimplePointAlias(d.getPos(), img));
        // FIXME: trim "|"
        out.drawUserMarker(d.getPos(), d.getMarkerKind(), d.getColor(), afl::string::strFirst(d.getComment(), "|"));
        //     if (divi < 2*mult) {
        //         /* draw marker */
        //         const TMarkerImage& img = getUserMarker(d.getMarkerKind(), true);
        //         drawMarker(ctx, img, origin);

        //         /* draw text */
        //         if (with_text) {
        //             string_t text = strFirst(d.getComment(), "|");
        //             if (text.size())
        //                 outTextF(ctx, origin.x, origin.y + img.height, 600, text);
        //         }
        //     } else {
        //         drawMarker(ctx, getUserMarker(d.getMarkerKind(), false), origin);
        //     }
        // }
        break;
    }

}

void
game::map::Renderer::renderPlanets(RendererListener& out)
{
    // ex GChartViewport::drawPlanets
    // const Configuration& config = m_viewport.universe().config();
    Universe& univ = m_viewport.universe();

    AnyPlanetType ty(univ);
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        Point pos;
        if (Planet* p = ty.getObjectByIndex(i)) {
            if (p->getPosition(pos)) {
                //     for (int img = getFirstImage(); img >= 0; img = getNextImage(img)) {
                renderPlanet(out, *p, pos);
                //         GfxPoint pt = scaleXY(chart_conf.getSimplePointAlias(p.getPos(), img));
                //         drawPlanetMarker(ctx, p, pt, show);
                //     }
                //     GPoint pt;
                //     if (chart_conf.getMode() == GChartConfiguration::Circular && chart_conf.getPointAlias(p.getPos(), pt, 1, true)) {
                //         drawPlanetMarker(ctx, p, scaleXY(pt), show);
                //     }
            }
        }
    }
}

void
game::map::Renderer::renderPlanet(RendererListener& out, const Planet& planet, Point pos)
{
    if (m_viewport.containsCircle(pos, 1)) {
        // Figure out flags
        int flags = 0;
        bool marked = planet.isMarked();

        // - ripUnowned, ripOwnPlanet, ripAlliedPlanet, ripEnemyPlanet
        int planetOwner;
        if (planet.getOwner(planetOwner)) {
            if (planetOwner == 0) {
                flags |= RendererListener::ripUnowned;
            } else {
                switch (m_viewport.teamSettings().getPlayerRelation(planetOwner)) {
                 case TeamSettings::ThisPlayer:
                    flags |= RendererListener::ripOwnPlanet;
                    break;

                 case TeamSettings::AlliedPlayer:
                    flags |= RendererListener::ripAlliedPlanet;
                    break;

                 case TeamSettings::EnemyPlayer:
                    flags |= RendererListener::ripEnemyPlanet;
                    break;
                }
            }
        } else {
            if (planet.hasAnyPlanetData()) {
                flags |= RendererListener::ripUnowned;
            }
        }

        // - ripHasBase
        if (planet.hasBase()) {
            flags |= RendererListener::ripHasBase;
        }

        // - ripOwnShips, ripAlliedShips, ripEnemyShips, ripGuessedAlliedShips, ripGuessedEnemyShips
        AnyShipType ships(m_viewport.universe());
        for (Id_t sid = ships.findFirstObjectAt(pos); sid != 0; sid = ships.findNextObjectAt(pos, sid)) {
            if (Ship* s = ships.getObjectByIndex(sid)) {
                int shipOwner;
                if (s->getOwner(shipOwner)) {
                    switch (m_viewport.teamSettings().getPlayerRelation(shipOwner)) {
                     case TeamSettings::ThisPlayer:
                        flags |= RendererListener::ripOwnShips;
                        break;

                     case TeamSettings::AlliedPlayer:
                        flags |= s->isReliablyVisible(0)
                            ? RendererListener::ripAlliedShips
                            : RendererListener::ripGuessedAlliedShips;
                        break;

                     case TeamSettings::EnemyPlayer:
                        flags |= s->isReliablyVisible(0)
                            ? RendererListener::ripEnemyShips
                            : RendererListener::ripGuessedEnemyShips;
                        break;
                    }
                }
                if (s->isMarked()) {
                    marked = true;
                }
            }
        }

        // Draw it
        // if (show & GChartOptions::co_WarpWells)
        //     drawWarpWell(can, pt);
        if (m_viewport.hasOption(Viewport::ShowSelection) && marked) {
            out.drawSelection(pos);
        }
        out.drawPlanet(pos, planet.getId(), flags);
    }
}

void
game::map::Renderer::renderShips(RendererListener& out)
{
    // ex GChartViewport::drawShips, sort-of
    // GfxContext ctx;
    // ctx.useCanvas(can);
    // ctx.useColorScheme(GfxStandardColorScheme::instance);
    // ctx.setTextAlign(1, 0);
    // ctx.useFont(*fonts[getFont()]);

    AnyShipType ships(m_viewport.universe());
    for (Id_t i = ships.findNextIndex(0); i != 0; i = ships.findNextIndex(i)) {
        if (Ship* s = ships.getObjectByIndex(i)) {
            Point shipPosition;
            if (s->getPosition(shipPosition)
                && AnyPlanetType(m_viewport.universe()).findFirstObjectAt(shipPosition) == 0)
            {
                // Ship has known owner and is not at a planet
                // for (int img = getFirstImage(); img >= 0; img = getNextImage(img)) {
                //     GfxPoint pt = scaleXY(chart_conf.getSimplePointAlias(s.getPos(), img));
                renderShipMarker(out, *s, shipPosition);
                // }

                // if (chart_conf.getMode() == GChartConfiguration::Circular) {
                //     GPoint pt;
                //     if (chart_conf.getPointAlias(s.getPos(), pt, 1, true)) {
                //         drawShipMarker(ctx, s, scaleXY(pt), show);
                //     }
                // }
            }
        }
    }
}

void
game::map::Renderer::renderShipMarker(RendererListener& out,
                                      const Ship& ship,
                                      Point shipPosition)
{
    // ex GChartViewport::drawShipMarker

    // Check ship owner
    int shipOwner;
    if (!ship.getOwner(shipOwner)) {
        return;
    }

    // FIXME: does not belong here; goes in drawShipSelAndVectors
    if (m_viewport.hasOption(Viewport::ShowSelection) && ship.isMarked()) {
        out.drawSelection(shipPosition);
    }

    // Draw
    TeamSettings::Relation rel = m_viewport.teamSettings().getPlayerRelation(shipOwner);
    out.drawShip(shipPosition, ship.getId(), rel);

    // FIXME: where does this go?
    if (ship.isFleetLeader()) {
        out.drawFleetLeader(shipPosition, ship.getId(), rel);
    }

//     if ((show & GChartOptions::co_Labels) != 0) {
//         if (sh.getOrbitPlanetId() != 0)
//             pt.y += 4;
//         if ((show & GChartOptions::co_ShipDots) == 0)
//             pt.y += 1;
//         outText(ctx, pt.x, pt.y, getLabel(ShipLabel, sh.getId()));
//     }
}
