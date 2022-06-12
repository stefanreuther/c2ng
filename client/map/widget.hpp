/**
  *  \file client/map/widget.hpp
  */
#ifndef C2NG_CLIENT_MAP_WIDGET_HPP
#define C2NG_CLIENT_MAP_WIDGET_HPP

#include "client/map/callback.hpp"
#include "client/map/renderer.hpp"
#include "game/map/renderlist.hpp"
#include "game/proxy/maprendererproxy.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"

namespace client { namespace map {

    class Overlay;

    class Widget : public ui::SimpleWidget, public Callback {
     public:
        enum Mode {
            NormalMode,
            ScannerMode
        };

        Widget(util::RequestSender<game::Session> gameSender, ui::Root& root, gfx::Point preferredSize);
        ~Widget();

        void setCenter(game::map::Point pt);

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        // Callback:
        virtual void removeOverlay(Overlay& over);
        virtual void requestRedraw();
        virtual void requestRedraw(gfx::Rectangle& area);

        void setMode(Mode m);
        void addOverlay(Overlay& over);
        void setZoomToInclude(game::map::Point pt);
        void zoomIn();
        void zoomOut();
        void setZoom(int mult, int divi);
        void toggleOptions(game::map::RenderOptions::Options_t opts);
        void setDrawingTagFilter(util::Atom_t tag);
        void clearDrawingTagFilter();
        void setShipTrailId(game::Id_t id);

        const Renderer& renderer() const;

        ui::Root& root();

     private:
        void onUpdate(afl::base::Ptr<game::map::RenderList> renderList);
        void maybeRequestNewRange();
        void updateModeConfiguration(bool force);

        Renderer m_renderer;
        game::proxy::MapRendererProxy m_proxy;
        ui::Root& m_root;
        gfx::Point m_preferredSize;

        Mode m_mode;
        game::map::RenderOptions::Area m_currentConfigurationArea;

        game::map::Point m_min;
        game::map::Point m_max;

        std::vector<Overlay*> m_overlays;
    };

} }

#endif
