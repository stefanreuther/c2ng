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

    class Renderer {
     public:
        Renderer(Viewport& viewport);
        ~Renderer();

        void render(RendererListener& out);

     private:
        Viewport& m_viewport;

        void renderGrid(RendererListener& out);
        void renderMinefields(RendererListener& out);
        void renderPlanets(RendererListener& out);
        void renderPlanet(RendererListener& out, const Planet& planet, Point pos);
        void renderShips(RendererListener& out);
        void renderShipMarker(RendererListener& out, const Ship& ship, Point shipPosition);
    };

} }

#endif
