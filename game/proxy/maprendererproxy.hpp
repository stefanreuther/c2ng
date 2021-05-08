/**
  *  \file game/proxy/maprendererproxy.hpp
  */
#ifndef C2NG_GAME_PROXY_MAPRENDERERPROXY_HPP
#define C2NG_GAME_PROXY_MAPRENDERERPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/map/renderlist.hpp"
#include "game/map/viewport.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "game/map/renderoptions.hpp"

namespace game { namespace proxy {

    /** Asynchronous, bidirectional proxy for starchart rendering.
        This proxies a game::map::Viewport and game::map::Renderer.

        To use,
        - construct
        - observe sig_update to receive render lists
        - configure parameters; in particular, range */
    class MapRendererProxy {
     public:
        // FIXME: This will re-render and update the observer whenever anything changes.
        // Add some way to combine these requests.

        /** Constructor.
            \param gameSender Game sender
            \param dispatcher Dispatcher to receive replies on */
        MapRendererProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& dispatcher);

        /** Destructor. */
        ~MapRendererProxy();

        /** Set configuration.
            Selects the configuration area that will be used for rendering.
            \param area Area; default is Normal */
        void setConfiguration(game::map::RenderOptions::Area area);

        /** Set game coordinate range.
            \param min Minimum X/Y (inclusive)
            \param max Maximum X/Y (inclusive)
            \see game::map::Viewport::setRange */
        void setRange(game::map::Point min, game::map::Point max);

        /** Toggle rendering options.
            \param opts Options to toggle
            \see game::map::RenderOptions::toggleOptions, game::map::Viewport::setOptions */
        void toggleOptions(game::map::RenderOptions::Options_t opts);

        /** Set drawing tag filter.
            \param tag Show only drawings with this tag.
            \see game::map::Viewport::setDrawingTagFilter */
        void setDrawingTagFilter(util::Atom_t tag);

        /** Clear drawing tag filter.
            All drawings will be shown.
            \see game::map::Viewport::clearDrawingTagFilter */
        void clearDrawingTagFilter();

        /** Signal: render list update.
            Invoked upon every change to the universe (Session::notifyListeners()) or rendering parameters.
            \param renderlist newly-created RenderList instance containing current starchart content */
        afl::base::Signal<void(afl::base::Ptr<game::map::RenderList> renderlist)> sig_update;

     private:
        util::RequestReceiver<MapRendererProxy> m_receiver;

        class Trampoline;
        class TrampolineFromSession;
        util::RequestSender<Trampoline> m_trampoline;
    };

} }

#endif
