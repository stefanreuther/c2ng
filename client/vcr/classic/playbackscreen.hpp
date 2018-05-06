/**
  *  \file client/vcr/classic/playbackscreen.hpp
  */
#ifndef C2NG_CLIENT_VCR_CLASSIC_PLAYBACKSCREEN_HPP
#define C2NG_CLIENT_VCR_CLASSIC_PLAYBACKSCREEN_HPP

#include <queue>
#include "afl/sys/loglistener.hpp"
#include "client/vcr/classic/event.hpp"
#include "client/vcr/classic/eventconsumer.hpp"
#include "client/vcr/classic/player.hpp"
#include "client/vcr/classic/renderer.hpp"
#include "client/vcr/classic/scheduler.hpp"
#include "client/vcr/playbackcontrolwidget.hpp"
#include "client/vcr/unitstatuswidget.hpp"
#include "game/session.hpp"
#include "game/vcr/classic/types.hpp"
#include "ui/root.hpp"
#include "ui/widgets/spritewidget.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/slaverequestsender.hpp"
#include "util/stringinstructionlist.hpp"

namespace client { namespace vcr { namespace classic {

    class PlaybackScreen : private PlayerListener, private EventConsumer {
     public:
        PlaybackScreen(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, size_t index, afl::sys::LogListener& log);
        ~PlaybackScreen();

        int run();

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::Session> m_gameSender;
        util::RequestReceiver<PlayerListener> m_reply;
        util::SlaveRequestSender<game::Session, Player> m_playerSender;
        size_t m_index;
        afl::sys::LogListener& m_log;

        // PlayerListener:
        virtual void handleEvents(util::StringInstructionList& list, bool finish);

        void preloadImages();
        void requestEvents();
        void requestJump(int32_t time);

        ui::widgets::SpriteWidget m_spriteWidget;
        UnitStatusWidget m_leftStatus;
        UnitStatusWidget m_rightStatus;
        PlaybackControlWidget m_playbackControl;
        std::auto_ptr<Renderer> m_renderer;

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
        int32_t m_targetTime;
        std::auto_ptr<Scheduler> m_scheduler;

        afl::base::Ref<gfx::Timer> m_timer;

        int m_ticksPerBattleCycle;
        int m_ticks;
        int m_tickInterval;

        enum PlayState {
            Paused,
            Playing
        };
        PlayState m_playState;

        virtual void placeObject(game::vcr::classic::Side side, const game::vcr::classic::EventListener::UnitInfo& info);
        virtual void pushEvent(client::vcr::classic::Event e);
        virtual void removeAnimations(int32_t id);

        std::queue<Event> m_events;
        int32_t m_currentTime;  // Time we're showing
        int32_t m_queuedTime;   // Last time in event list

        void onTogglePlay();
        void onMoveToBeginning();
        void onMoveBy(int delta);
        void onMoveToEnd();

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
