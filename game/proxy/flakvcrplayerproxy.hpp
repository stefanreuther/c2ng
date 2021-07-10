/**
  *  \file game/proxy/flakvcrplayerproxy.hpp
  *  \brief Class game::proxy::FlakVcrPlayerProxy
  */
#ifndef C2NG_GAME_PROXY_FLAKVCRPLAYERPROXY_HPP
#define C2NG_GAME_PROXY_FLAKVCRPLAYERPROXY_HPP

#include "afl/base/signal.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/proxy/vcrdatabaseadaptor.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/stringinstructionlist.hpp"

namespace game { namespace proxy {

    /** Proxy for FLAK VCR playback.
        Proxies a game::vcr::flak::Visualizer and a game::vcr::flak::EventRecorder
        to stream a sequence of events from game to UI.

        All requests will answer with a response package, containing a list of events.
        All requests and responses are asynchronous.

        To play a fight,
        - construct FlakVcrPlayerProxy
        - call initRequest() to select a fight and retrieve first events
        - as long as the fight proceeds, call eventRequest() to retrieve further events
        - to jump within the fight, call jumpRequest();
          this will answer with an event package containing events starting at the given time. */
    class FlakVcrPlayerProxy {
     public:
        typedef afl::container::PtrVector<util::StringInstructionList> Result_t;

        /** Constructor.
            \param sender Access to combat
            \param recv   RequestDispatcher to receive updates in this thread */
        FlakVcrPlayerProxy(util::RequestSender<VcrDatabaseAdaptor> sender, util::RequestDispatcher& recv);

        /** Destructor. */
        ~FlakVcrPlayerProxy();

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
            Answers with a sig_event with events starting at the given time (=contains an updateTime(time) request).
            If that time does not exist, answers with an empty response and result=false.
            Jumping to position 0 will restart the fight (with unit setup callbacks).
            \param time Time */
        void jumpRequest(int32_t time);

        /** Signal: result.
            \param result  List of events, grouped by time.
            \param stop    Stop flag: true if battle ends, false if more events can be retrieved. */
        afl::base::Signal<void(Result_t&, bool)> sig_event;

     private:
        class Trampoline;
        class TrampolineFromAdaptor;

        util::RequestReceiver<FlakVcrPlayerProxy> m_reply;
        util::RequestSender<Trampoline> m_request;
    };

} }


#endif
