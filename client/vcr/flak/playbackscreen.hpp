/**
  *  \file client/vcr/flak/playbackscreen.hpp
  *  \brief Class client::vcr::flak::PlaybackScreen
  */
#ifndef C2NG_CLIENT_VCR_FLAK_PLAYBACKSCREEN_HPP
#define C2NG_CLIENT_VCR_FLAK_PLAYBACKSCREEN_HPP

#include "afl/container/ptrqueue.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "client/vcr/cameracontrolwidget.hpp"
#include "client/vcr/flak/arenawidget.hpp"
#include "client/vcr/playbackcontrolwidget.hpp"
#include "client/widgets/combatunitlist.hpp"
#include "game/proxy/flakvcrplayerproxy.hpp"
#include "game/proxy/vcrdatabaseadaptor.hpp"
#include "game/session.hpp"
#include "game/vcr/flak/visualisationsettings.hpp"
#include "game/vcr/flak/visualisationstate.hpp"
#include "gfx/keyeventconsumer.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace vcr { namespace flak {

    /** FLAK Playback Screen.
        Displays animated FLAK combat using a FlakVcrPlayerProxy. */
    class PlaybackScreen : public gfx::KeyEventConsumer {
     public:
        /** Constructor.
            \param root           UI root
            \param tx             Translator
            \param adaptorSender  Access to VCR database
            \param index          Index into VCR database
            \param log            Logger */
        PlaybackScreen(ui::Root& root, afl::string::Translator& tx,
                       util::RequestSender<game::proxy::VcrDatabaseAdaptor> adaptorSender,
                       size_t index, afl::sys::LogListener& log);
        ~PlaybackScreen();

        void run();

        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::proxy::VcrDatabaseAdaptor> m_adaptorSender;
        game::proxy::FlakVcrPlayerProxy m_proxy;
        size_t m_index;
        afl::sys::LogListener& m_log;
        afl::base::Ref<gfx::Timer> m_timer;

        game::PlayerArray<String_t> m_playerAdjectives;
        game::TeamSettings m_teamSettings;

        game::vcr::flak::VisualisationState m_visState;
        game::vcr::flak::VisualisationSettings m_visSettings;
        ArenaWidget m_arena;
        PlaybackControlWidget m_playbackControl;
        CameraControlWidget m_cameraControl;
        client::widgets::CombatUnitList m_unitList;

        enum State {
            /* Initializing: wait for initial placement of units.
               - request for initial events active */
            Initializing,
            /* Jumping: the next set of events we'll receive will be after a jump.
               - request for jump active  */
            Jumping,
            /* Before jump: want to jump but previous request still active.
               - request for events, then request for jump active */
            BeforeJumping,
            /* Forwarding: attempting to reach a specific time.
               - request for events active */
            Forwarding,
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

        int32_t m_targetTime;
        game::vcr::flak::VisualisationState m_shadowState;

        enum PlayState {
            Paused,
            Playing
        };
        PlayState m_playState;

        void loadEnvironment();

        void onEvent(game::proxy::FlakVcrPlayerProxy::Result_t& events, bool finished);
        void onTimer();
        void onTogglePlay();
        void onMoveToBeginning();
        void onMoveBy(int delta);
        void onMoveToEnd();
        void onPlay();
        void onPause();
        void onFollow();

        void handleEventReceptionInit(bool finished);
        void handleEventReceptionRed(bool finished);
        void handleEventReceptionYellowGreen(bool finished);
        bool playTick(bool initial);
        bool playShadow();
        void finishShadow();
        void jumpTo(int32_t time);
        void processJump(bool finished);

        void updatePlayState();
        void initList();
        void updateList();
        void updateCamera();
        void updateGrid();
        void updateFollowedFleet();
        void updateMode();

        void handleChanges(game::vcr::flak::VisualisationSettings::Changes_t ch);

        static const char* toString(State st);
        void setState(State st, const char* why);
    };

} } }

#endif
