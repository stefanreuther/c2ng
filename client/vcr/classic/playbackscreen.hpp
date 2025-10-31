/**
  *  \file client/vcr/classic/playbackscreen.hpp
  *  \brief Class client::vcr::classic::PlaybackScreen
  */
#ifndef C2NG_CLIENT_VCR_CLASSIC_PLAYBACKSCREEN_HPP
#define C2NG_CLIENT_VCR_CLASSIC_PLAYBACKSCREEN_HPP

#include <queue>
#include "afl/sys/loglistener.hpp"
#include "client/vcr/classic/renderer.hpp"
#include "client/vcr/configuration.hpp"
#include "client/vcr/playbackcontrolwidget.hpp"
#include "client/vcr/unitstatuswidget.hpp"
#include "game/proxy/classicvcrplayerproxy.hpp"
#include "game/session.hpp"
#include "game/vcr/classic/eventlistener.hpp"
#include "game/vcr/classic/scheduledevent.hpp"
#include "game/vcr/classic/scheduledeventconsumer.hpp"
#include "game/vcr/classic/types.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "ui/widgets/spritewidget.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/stringinstructionlist.hpp"

namespace client { namespace vcr { namespace classic {

    /** Classic VCR Playback screen. */
    class PlaybackScreen : private game::vcr::classic::ScheduledEventConsumer {
     public:
        /** Constructor.
            @param root          UI root
            @param tx            Translator
            @param adaptorSender Access to VCR database
            @param index         Index of battle to access
            @param confProxy     ConfigurationProxy instance
            @param log           Logger */
        PlaybackScreen(ui::Root& root, afl::string::Translator& tx,
                       util::RequestSender<game::proxy::VcrDatabaseAdaptor> adaptorSender,
                       size_t index,
                       game::proxy::ConfigurationProxy& confProxy,
                       afl::sys::LogListener& log);
        ~PlaybackScreen();

        /** Display and operate the playback screen. */
        void run();

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::proxy::VcrDatabaseAdaptor> m_adaptorSender;
        game::proxy::ClassicVcrPlayerProxy m_proxy;
        const size_t m_index;
        game::proxy::ConfigurationProxy& m_configProxy;
        afl::sys::LogListener& m_log;
        ui::EventLoop m_loop;

        Configuration m_config;
        ui::widgets::SpriteWidget m_spriteWidget;
        UnitStatusWidget m_leftStatus;
        UnitStatusWidget m_rightStatus;
        PlaybackControlWidget m_playbackControl;
        std::auto_ptr<Renderer> m_renderer;

        /* Global state machine.
           Each event identifies:
           - a request currently active on the proxy
           - whether we are waiting for an opportunity to continue playback (timer tick during "Playing" status) */
        enum State {
            /* Initializing: wait for initial placement of units.
               - request for initial events active */
            Initializing,
            /* Jumping: the next set of events we'll receive will be after a jump.
               - request for jump active
               - wait for continuation NOT active  */
            Jumping,
            /* Before jump: want to jump but previous request still active.
               - request for events, then request for jump active
               - wait for continuation NOT active */
            BeforeJumping,
            /* Forwarding: attempting to reach a specific time.
               - request for events active
               - wait for continuation NOT active */
            Forwarding,
            /* Red: buffer too short to play.
               - request for events active
               - wait for continuation NOT active */
            Red,
            /* Yellow: buffer good to play but draining.
               - request for events active
               - wait for continuation active, last event is a Wait command */
            Yellow,
            /* Green: buffer good to play.
               - request for events NOT active.
               - wait for continuation active, last event is a Wait command */
            Green,
            /* Draining: buffer is draining, provider signalled end of fight.
               - request for events NOT active.
               - wait for continuation active, last event is a Wait command */
            Draining,
            /* Finished: buffer is empty.
               - request for events NOT active.
               - wait for continuation NOT active */
            Finished
        };
        State m_state;

        /* When jumping, desired target time.
           Events are played invisibly until this time is reached. */
        int32_t m_targetTime;

        /* Event scheduler.
           Turns events into callbacks to placeObject/pushEvent/removeAnimations. */
        std::auto_ptr<game::vcr::classic::EventListener> m_scheduler;

        /* Timer according to parameters below. */
        afl::base::Ref<gfx::Timer> m_timer;

        /* Number of timer tick within a battle cycle.
           Counts up from 0 until Configuration::getNumTicksPerBattleCycle(). */
        int m_ticks;

        /* Playback status */
        enum PlayState {
            Paused,
            Playing
        };
        PlayState m_playState;

        /* Pending events */
        std::queue<game::vcr::classic::ScheduledEvent> m_events;
        int32_t m_currentTime;  // Time we're showing
        int32_t m_queuedTime;   // Last time in event list

        void handleError(String_t msg);
        void handleEvents(util::StringInstructionList& list, bool finish);

        void prepare();
        void requestEvents();
        void requestJump(int32_t time);

        virtual void placeObject(game::vcr::classic::Side side, const game::vcr::classic::EventListener::UnitInfo& info);
        virtual void pushEvent(game::vcr::classic::ScheduledEvent e);
        virtual void removeAnimations(int32_t from, int32_t to);

        void onTogglePlay();
        void onMoveToBeginning();
        void onMoveBy(int delta);
        void onMoveToEnd();
        void onChangeSpeed(bool faster);

        void jumpTo(int32_t t);
        void onPlay();
        void onPause();
        void onTick();

        bool executeEvents(int32_t timeLimit);
        void handleEventReceptionRed(bool finish);
        void handleEventReceptionYellowGreen(bool finish);
        void handleEventReceptionForwarding(bool finish);

        UnitStatusWidget& unitStatus(game::vcr::classic::Side side);

        static const char* toString(State st);
        void setState(State st, const char* why);
    };

} } }

#endif
