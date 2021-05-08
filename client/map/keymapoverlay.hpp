/**
  *  \file client/map/keymapoverlay.hpp
  *  \brief Class client::map::KeymapOverlay
  */
#ifndef C2NG_CLIENT_MAP_KEYMAPOVERLAY_HPP
#define C2NG_CLIENT_MAP_KEYMAPOVERLAY_HPP

#include "client/map/overlay.hpp"
#include "game/proxy/keymapproxy.hpp"
#include "gfx/timer.hpp"

namespace client { namespace map {

    class Screen;

    /** Starchart keymap overlay.
        This overlay is used when a custom keymap is active.
        Essentially, this is a largely simplified version of the logic of client::si::KeymapHandler,
        which is used for regular UI contexts.
        For this one, the state tracking and script interface logic is performed by class Screen. */
    class KeymapOverlay : public Overlay, private game::proxy::KeymapProxy::Listener {
     public:
        /** Constructor.
            \param parent     Screen
            \param keymapName Keymap to use
            \param prefix     Prefix for keymap key */
        KeymapOverlay(Screen& parent, String_t keymapName, int prefix);
        ~KeymapOverlay();

        // Overlay:
        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

     private:
        // KeymapProxy::Listener:
        virtual void updateKeyList(util::KeySet_t& keys);
        void show();

        Screen& m_parent;
        String_t m_keymapName;
        int m_prefix;
        game::proxy::KeymapProxy m_proxy;
        util::KeySet_t m_keys;
        afl::base::Ref<gfx::Timer> m_timer;
        bool m_shown;
};

} }

#endif
