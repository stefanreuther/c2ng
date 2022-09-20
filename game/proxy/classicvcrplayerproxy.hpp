/**
  *  \file game/proxy/classicvcrplayerproxy.hpp
  *  \brief Class game::proxy::ClassicVcrPlayerProxy
  */
#ifndef C2NG_GAME_PROXY_CLASSICVCRPLAYERPROXY_HPP
#define C2NG_GAME_PROXY_CLASSICVCRPLAYERPROXY_HPP

#include "game/proxy/vcrdatabaseadaptor.hpp"
#include "game/vcr/classic/types.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/stringinstructionlist.hpp"

namespace game { namespace proxy {

    /** Proxy for classic (1:1) VCR playback.
        Proxies a game::vcr::classic::EventVisualizer and a game::vcr::classic::EventRecorder
        to stream a sequence of events from game to UI.

        All requests will answer with a response package, containing a list of events.
        All requests and responses are asynchronous.

        To play a fight,
        - construct ClassicVcrPlayerProxy
        - call initRequest() to select a fight and retrieve first events
        - as long as the fight proceeds, call eventRequest() to retrieve further events
        - to jump within the fight, call jumpRequest();
          this will answer with an event package containing events starting at the given time. */
    class ClassicVcrPlayerProxy {
     public:
        /** Constructor.
            \param sender Access to combat
            \param recv   RequestDispatcher to receive updates in this thread */
        ClassicVcrPlayerProxy(util::RequestSender<VcrDatabaseAdaptor> sender, util::RequestDispatcher& recv);

        /** Destructor. */
        ~ClassicVcrPlayerProxy();

        /** Initialize.
            Start playback of a fight, selected by index.
            Answers with a sig_event with the initial events.
            Initial events will mainly set up units, but not yet fight.
            \param index Index (0-based); use VcrDatabaseProxy to determine maximum */
        void initRequest(size_t index);

        /** Send more events.
            Answers with a sig_event with subsequent events. */
        void eventRequest();

        /** Jump to a new location.
            Answers with a sig_event with events starting at the given time.
            \param time Time */
        void jumpRequest(game::vcr::classic::Time_t time);

        /** Signal: events.
            Reports battle progress in response to initRequest(), eventRequest(), jumpRequest().
            \param events Instructions to replay
            \param end    true if this is the last batch of events */
        afl::base::Signal<void(util::StringInstructionList&, bool)> sig_event;

        /** Signal: error.
            Reports a textual error.
            After this event, a single sig_event with end=true will be reported, but no further sig_event callbacks.
            \param msg Human-readable error message */
        afl::base::Signal<void(String_t)> sig_error;

     private:
        class Trampoline;
        class TrampolineFromAdaptor;

        util::RequestReceiver<ClassicVcrPlayerProxy> m_reply;
        util::RequestSender<Trampoline> m_request;
    };

} }

#endif
