/**
  *  \file game/map/renderer.hpp
  */
#ifndef C2NG_GAME_MAP_RENDERER_HPP
#define C2NG_GAME_MAP_RENDERER_HPP

#include "game/map/universe.hpp"

namespace game { namespace map {

    class Viewport;
    class RendererListener;
    class Planet;
    class Drawing;

    class Renderer {
     public:
        Renderer(Viewport& viewport);
        ~Renderer();

        void render(RendererListener& out);

     private:
        Viewport& m_viewport;

        void renderGrid(RendererListener& out);
        void renderMinefields(RendererListener& out);
        void renderDrawings(RendererListener& out);
        void renderDrawing(RendererListener& out, const Drawing& d);
        void renderPlanets(RendererListener& out);
        void renderPlanet(RendererListener& out, const Planet& planet, Point pos);
        void renderShips(RendererListener& out);
        void renderShipMarker(RendererListener& out, const Ship& ship, Point shipPosition);
    };

} }

#endif
