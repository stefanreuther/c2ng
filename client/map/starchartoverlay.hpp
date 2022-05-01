/**
  *  \file client/map/starchartoverlay.hpp
  */
#ifndef C2NG_CLIENT_MAP_STARCHARTOVERLAY_HPP
#define C2NG_CLIENT_MAP_STARCHARTOVERLAY_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/string/translator.hpp"
#include "client/dialogs/newdrawing.hpp"
#include "client/map/overlay.hpp"
#include "game/map/rangeset.hpp"
#include "ui/root.hpp"

namespace client { namespace map {

    class Location;
    class Screen;

    /** Map widget overlay for starchart screen. */
    class StarchartOverlay : public Overlay {
     public:
        StarchartOverlay(ui::Root& root,
                         afl::string::Translator& tx,
                         Location& loc,
                         Screen& scr);

        // Overlay:
        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);

        // EventConsumer:
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        Location& m_location;
        Screen& m_screen;

        bool m_drawingTagFilterActive;
        util::Atom_t m_drawingTagFilter;
        String_t m_drawingTagFilterName;

        game::map::Point m_cursorPosition;
        gfx::Rectangle m_cursorArea;
        int m_cursorPhase;

        std::auto_ptr<game::map::RangeSet> m_visRange;

        afl::base::SignalConnection conn_objectChange;
        afl::base::SignalConnection conn_positionChange;
        afl::base::SignalConnection conn_effectTimer;

        void onChange();
        void onEffectTimer();

        void editDrawingTagFilter();
        void editMarkerColor();
        void startDrawing();
        void createMarker(const client::dialogs::NewDrawingInfo& info);
        void startMarkRange();
        void editMarkerTag();
        void editMarkerComment();
        void startMovingMarker();
        void startDeleting();
        void startDistance();
        void editVisibilityRange();
        void moveInsideOut();

        void setDrawingTagFilter(util::Atom_t tag, String_t tagName);
        void clearDrawingTagFilter();
    };

} }

#endif
