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
#include "util/requestsender.hpp"

namespace client { namespace map {

    class MovementOverlay : public Overlay {
     public:
        /* PCC2 flag mappings is bonkers.
           sc_KeyboardMode:     Movement, Config - never used
           sc_Keyboard:         Movement
           sc_StyleChange:      Config
           sc_Zoomable:         tbd
           sc_ShipsOnly:        not implemented */

        enum Mode {
            /** Accept normal (movement) keys. */
            AcceptMovementKeys,

            /** Accept configuration keys. */
            AcceptConfigKeys
        };
        typedef afl::bits::SmallSet<Mode> Modes_t;

        MovementOverlay(util::RequestDispatcher& disp, util::RequestSender<game::Session> gameSender);
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

        afl::base::Signal<void(game::map::Point)> sig_move;
        afl::base::Signal<void(game::map::Point)> sig_doubleClick;

     private:
        util::RequestSender<game::Session> m_gameSender;
        game::proxy::LockProxy m_lockProxy;
        Modes_t m_modes;

        bool m_valid;
        game::map::Point m_position;

        void moveBy(int dx, int dy, const Renderer& ren);
        void moveTo(game::map::Point pt, const Renderer& ren);
        void lockItem(game::map::Point target, bool left, bool markedOnly, bool optimizeWarp, const Renderer& ren);

        void onLockResult(game::map::Point result);
    };

} }

#endif
