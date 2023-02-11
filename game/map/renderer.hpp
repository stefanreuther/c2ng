/**
  *  \file game/map/renderer.hpp
  *  \brief Class game::map::Renderer
  */
#ifndef C2NG_GAME_MAP_RENDERER_HPP
#define C2NG_GAME_MAP_RENDERER_HPP

#include "game/map/universe.hpp"

namespace game { namespace map {

    class Viewport;
    class RendererListener;
    class Planet;
    class Drawing;

    /** Map renderer.
        Enumerates all objects visible on a Viewport, and calls appropriate lower-level methods on a RendererListener.
        The idea is to map the RendererListener output directly to some drawing primitives, with minimal postprocessing.

        This class is responsible for all configuration processing.
        Although the configuration value "fill minefields" is only needed by the RendererListener (UI side),
        we process it here as well (game side) and hand the ready-made value to the listener;
        this way, we don't need a second path to transmit configuration.

        This class provides coarse clipping.
        The RendererListener still needs to clip themselves.

        This class provides basic layering ("this on top of that") by producing callbacks in correct order.
        A second callback is expected to draw atop the first one when using the same coordinates.
        This is especially relevant for ships, which can produce multiple callbacks
        (draw larger icons, draw planet, draw ship dot atop the planet rings).

        This class implements projections for wrapped map modes.
        If a unit appears in multiple images, it is rendered multiple times as appropriate. */
    class Renderer {
     public:
        /** Constructor.
            @param viewport Viewport */
        explicit Renderer(const Viewport& viewport);

        /** Render map.
            Renders the map section selected by the Viewport specified on construction
            into the given RendererListener.
            @param out Listener */
        void render(RendererListener& out) const;

     private:
        struct State;

        const Viewport& m_viewport;

        void renderGrid(const State& st) const;
        void renderRectangularGrid(const State& st) const;
        void renderCircularGrid(const State& st) const;
        void renderMinefields(const State& st) const;
        void renderUfos(const State& st) const;
        void renderIonStorms(const State& st) const;
        void renderDrawings(const State& st) const;
        void renderDrawing(const State& st, const Drawing& d) const;
        void renderShipExtras(const State& st) const;
        void renderShipTrail(const State& st, const Ship& sh, int shipOwner, int turnNumber) const;
        void renderShipVector(const State& st, const Ship& sh, int shipOwner) const;
        void renderPlanets(const State& st) const;
        void renderPlanet(const State& st, const Planet& planet, Point pos) const;
        void renderWarpWell(const State& st, Point pos) const;
        void renderShips(const State& st) const;
        void renderShip(const State& st, const Ship& ship, Point shipPosition, int shipOwner, bool atPlanet, const String_t& label) const;

        std::pair<int,bool> getPlanetFlags(const Planet& planet, Point pos) const;
    };

} }

#endif
