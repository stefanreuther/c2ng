/**
  *  \file client/map/markrangeoverlay.hpp
  *  \brief Class client::map::MarkRangeOverlay
  */
#ifndef C2NG_CLIENT_MAP_MARKRANGEOVERLAY_HPP
#define C2NG_CLIENT_MAP_MARKRANGEOVERLAY_HPP

#include "afl/string/translator.hpp"
#include "client/map/overlay.hpp"
#include "game/proxy/selectionproxy.hpp"
#include "ui/root.hpp"

namespace client { namespace map {

    class Location;
    class Screen;

    /** Starchart mode: mark a range. */
    class MarkRangeOverlay : public Overlay {
     public:
        MarkRangeOverlay(ui::Root& root, afl::string::Translator& tx, Location& loc, Screen& screen);
        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

     private:
        void onPositionChange(game::map::Point pt);
        void onNumObjectsInRange(int n);
        void rebuildSelection();

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        Location& m_location;
        Screen& m_screen;
        game::map::Point m_origin;
        game::map::Point m_end;
        game::proxy::SelectionProxy m_proxy;
        int m_numObjectsInRange;

        afl::base::SignalConnection conn_positionChange;
        afl::base::SignalConnection conn_numObjectsInRange;
    };


} }

#endif
