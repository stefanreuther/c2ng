/**
  *  \file game/map/renderer.cpp
  */

#include <cmath>
#include "game/map/renderer.hpp"
#include "afl/bits/smallset.hpp"
#include "game/interface/labelextra.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/boundingbox.hpp"
#include "game/map/configuration.hpp"
#include "game/map/historyshiptype.hpp"
#include "game/map/renderlist.hpp"
#include "game/map/universe.hpp"
#include "game/map/viewport.hpp"
#include "util/math.hpp"

using game::config::HostConfiguration;
using game::interface::LabelExtra;

class game::map::Renderer::State {
 public:
    State(const Viewport& viewport, RendererListener& listener)
        : m_viewport(viewport),
          m_listener(listener),
          m_maxImage(),
          m_visibleImages()
        {
            BoundingBox bbox;
            bbox.addUniverse(viewport.universe(), viewport.mapConfiguration());

            m_maxImage = viewport.mapConfiguration().getNumRectangularImages();
            for (int i = 0; i < m_maxImage; ++i) {
                if (viewport.containsRectangle(bbox.getMinimumCoordinates(), bbox.getMaximumCoordinates())) {
                    m_visibleImages += i;
                }
            }
        }

    // FIXME: make a distinction between rectangular images (getFirstImage, getNextImage, getSimplePointAlias)
    // and point images (for planets/ships/markers, getPointAlias).

    /** Get index of first visible map image.
        \return index, or -1 if none */
    int getFirstImage() const
        {
            // ex GChartViewport::getFirstImage
            return getNextImage(-1);
        }

    /** Get index of next map image after n.
        \return index, or -1 if none */
    int getNextImage(int n) const
        {
            // ex GChartViewport::getNextImage
            while (1) {
                ++n;
                if (n >= m_maxImage) {
                    return -1;
                }
                if (m_visibleImages.contains(n)) {
                    return n;
                }
            }
        }

    void drawGridLine(Point a, Point b) const
        {
            if (m_viewport.containsLine(a, b)) {
                m_listener.drawGridLine(a, b);
            }
        }

    void drawBorderLine(Point a, Point b) const
        {
            if (m_viewport.containsLine(a, b)) {
                m_listener.drawBorderLine(a, b);
            }
        }

    void drawShipTrail(Point a, Point b, TeamSettings::Relation rel, int flags, int age) const
        {
            if (m_viewport.containsLine(a, b)) {
                m_listener.drawShipTrail(a, b, rel, flags, age);
            }
        }

    void drawShipWaypoint(Point a, Point b, TeamSettings::Relation rel) const
        {
            if (m_viewport.containsLine(a, b)) {
                m_listener.drawShipWaypoint(a, b, rel);
            }
        }

    void drawShipVector(Point a, Point b, TeamSettings::Relation rel) const
        {
            if (m_viewport.containsLine(a, b)) {
                m_listener.drawShipVector(a, b, rel);
            }
        }

    RendererListener& listener() const
        { return m_listener; }

 private:
    const Viewport& m_viewport;
    RendererListener& m_listener;
    int m_maxImage;
    afl::bits::SmallSet<int> m_visibleImages;
};



game::map::Renderer::Renderer(Viewport& viewport)
    : m_viewport(viewport)
{ }

game::map::Renderer::~Renderer()
{ }

void
game::map::Renderer::render(RendererListener& out) const
{
    // ex GChartViewport::drawAux
    State st(m_viewport, out);

    renderGrid(st);

    if (m_viewport.hasOption(Viewport::ShowMinefields)) {
        renderMinefields(st);
    }
    if (m_viewport.hasOption(Viewport::ShowUfos)) {
        renderUfos(st);
    }
    if (m_viewport.hasOption(Viewport::ShowIonStorms)) {
        renderIonStorms(st);
    }
    if (m_viewport.hasOption(Viewport::ShowDrawings)) {
        renderDrawings(st);
    }
    renderShipExtras(st);
    renderPlanets(st);
    renderShips(st);
}

/* Render grid and borders.
   Handles ShowGrid, ShowBorders, ShowOutsideGrid options. */
void
game::map::Renderer::renderGrid(const State& st) const
{
    // ex GChartViewport::drawSectors (sort-of)
    const Configuration& config = m_viewport.mapConfiguration();
    switch (config.getMode()) {
     case Configuration::Flat:
     case Configuration::Wrapped:
        renderRectangularGrid(st);
        break;

     case Configuration::Circular:
        renderCircularGrid(st);
        break;
    }
}

/* Implementation of renderGrid() for rectangular maps. */
void
game::map::Renderer::renderRectangularGrid(const State& st) const
{
    const Configuration& config = m_viewport.mapConfiguration();
    const int dx = std::min(config.getSize().getX() / 200, 10);
    const int dy = std::min(config.getSize().getY() / 200, 10);

    // Grid
    if (m_viewport.hasOption(Viewport::ShowGrid)) {
        for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
            const Point center = config.getSimplePointAlias(config.getCenter(), img);

            for (int i = -dx; i <= dx; ++i) {
                const Point a(center.getX() + 100*i, center.getY() - 100*dy);
                const Point b(center.getX() + 100*i, center.getY() + 100*dy);
                st.drawGridLine(a, b);
            }

            for (int i = -dx; i <= dx; ++i) {
                const Point a(center.getX() - 100*dx, center.getY() + 100*i);
                const Point b(center.getX() + 100*dx, center.getY() + 100*i);
                st.drawGridLine(a, b);
            }
        }
    }

    // Borders
    if (m_viewport.hasOption(Viewport::ShowBorders)) {
        for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
            const Point p1 = config.getSimplePointAlias(config.getMinimumCoordinates(), img);
            const Point p2 = config.getSimplePointAlias(config.getMaximumCoordinates(), img);
            st.drawBorderLine(p1, Point(p1.getX(), p2.getY()));
            st.drawBorderLine(Point(p2.getX(), p1.getY()), p2);
            st.drawBorderLine(p1, Point(p2.getX(), p1.getY()));
            st.drawBorderLine(Point(p1.getX(), p2.getY()), p2);
        }
    }
}

/* Implementation of renderGrid() for circular maps. */
void
game::map::Renderer::renderCircularGrid(const State& st) const
{
    const Configuration& config = m_viewport.mapConfiguration();

    const int size = config.getSize().getX();
    const int dx = std::min(int(size / 100), 10);
    const int dy = dx;
    const Point center = config.getCenter();

    if (m_viewport.hasOption(Viewport::ShowGrid)) {
        // Inside lines
        const int32_t sr = util::squareInteger(size);
        for (int i = -dx; i <= dx; ++i) {
            int yc = std::min(int(std::sqrt(double(sr - util::squareInteger(100 * i)))), 1000);
            st.drawGridLine(Point(center.getX() + 100*i, center.getY() - yc),
                            Point(center.getX() + 100*i, center.getY() + yc));
        }

        for (int i = -dy; i <= dy; ++i) {
            int xc = std::min(int(std::sqrt(double(sr - util::squareInteger(100 * i)))), 1000);
            st.drawGridLine(Point(center.getX() - xc, center.getY() - 100*i),
                            Point(center.getX() + xc, center.getY() - 100*i));
        }
    }

    if (m_viewport.hasOption(Viewport::ShowBorders)) {
        // Border
        st.listener().drawBorderCircle(center, size);
    }

    if (m_viewport.hasOption(Viewport::ShowOutsideGrid)) {
        // Vertical outside lines
        for (int xi = -dx; xi <= dx; ++xi) {
            bool drawing = false;
            Point cursor;
            for (int yi = -10*dy; yi <= 10*dy; ++yi) {
                Point pt(100*xi + center.getX(), 10*yi + center.getY());
                Point pt1;
                if (config.getPointAlias(pt, pt1, 1, false)) {
                    if (drawing) {
                        st.drawGridLine(cursor, pt1);
                    }
                    cursor = pt1;
                    drawing = true;
                } else {
                    drawing = false;
                }
            }
        }

        // Horizontal outside lines
        for (int yi = -dy; yi <= dy; ++yi) {
            bool drawing = false;
            Point cursor;
            for (int xi = -10*dx; xi <= 10*dx; ++xi) {
                Point pt(10*xi + center.getX(), 100*yi + center.getY());
                Point pt1;
                if (config.getPointAlias(pt, pt1, 1, false)) {
                    if (drawing) {
                        st.drawGridLine(cursor, pt1);
                    }
                    cursor = pt1;
                    drawing = true;
                } else {
                    drawing = false;
                }
            }
        }
    }
}

/* Render minefields.
   Handles FillMinefields option. */
void
game::map::Renderer::renderMinefields(const State& st) const
{
    // ex GChartViewport::drawMines
    bool filled = m_viewport.hasOption(Viewport::FillMinefields);
    bool decay = m_viewport.hasOption(Viewport::ShowMineDecay);
    const MinefieldType& ty = m_viewport.universe().minefields();
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        if (const Minefield* mf = ty.get(i)) {
            Point pt;
            int owner;
            int radius;
            if (mf->getPosition(pt) && mf->getOwner(owner) && mf->getRadius(radius)) {
                const Configuration& config = m_viewport.mapConfiguration();
                if (decay) {
                    radius = Minefield::getRadiusFromUnits(mf->getUnitsAfterDecay(mf->getUnits(), m_viewport.hostVersion(), m_viewport.hostConfiguration()));
                }
                for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                    const Point imgPos = config.getSimplePointAlias(pt, img);
                    if (m_viewport.containsCircle(imgPos, radius)) {
                        st.listener().drawMinefield(imgPos, mf->getId(), radius, mf->isWeb(), m_viewport.teamSettings().getPlayerRelation(owner), filled);
                    }
                }
            }
        }
    }
}

/* Render Ufos.
   Handles FillUfos option. */
void
game::map::Renderer::renderUfos(const State& st) const
{
    // ex GChartViewport::drawUfos
    const Configuration& config = m_viewport.mapConfiguration();
    UfoType& ty = m_viewport.universe().ufos();
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        if (const Ufo* ufo = ty.getObjectByIndex(i)) {
            Point center;
            int radius;
            if (ufo->getRadius(radius) && ufo->getPosition(center)) {
                // Check other end
                // FIXME: here, we remain in this map image, even if drawing across the seam
                // would produce a shorter line.
                Point otherCenter;
                bool drawOther = false;
                if (const Ufo* otherEnd = ufo->getOtherEnd()) {
                    if (otherEnd->getPosition(otherCenter)) {
                        if (center.getY() < otherCenter.getY() || (center.getY() == otherCenter.getY() && center.getX() < otherCenter.getX())) {
                            drawOther = true;
                        }
                    }
                }

                for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                    // Draw the Ufo
                    Point imgCenter = config.getSimplePointAlias(center, img);
                    if (m_viewport.containsCircle(imgCenter, radius)) {
                        st.listener().drawUfo(imgCenter, i, radius, ufo->getColorCode(), ufo->getSpeed().orElse(-1), ufo->getHeading().orElse(-1), m_viewport.hasOption(Viewport::FillUfos));
                    }

                    // Draw connection to other end
                    if (drawOther) {
                        Point imgOtherCenter = config.getSimplePointAlias(otherCenter, img);
                        if (m_viewport.containsLine(imgCenter, imgOtherCenter)) {
                            st.listener().drawUfoConnection(imgCenter, imgOtherCenter, ufo->getColorCode());
                        }
                    }
                }
            }
        }
    }
}

/* Render ion storms.
   Handles FillIonStorms option. */
void
game::map::Renderer::renderIonStorms(const State& st) const
{
    // ex GChartViewport::drawIons
    const Configuration& config = m_viewport.mapConfiguration();
    IonStormType& ty = m_viewport.universe().ionStormType();
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        if (const IonStorm* ion = ty.getObjectByIndex(i)) {
            Point center;
            int radius;
            if (ion->getRadius(radius) && ion->getPosition(center)) {
                for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                    Point imgCenter = config.getSimplePointAlias(center, img);
                    if (m_viewport.containsCircle(imgCenter, radius)) {
                        st.listener().drawIonStorm(imgCenter, radius, ion->getVoltage().orElse(0), ion->getSpeed().orElse(0), ion->getHeading().orElse(-1), m_viewport.hasOption(Viewport::FillIonStorms));
                    }
                }
            }
        }
    }
}

/* Render user drawings and explosions. */
void
game::map::Renderer::renderDrawings(const State& st) const
{
    // ex GChartViewport::drawDrawings
    // Drawings
    const DrawingContainer& d = m_viewport.universe().drawings();
    for (DrawingContainer::Iterator_t i = d.begin(); i != d.end(); ++i) {
        if (const Drawing* p = *i) {
            if (p->isVisible() && m_viewport.isDrawingTagVisible(p->getTag())) {
                renderDrawing(st, *p);
            }
        }
    }

    // Explosions
    const Configuration& config = m_viewport.mapConfiguration();
    ExplosionType& e = m_viewport.universe().explosions();
    for (Id_t i = e.findNextIndex(0); i != 0; i = e.findNextIndex(i)) {
        if (const Explosion* ex = e.getObjectByIndex(i)) {
            Point pt;
            if (ex->getPosition(pt)) {
                for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                    Point imgPos = config.getSimplePointAlias(pt, img);
                    if (m_viewport.containsCircle(imgPos, 10)) {
                        st.listener().drawExplosion(imgPos);
                    }
                }
            }
        }
    }
}

/* Render single drawing. */
void
game::map::Renderer::renderDrawing(const State& st, const Drawing& d) const
{
    const Configuration& config = m_viewport.mapConfiguration();
    switch (d.getType()) {
     case Drawing::LineDrawing:
        for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
            const Point origin = config.getSimplePointAlias(d.getPos(),  img);
            const Point end    = config.getSimplePointAlias(d.getPos2(), img);
            if (m_viewport.containsLine(origin, end)) {
                st.listener().drawUserLine(origin, end, d.getColor());
            }
        }
        break;
     case Drawing::RectangleDrawing:
        for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
            const Point origin = config.getSimplePointAlias(d.getPos(),  img);
            const Point end    = config.getSimplePointAlias(d.getPos2(), img);
            if (m_viewport.containsRectangle(origin, end)) {
                st.listener().drawUserRectangle(origin, end, d.getColor());
            }
        }
        break;
     case Drawing::CircleDrawing:
        for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
            const Point origin = config.getSimplePointAlias(d.getPos(), img);
            if (m_viewport.containsCircle(origin, d.getCircleRadius())) {
                st.listener().drawUserCircle(origin, d.getCircleRadius(), d.getColor());
            }
        }
        break;
     case Drawing::MarkerDrawing: {
        // Label
        String_t label;
        if (m_viewport.hasOption(Viewport::ShowLabels)) {
            label = afl::string::strFirst(d.getComment(), "|");
        }

        // An estimate of the size, for clipping purposes
        Point dim(20 + 30*std::min(1000, int(label.size())), 20);

        for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
            const Point origin = config.getSimplePointAlias(d.getPos(), img);
            if (m_viewport.containsRectangle(origin - dim, origin + dim)) {
                st.listener().drawUserMarker(origin, d.getMarkerKind(), d.getColor(), label);
            }
        }
        break;
     }
    }
}

/* Render ship extras.
   Renders all ship icons with MORE than one pixel (=which we allow to be covered by one-pixel things later).
   - selections
   - message markers
   - fleet leader icons
   - ship icons if ShowShipDots is disabled
   - history trails */
void
game::map::Renderer::renderShipExtras(const State& st) const
{
    // ex GChartViewport::drawShipSelAndVectors
    const Configuration& config = m_viewport.mapConfiguration();
    AnyShipType ty(m_viewport.universe().ships());

    // Selections
    if (m_viewport.hasOption(Viewport::ShowSelection)) {
        // Draw markers for selected ships
        // FIXME: do not draw when ship is orbiting a planet; in this case, renderPlanet does it.
        for (Id_t i = ty.findNextIndexNoWrap(0, true); i != 0; i = ty.findNextIndexNoWrap(i, true)) {
            const Ship* sh = ty.getObjectByIndex(i);
            Point shipPosition;
            if (sh != 0 && sh->getPosition(shipPosition)) {
                // Regular images
                for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                    st.listener().drawSelection(config.getSimplePointAlias(shipPosition, img));
                }

                // Special case for circular wrap
                if (config.getMode() == Configuration::Circular) {
                    Point pt;
                    if (config.getPointAlias(shipPosition, pt, 1, true)) {
                        st.listener().drawSelection(pt);
                    }
                }
            }
        }
    }

    // Message markers
    if (m_viewport.hasOption(Viewport::ShowMessages)) {
        for (Id_t i = ty.findNextIndexNoWrap(0, false); i != 0; i = ty.findNextIndexNoWrap(i, false)) {
            const Ship* sh = ty.getObjectByIndex(i);
            Point shipPosition;
            if (sh != 0 && !sh->messages().empty() && sh->getPosition(shipPosition)) {
                for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                    st.listener().drawMessageMarker(config.getSimplePointAlias(shipPosition, img));
                }
            }
        }

        AnyPlanetType pty(m_viewport.universe().planets());
        for (Id_t i = pty.findNextIndexNoWrap(0, false); i != 0; i = pty.findNextIndexNoWrap(i, false)) {
            const Planet* pl = pty.getObjectByIndex(i);
            Point planetPosition;
            if (pl != 0 && !pl->messages().empty() && pl->getPosition(planetPosition)) {
                for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                    st.listener().drawMessageMarker(config.getSimplePointAlias(planetPosition, img));
                }
            }
        }
    }

    // Ship and fleet icons (risShowIcon, risFleetLeader)
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        const Ship* sh = ty.getObjectByIndex(i);
        Point shipPosition;
        int shipOwner;
        if (sh != 0 && sh->getPosition(shipPosition) && sh->getOwner(shipOwner)) {
            // Draw icon if enabled and we're not at a planet
            int flags = 0;
            if (!m_viewport.hasOption(Viewport::ShowShipDots)
                && AnyPlanetType(m_viewport.universe().planets()).findNextObjectAt(shipPosition, 0, false) == 0)
            {
                flags |= RendererListener::risShowIcon;
            }

            // Draw fleet marker if required
            if (sh->isFleetLeader()) {
                flags |= RendererListener::risFleetLeader;
            }

            // Draw if any icon
            if (flags != 0) {
                const TeamSettings::Relation rel = m_viewport.teamSettings().getPlayerRelation(shipOwner);
                for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                    st.listener().drawShip(config.getSimplePointAlias(shipPosition, img), sh->getId(), rel, flags, String_t());
                }

                // Special case for circular wrap
                if (config.getMode() == Configuration::Circular) {
                    Point pt;
                    if (config.getPointAlias(shipPosition, pt, 1, true)) {
                        st.listener().drawShip(pt, sh->getId(), rel, flags, String_t());
                    }
                }
            }
        }
    }

    // Ship trails
    HistoryShipType& histType = m_viewport.universe().historyShips();
    if (m_viewport.hasOption(Viewport::ShowTrails)) {
        // All trails
        for (Id_t i = histType.findNextIndex(0); i != 0; i = histType.findNextIndex(i)) {
            const Ship* sh = histType.getObjectByIndex(i);
            int shipOwner;
            if (sh != 0 && sh->getOwner(shipOwner)) {
                renderShipTrail(st, *sh, shipOwner, m_viewport.getTurnNumber());
                renderShipVector(st, *sh, shipOwner);
            }
        }
    } else if (const Ship* sh = histType.getObjectByIndex(m_viewport.getShipTrailId())) {
        // One ship's trail
        int shipOwner;
        if (sh->getOwner(shipOwner)) {
            renderShipTrail(st, *sh, shipOwner, m_viewport.getTurnNumber());
            renderShipVector(st, *sh, shipOwner);
        }
    } else {
        // No trails
    }
}

/* Render single ship trail (=past positions). */
void
game::map::Renderer::renderShipTrail(const State& st, const Ship& sh, int shipOwner, int turnNumber) const
{
    // ex GChartViewport::drawShipTrail
    // Like PCC2,  we try not to assume any knowledge about how many ship track entries there are per ship.
    // Therefore, we always draw 16 turns max (stemming from the fact that PCC2 UI uses 8 colors).
    // We draw forward in time, so that a new line overwrites an old one if needed.
    // Unlike PCC2, we do the image loop here, not in the caller, to allow for possible handling of wrap.
    const Configuration& config = m_viewport.mapConfiguration();
    const TeamSettings::Relation rel = m_viewport.teamSettings().getPlayerRelation(shipOwner);

    const int LIMIT = 16;
    for (int i = 0; i < LIMIT; ++i) {
        const ShipHistoryData::Track* me   = sh.getHistoryLocation(turnNumber - LIMIT + i);
        const ShipHistoryData::Track* next = sh.getHistoryLocation(turnNumber - LIMIT + i+1);

        int age = LIMIT-i-1;

        // Draw lines
        int meX, meY, nextX, nextY;
        int speed, heading;
        if (   me != 0   && me->x.get(meX)     && me->y.get(meY)
            && next != 0 && next->x.get(nextX) && next->y.get(nextY))
        {
            // Both positions known, so simply connect them
            // FIXME: wrapping code from PCC1 is missing here
//         IF ChartConf.Wrap=cmWrap THEN BEGIN
//           sx := ScaleL(view, ChartConf.SizeX);
//           sy := ScaleL(view, ChartConf.SizeY);
//           IF nX+sx < XCor THEN Inc(nX, 2*sx) ELSE
//           IF Xcor < nX-sx THEN Dec(nX, 2*sx);

//           IF nY+sy < YCor THEN Inc(nY, 2*sy) ELSE
//           IF Ycor < nY-sy THEN Dec(nY, 2*sy);
//         {$IFDEF CircularWrap}
//         END ELSE IF ChartConf.Wrap=cmCircle THEN BEGIN
//           { check whether we wrapped this turn }
//           sx := psh^.locs[slot+1].X - psh^.locs[slot].X;
//           sy := psh^.locs[slot+1].Y - psh^.locs[slot].Y;
//           IF Sqr(LONGINT(sx)) + Sqr(LONGINT(sy)) >= Sqr(LONGINT(ChartConf.SizeX)) THEN BEGIN
//             { yes, we wrapped.
//               Draw two lines: one from the previous location to the
//               current point outside, one from the previous outside
//               point to the current location }
//             sx := psh^.locs[slot].X;
//             sy := psh^.locs[slot].Y;
//             qx := psh^.locs[slot+1].X;
//             qy := psh^.locs[slot+1].Y;
//             IF CircularMoveOutside(sx, sy, TRUE) AND CircularMoveOutside(qx, qy, TRUE) THEN BEGIN
//               MLineTo(view.x0 + ScaleL(view, qx), view.y0 - ScaleL(view, qy));
//               MoveTo(view.x0 + ScaleL(view, sx), view.y0 - ScaleL(view, sy));
//             END;
//           END;
//         {$ENDIF}
            Point mePos(meX, meY);
            Point nextPos(nextX, nextY);
            for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                st.drawShipTrail(config.getSimplePointAlias(mePos, img), config.getSimplePointAlias(nextPos, img), rel, RendererListener::TrailFromPosition | RendererListener::TrailToPosition, age);
            }
        } else if (me != 0 && me->x.get(meX) && me->y.get(meY) && me->heading.get(heading) && me->speed.get(speed)) {
            // This position and heading known, so draw a line leaving here
            int way = std::max(15, util::squareInteger(speed) / 2);
            Point mePos(meX, meY);
            Point nextPos(meX + util::roundToInt(way * std::sin(heading * util::PI / 180)),
                          meY + util::roundToInt(way * std::cos(heading * util::PI / 180)));
            for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                st.drawShipTrail(config.getSimplePointAlias(mePos, img), config.getSimplePointAlias(nextPos, img), rel, RendererListener::TrailFromPosition, age);
            }
        } else if (next != 0 && next->x.get(nextX) && next->y.get(nextY) && next->heading.get(heading) && next->speed.get(speed)) {
            // This position and heading known, so draw a line arriving there
            int way = std::max(15, util::squareInteger(speed) / 2);
            Point nextPos(nextX, nextY);
            Point mePos(nextX - util::roundToInt(way * std::sin(heading * util::PI / 180)),
                        nextY - util::roundToInt(way * std::cos(heading * util::PI / 180)));
            for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                st.drawShipTrail(config.getSimplePointAlias(mePos, img), config.getSimplePointAlias(nextPos, img), rel, RendererListener::TrailToPosition, age);
            }
        } else {
            // Nothing known
        }
    }
}

/* Render single ship vector (=future ship positions). */
void
game::map::Renderer::renderShipVector(const State& st, const Ship& sh, int shipOwner) const
{
    // ex GChartViewport::drawShipVector
    // Change to PCC2: this does not call drawShipTrail. This does the image loop internally for possible wrap support.
    const Configuration& config = m_viewport.mapConfiguration();
    const TeamSettings::Relation rel = m_viewport.teamSettings().getPlayerRelation(shipOwner);

    /* Auto Task */
//     ppred := Ships^[sid].Predict;
//     IF ppred<>NIL THEN BEGIN
//       Color := Dark;
//       WriteMode := wmOR;
//       x0 := Ships^[sid].x;
//       y0 := Ships^[sid].y;
//       x0t := view.x0 + ScaleL(view, x0);
//       y0t := view.y0 - ScaleL(view, y0);
//       FOR i:=1 TO ppred^.valid DO BEGIN
//         x1 := ppred^.x[i];
//         y1 := ppred^.y[i];
//         MoveToNearest(x0,y0,x1,y1);
//         x1t := view.x0 + ScaleL(view, x1);
//         y1t := view.y0 - ScaleL(view, y1);
//         LongLine(x0t, y0t, x1t-x0t, y1t-y0t);
//         IF (x1t > -waypointCrossSize) AND (y1t > -waypointCrossSize) AND (x1t < 5000) AND (y1t < 5000) THEN BEGIN
//           j := waypointCrossSize DIV 2;
//           HLine(x1t-j, y1t, x1t+j);
//           VLine(x1t, y1t-j, y1t+j);
//         END;
//         x0 := x1;
//         y0 := y1;
//         x0t := x1t;
//         y0t := y1t;
//       END;
//       WriteMode := 0;
//     END;

    Point shipPos;
    if (sh.getPosition(shipPos)) {
        // Waypoint
        Point shipWaypoint;
        if (sh.getWaypoint().get(shipWaypoint) && shipWaypoint != shipPos) {
            for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                st.drawShipWaypoint(config.getSimplePointAlias(shipPos, img), config.getSimplePointAlias(shipWaypoint, img), rel);
            }
        }

        // Speed and heading
        int speed, heading;
        if (sh.getWarpFactor
            ().get(speed) && sh.getHeading().get(heading) && speed > 0) {
            // This is a simplification against PCC1/PCC2 by using the computed heading.
            // It comes with a certain imprecision: a 81 ly circle has a circumference of 509, but we reach only 360 points = 70%.
            int dist = util::squareInteger(speed);
            if (sh.hasSpecialFunction(game::spec::BasicHullFunction::Gravitonic, m_viewport.shipScores(), m_viewport.shipList(), m_viewport.hostConfiguration())) {
                dist *= 2;
            }

            Point end = shipPos + Point(util::roundToInt(dist * std::sin(heading * util::PI / 180)),
                                        util::roundToInt(dist * std::cos(heading * util::PI / 180)));
            for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                st.drawShipVector(config.getSimplePointAlias(shipPos, img), config.getSimplePointAlias(end, img), rel);
            }
        }
    }
}

/* Render planets. */
void
game::map::Renderer::renderPlanets(const State& st) const
{
    // ex GChartViewport::drawPlanets
    Universe& univ = m_viewport.universe();

    AnyPlanetType ty(univ.planets());
    for (Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        Point pos;
        if (Planet* p = ty.getObjectByIndex(i)) {
            if (p->getPosition(pos)) {
                renderPlanet(st, *p, pos);
            }
        }
    }
}

/* Render single planet. */
void
game::map::Renderer::renderPlanet(const State& st, const Planet& planet, Point pos) const
{
    // An estimate of the size of a planet icon, including rings, markers, warp well, etc.
    // Setting this too low means a partially-visible icon at the edge disappears a little too quick.
    const int SIZE = 15;

    const Configuration& config = m_viewport.mapConfiguration();

    bool infoKnown = false;
    std::pair<int,bool> info;

    String_t label;
    if (m_viewport.hasOption(Viewport::ShowLabels)) {
        if (const LabelExtra* ex = m_viewport.labels()) {
            label = ex->planetLabels().getLabel(planet.getId());
        }
    }

    for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
        const Point imgPos = config.getSimplePointAlias(pos, img);
        if (m_viewport.containsCircle(imgPos, SIZE)) {
            // Figure out flags
            if (!infoKnown) {
                info = getPlanetFlags(planet, pos);
                infoKnown = true;
            }

            // Draw it
            if (m_viewport.hasOption(Viewport::ShowWarpWells)) {
                renderWarpWell(st, imgPos);
            }
            if (m_viewport.hasOption(Viewport::ShowSelection) && info.second) {
                st.listener().drawSelection(imgPos);
            }
            st.listener().drawPlanet(imgPos, planet.getId(), info.first, label);
        }
    }

    // Special case for circular
    if (config.getMode() == Configuration::Circular) {
        Point imgPos;
        if (config.getPointAlias(pos, imgPos, 1, true)) {
            if (m_viewport.containsCircle(imgPos, SIZE)) {
                // Figure out flags
                if (!infoKnown) {
                    info = getPlanetFlags(planet, pos);
                    infoKnown = true;
                }

                // Draw it
                if (m_viewport.hasOption(Viewport::ShowWarpWells)) {
                    renderWarpWell(st, imgPos);
                }
                if (m_viewport.hasOption(Viewport::ShowSelection) && info.second) {
                    st.listener().drawSelection(imgPos);
                }
                st.listener().drawPlanet(imgPos, planet.getId(), info.first, label);
            }
        }
    }
}

/* Render single warp well. */
void
game::map::Renderer::renderWarpWell(const State& st, Point pos) const
{
    // ex GChartViewport::drawWarpWell
    // For now, keep it simple, functionality-wise and speed-wise.
    // Functionality: this just draws an edgy circle, and does not draw deformed warp wells when they overlap (same as PCC1/2).
    // Speed: this generates one call per edge, and therefore a correspondingly large number of scaling operations.
    // We should eventually make this more efficient.

    const HostConfiguration& config = m_viewport.hostConfiguration();
    const int range = config[HostConfiguration::GravityWellRange]();

    if (config[HostConfiguration::AllowGravityWells]() && range > 0) {
        if (config[HostConfiguration::RoundGravityWells]()) {
            // Draw 8 octants, tracing a circle, starting at (range,0), until we meed the 45 degree point
            int wwx = range;
            int wwy = 0;

            // North/south/east/west poles
            st.listener().drawWarpWellEdge(pos + Point(-wwx, 0), RendererListener::West);
            st.listener().drawWarpWellEdge(pos + Point( wwx, 0), RendererListener::East);
            st.listener().drawWarpWellEdge(pos + Point(0, -wwx), RendererListener::South);
            st.listener().drawWarpWellEdge(pos + Point(0,  wwx), RendererListener::North);

            while (wwx > wwy) {
                // If advancing a step away from the axis, step towards the other axis;
                // draw the "cap" of the side step.
                if (util::squareInteger(wwx) + util::squareInteger(wwy+1) > util::squareInteger(range)) {
                    st.listener().drawWarpWellEdge(pos + Point(-wwx, -wwy), RendererListener::South);
                    st.listener().drawWarpWellEdge(pos + Point(-wwx,  wwy), RendererListener::North);
                    st.listener().drawWarpWellEdge(pos + Point( wwx, -wwy), RendererListener::South);
                    st.listener().drawWarpWellEdge(pos + Point( wwx,  wwy), RendererListener::North);
                    st.listener().drawWarpWellEdge(pos + Point(-wwy,  wwx), RendererListener::West);
                    st.listener().drawWarpWellEdge(pos + Point(-wwy, -wwx), RendererListener::West);
                    st.listener().drawWarpWellEdge(pos + Point( wwy,  wwx), RendererListener::East);
                    st.listener().drawWarpWellEdge(pos + Point( wwy, -wwx), RendererListener::East);
                    --wwx;
                }

                // We can make this step successfully; draw the sides.
                ++wwy;
                st.listener().drawWarpWellEdge(pos + Point(-wwx, -wwy), RendererListener::West);
                st.listener().drawWarpWellEdge(pos + Point(-wwx,  wwy), RendererListener::West);
                st.listener().drawWarpWellEdge(pos + Point( wwx, -wwy), RendererListener::East);
                st.listener().drawWarpWellEdge(pos + Point( wwx,  wwy), RendererListener::East);
                st.listener().drawWarpWellEdge(pos + Point(-wwy, -wwx), RendererListener::South);
                st.listener().drawWarpWellEdge(pos + Point(-wwy,  wwx), RendererListener::North);
                st.listener().drawWarpWellEdge(pos + Point( wwy, -wwx), RendererListener::South);
                st.listener().drawWarpWellEdge(pos + Point( wwy,  wwx), RendererListener::North);
            }
        } else {
            // Just a plain rectangle
            for (int i = -range; i <= range; ++i) {
                st.listener().drawWarpWellEdge(pos + Point(i, -range), RendererListener::South);
                st.listener().drawWarpWellEdge(pos + Point(i,  range), RendererListener::North);
                st.listener().drawWarpWellEdge(pos + Point(-range, i), RendererListener::West);
                st.listener().drawWarpWellEdge(pos + Point( range, i), RendererListener::East);
            }
        }
    }
}

/* Render ships: dot icons (if enabled), labels. */
void
game::map::Renderer::renderShips(const State& st) const
{
    // ex GChartViewport::drawShips, sort-of
    const Configuration& config = m_viewport.mapConfiguration();
    AnyShipType ships(m_viewport.universe().ships());
    for (Id_t i = ships.findNextIndex(0); i != 0; i = ships.findNextIndex(i)) {
        if (Ship* s = ships.getObjectByIndex(i)) {
            Point shipPosition;
            int shipOwner;
            if (s->getPosition(shipPosition) && s->getOwner(shipOwner)) {
                bool atPlanet = AnyPlanetType(m_viewport.universe().planets()).findNextObjectAt(shipPosition, 0, false) != 0;

                String_t label;
                if (m_viewport.hasOption(Viewport::ShowLabels)) {
                    if (const LabelExtra* ex = m_viewport.labels()) {
                        label = ex->shipLabels().getLabel(i);
                    }
                }

                for (int img = st.getFirstImage(); img >= 0; img = st.getNextImage(img)) {
                    renderShip(st, *s, shipPosition, shipOwner, atPlanet, label);
                }
                if (config.getMode() == Configuration::Circular) {
                    Point imgPos;
                    if (config.getPointAlias(shipPosition, imgPos, 1, true)) {
                        renderShip(st, *s, imgPos, shipOwner, atPlanet, label);
                    }
                }
            }
        }
    }
}

/* Render single ship. */
void
game::map::Renderer::renderShip(const State& st, const Ship& ship, Point shipPosition, int shipOwner, bool atPlanet, const String_t& label) const
{
    // ex GChartViewport::drawShipMarker
    // If not at planet, and draw risShowDot.
    const TeamSettings::Relation rel = m_viewport.teamSettings().getPlayerRelation(shipOwner);

    // If not at planet, and configured, draw risShowDot.
    // risShowIcon and risFleetLeader are drawn in renderShipExtras.
    int flag = 0;
    if (!atPlanet && m_viewport.hasOption(Viewport::ShowShipDots)) {
        flag |= RendererListener::risShowDot;
    }

    // If label present, draw it.
    if ((flag != 0 && m_viewport.containsCircle(shipPosition, 1))
        || (m_viewport.containsText(shipPosition, label)))
    {
        st.listener().drawShip(shipPosition, ship.getId(), rel, flag | RendererListener::risAtPlanet, label);
    }
}

/* Get flags for a planet.
   \param planet  Planet to check
   \param pos     Planet position
   \return pair of flags (ripXxx), is-marked flag */
std::pair<int,bool>
game::map::Renderer::getPlanetFlags(const Planet& planet, Point pos) const
{
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
    AnyShipType ships(m_viewport.universe().ships());
    for (Id_t sid = ships.findNextObjectAt(pos, 0, false); sid != 0; sid = ships.findNextObjectAt(pos, sid, false)) {
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

    return std::make_pair(flags, marked);
}
