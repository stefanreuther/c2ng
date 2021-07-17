/**
  *  \file client/vcr/flak/playbackscreen.hpp
  */
#ifndef C2NG_CLIENT_VCR_FLAK_PLAYBACKSCREEN_HPP
#define C2NG_CLIENT_VCR_FLAK_PLAYBACKSCREEN_HPP

#include "afl/container/ptrqueue.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "client/vcr/flak/arenawidget.hpp"
#include "client/vcr/playbackcontrolwidget.hpp"
#include "game/proxy/flakvcrplayerproxy.hpp"
#include "game/proxy/vcrdatabaseadaptor.hpp"
#include "game/vcr/flak/visualisationsettings.hpp"
#include "game/vcr/flak/visualisationstate.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace vcr { namespace flak {

    class PlaybackScreen {
     public:
        PlaybackScreen(ui::Root& root, afl::string::Translator& tx,
                       util::RequestSender<game::proxy::VcrDatabaseAdaptor> adaptorSender,
                       size_t index, afl::sys::LogListener& log);
        ~PlaybackScreen();

        void run();

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::proxy::VcrDatabaseAdaptor> m_adaptorSender;
        game::proxy::FlakVcrPlayerProxy m_proxy;
        size_t m_index;
        afl::sys::LogListener& m_log;
        afl::base::Ref<gfx::Timer> m_timer;

        game::vcr::flak::VisualisationState m_visState;
        game::vcr::flak::VisualisationSettings m_visSettings;
        ArenaWidget m_arena;
        PlaybackControlWidget m_playbackControl;

        enum State {
            /* Initializing: wait for initial placement of units.
               - request for initial events active */
            Initializing,
            // /* Jumping: the next set of events we'll receive will be after a jump.
            //    - request for jump active  */
            // Jumping,
            // /* Before jump: want to jump but previous request still active.
            //    - request for events, then request for jump active */
            // BeforeJumping,
            // /* Forwarding: attempting to reach a specific time.
            //    - request for events active */
            // Forwarding,
            /* Red: buffer too short to play.
               - request for events active */
            Red,
            /* Yellow: buffer good to play but draining.
               - request for events active */
            Yellow,
            /* Green: buffer good to play.
               - request for events NOT active. */
            Green,
            /* Draining: buffer is draining, provider signalled end of fight.
               - request for events NOT active. */
            Draining,
            /* Finished: buffer is empty.
               - request for events NOT active. */
            Finished
        };
        State m_state;
        afl::container::PtrQueue<util::StringInstructionList> m_queue;

        int m_tickInterval;

        enum PlayState {
            Paused,
            Playing
        };
        PlayState m_playState;

        void onEvent(game::proxy::FlakVcrPlayerProxy::Result_t& events, bool finished);
        void onTimer();
        void onTogglePlay();
        void onMoveToBeginning();
        void onMoveBy(int delta);
        void onMoveToEnd();
        void onPlay();
        void onPause();

        void handleEventReceptionRed(bool finished);
        void handleEventReceptionYellowGreen(bool finished);
        bool playTick();
        void updatePlayState();

        static const char* toString(State st);
        void setState(State st, const char* why);
    };

} } }

#endif
