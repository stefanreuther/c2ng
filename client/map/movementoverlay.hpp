/**
  *  \file client/map/movementoverlay.hpp
  */
#ifndef C2NG_CLIENT_MAP_MOVEMENTOVERLAY_HPP
#define C2NG_CLIENT_MAP_MOVEMENTOVERLAY_HPP

#include "afl/base/signal.hpp"
#include "afl/bits/smallset.hpp"
#include "client/map/overlay.hpp"
#include "game/map/point.hpp"
#include "game/proxy/lockproxy.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "ui/tooltip.hpp"
#include "util/requestsender.hpp"

namespace client { namespace map {

    class Widget;

    class MovementOverlay : public Overlay {
     public:
        /* PCC2 flag mappings is bonkers.
           sc_KeyboardMode:     Movement, Config - never used
           sc_Keyboard:         Movement
           sc_StyleChange:      Config
           sc_Zoomable:         Zoom
           sc_ShipsOnly:        not implemented */

        enum Mode {
            /** Accept normal (movement) keys. */
            AcceptMovementKeys,

            /** Accept configuration keys. */
            AcceptConfigKeys,

            /** Accept zoom keys. */
            AcceptZoomKeys
        };
        typedef afl::bits::SmallSet<Mode> Modes_t;

        MovementOverlay(util::RequestDispatcher& disp, util::RequestSender<game::Session> gameSender, Widget& parent, afl::string::Translator& tx);
        ~MovementOverlay();

        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);

        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

        void setMode(Mode mode, bool enable);

        void setPosition(game::map::Point pt);
        void clearPosition();
        bool getPosition(game::map::Point& out) const;

        void setLockOrigin(game::map::Point pt, bool isHyperdriving, game::Id_t shipId);

        /** Perform keyboard movement mode.
            This gives the containing Widget exclusive focus,
            and lets the user move the pointer using the keyboard or the mouse.
            This function returns when the user exits keyboard mode.

            \param ren Renderer */
        void doKeyboardMode(const Renderer& ren);

        afl::base::Signal<void(game::map::Point)> sig_move;
        afl::base::Signal<void(game::map::Point)> sig_doubleClick;

     private:
        util::RequestSender<game::Session> m_gameSender;
        game::proxy::LockProxy m_lockProxy;
        Widget& m_parent;
        afl::string::Translator& m_translator;
        Modes_t m_modes;
        ui::Tooltip m_toolTip;

        bool m_keyboardMode;
        bool m_keyboardAdviceOnTop;
        bool m_valid;
        game::map::Point m_position;

        gfx::Point m_hoveredPoint;

        void moveBy(int dx, int dy, const Renderer& ren);
        void moveTo(game::map::Point pt, const Renderer& ren);
        void lockItem(game::map::Point target, bool left, bool markedOnly, bool optimizeWarp, const Renderer& ren);
        void configureLockProxy(const Renderer& ren);

        void onLockResult(game::map::Point result);
        void onUnitNameResult(game::map::Point result, String_t names);
        void onHover(gfx::Point pt);
    };

} }

#endif
