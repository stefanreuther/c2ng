/**
  *  \file client/map/markeroverlaybase.hpp
  */
#ifndef C2NG_CLIENT_MAP_MARKEROVERLAYBASE_HPP
#define C2NG_CLIENT_MAP_MARKEROVERLAYBASE_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/string/translator.hpp"
#include "client/map/overlay.hpp"
#include "game/proxy/drawingproxy.hpp"
#include "ui/root.hpp"

namespace client { namespace map {

    class Screen;

    /** Base class for a starchart screen overlay for manipulating a marker. */
    class MarkerOverlayBase : public Overlay {
     public:
        typedef game::proxy::DrawingProxy::Status_t Status_t;

        MarkerOverlayBase(ui::Root& root, afl::string::Translator& tx, Screen& screen, const game::map::Drawing& drawing);

        const game::map::Drawing& drawing() const;
        Screen& screen() const;
        ui::Root& root() const;
        afl::string::Translator& translator() const;

     protected:
        bool defaultHandleKey(util::Key_t key, int prefix, const Renderer& ren);

     private:
        void onDrawingUpdate(const Status_t& st);
        void editColor();
        void editTag();

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        Screen& m_screen;
        game::map::Drawing m_drawing;
        afl::base::SignalConnection conn_drawingUpdate;
    };

} }

#endif
