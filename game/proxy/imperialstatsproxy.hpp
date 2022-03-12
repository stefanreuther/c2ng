/**
  *  \file game/proxy/imperialstatsproxy.hpp
  *  \brief Class game::proxy::ImperialStatsProxy
  */
#ifndef C2NG_GAME_PROXY_IMPERIALSTATSPROXY_HPP
#define C2NG_GAME_PROXY_IMPERIALSTATSPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/map/info/types.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/stringlist.hpp"

namespace game { namespace proxy {

    /** Imperial Statistic Proxy.

        Provides bidirectional, asynchronous access to a game::map::info::Browser.
        For now, this will not send unsolicited updates.
        User must request updates whenever needed. */
    class ImperialStatsProxy {
     public:
        typedef game::map::info::Nodes_t Nodes_t;
        typedef game::map::info::PageOptions_t PageOptions_t;

        /** Constructor.
            @param gameSender  Game sender (to access game data)
            @param receiver    Receiver to receive replies */
        ImperialStatsProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver);

        /** Destructor. */
        ~ImperialStatsProxy();

        /** Request page content.
            Produces content for the given page, and returns a sig_pageContent signal.
            @param page Page
            @see game::map::info::Browser::renderPage() */
        void requestPageContent(game::map::info::Page page);

        /** Request page options.
            Produces list of options for the given page, and returns a sig_pageOptions signal.
            @param page Page
            @see game::map::info::Browser::renderPageOptions() */
        void requestPageOptions(game::map::info::Page page);

        /** Set page options for one page.
            This will not produce an update signal; call the desired request methods if you need one.
            @param page Page
            @param opts Options
            @see game::map::info::Browser::setPageOptions() */
        void setPageOptions(game::map::info::Page page, PageOptions_t opts);

        /** Signal: update of page content.
            Sent in response to requestPageContent().
            @param nodes Page content. Mutable so you can loot it. */
        afl::base::Signal<void(Nodes_t&)> sig_pageContent;

        /** Signal: update of page options.
            Sent in response to requestPageOptions().
            @param opts    List of options as value/label pair.
            @param current Current value. */
        afl::base::Signal<void(const util::StringList&, PageOptions_t)> sig_pageOptions;

     private:
        class Trampoline;
        class TrampolineFromSession;

        util::RequestReceiver<ImperialStatsProxy> m_receiver;
        util::RequestSender<Trampoline> m_sender;
    };

} }

#endif
