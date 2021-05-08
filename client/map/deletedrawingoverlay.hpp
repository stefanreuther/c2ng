/**
  *  \file client/map/deletedrawingoverlay.hpp
  */
#ifndef C2NG_CLIENT_MAP_DELETEDRAWINGOVERLAY_HPP
#define C2NG_CLIENT_MAP_DELETEDRAWINGOVERLAY_HPP

#include "afl/string/translator.hpp"
#include "client/map/markeroverlaybase.hpp"
#include "gfx/timer.hpp"
#include "ui/root.hpp"

namespace client { namespace map {

    class Screen;

    class DeleteDrawingOverlay : public MarkerOverlayBase {
     public:
        DeleteDrawingOverlay(ui::Root& root,
                             afl::string::Translator& tx,
                             client::map::Screen& screen,
                             const game::map::Drawing& drawing);
        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

     private:
        bool m_phase;
        afl::base::Ref<gfx::Timer> m_timer;

        void onTimer();
    };


} }

#endif
