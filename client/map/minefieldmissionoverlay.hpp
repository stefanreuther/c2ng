/**
  *  \file client/map/minefieldmissionoverlay.hpp
  */
#ifndef C2NG_CLIENT_MAP_MINEFIELDMISSIONOVERLAY_HPP
#define C2NG_CLIENT_MAP_MINEFIELDMISSIONOVERLAY_HPP

#include "afl/string/translator.hpp"
#include "client/map/overlay.hpp"
#include "game/map/minefieldformula.hpp"
#include "game/proxy/objectobserver.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace map {

    class MinefieldMissionOverlay : public Overlay {
     public:
        MinefieldMissionOverlay(ui::Root& root, afl::string::Translator& tx);
        ~MinefieldMissionOverlay();

        void setEffects(game::map::MinefieldEffects_t data);

        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

        void attach(game::proxy::ObjectObserver& oop);

     private:
        game::map::MinefieldEffects_t m_data;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestReceiver<MinefieldMissionOverlay> m_reply;
    };

} }

#endif
