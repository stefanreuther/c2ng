/**
  *  \file client/map/proxy.hpp
  */
#ifndef C2NG_CLIENT_MAP_PROXY_HPP
#define C2NG_CLIENT_MAP_PROXY_HPP

#include "afl/base/signal.hpp"
#include "util/requestsender.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/slaverequestsender.hpp"
#include "game/session.hpp"
#include "game/map/viewport.hpp"
#include "game/map/renderlist.hpp"

namespace client { namespace map {

    class Proxy {
     public:
        // FIXME: This will re-render and update the observer whenever anything changes.
        // Add some way to combine these requests.
        Proxy(util::RequestSender<game::Session> gameSender,
              util::RequestDispatcher& dispatcher);
        ~Proxy();

        void setRange(game::map::Point min, game::map::Point max);
        void setOption(game::map::Viewport::Option option, bool flag);

        afl::base::Signal<void(afl::base::Ptr<game::map::RenderList> renderlist)> sig_update;

     private:
        util::RequestReceiver<Proxy> m_receiver;

        class Trampoline;
        util::SlaveRequestSender<game::Session, Trampoline> m_trampoline;
    };

} }

#endif
