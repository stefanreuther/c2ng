/**
  *  \file ui/widgets/tiledpanel.hpp
  */
#ifndef C2NG_UI_WIDGETS_TILEDPANEL_HPP
#define C2NG_UI_WIDGETS_TILEDPANEL_HPP

#include "ui/layoutablegroup.hpp"
#include "gfx/canvas.hpp"
#include "ui/colorscheme.hpp"
#include "gfx/resourceprovider.hpp"
#include "afl/base/signalconnection.hpp"

namespace ui { namespace widgets {

    class TiledPanel : public LayoutableGroup {
     public:
        TiledPanel(gfx::ResourceProvider& provider,
                   ui::ColorScheme& scheme,
                   ui::layout::Manager& mgr);
        ~TiledPanel();

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        // LayoutableGroup:
        virtual gfx::Rectangle transformSize(gfx::Rectangle size, Transformation kind) const;

     private:
        gfx::ResourceProvider& m_resourceProvider;
        ui::ColorScheme& m_colorScheme;
        afl::base::SignalConnection conn_providerImageChange;
        afl::base::Ptr<gfx::Canvas> m_tile;

        void onImageChange();
        
    };

} }

#endif
