/**
  *  \file client/map/renderer.hpp
  */
#ifndef C2NG_CLIENT_MAP_RENDERER_HPP
#define C2NG_CLIENT_MAP_RENDERER_HPP

#include "gfx/rectangle.hpp"
#include "game/map/point.hpp"
#include "game/map/renderlist.hpp"
#include "afl/base/ptr.hpp"
#include "ui/colorscheme.hpp"

namespace client { namespace map {

    class Renderer {
     public:
        Renderer();
        ~Renderer();

        void setExtent(const gfx::Rectangle& area);
        void setCenter(game::map::Point center);
        void setRenderList(afl::base::Ptr<game::map::RenderList> renderList);

        void draw(gfx::Canvas& can, ui::ColorScheme& colorScheme) const;

        void getPreferredWorldRange(game::map::Point& min, game::map::Point& max) const;
        void getMinimumWorldRange(game::map::Point& min, game::map::Point& max) const;

     private:
        class Listener;

        gfx::Rectangle m_area;
        afl::base::Ptr<game::map::RenderList> m_renderList;
        int m_zoomMultiplier;
        int m_zoomDivider;
        game::map::Point m_center;

        gfx::Point scale(game::map::Point pt) const;
        int scale(int r) const;
        int getCrossSize() const;
    };

} }

#endif
