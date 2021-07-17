/**
  *  \file client/vcr/flak/playbackscreen.cpp
  */

#include "client/vcr/flak/playbackscreen.hpp"
#include "afl/string/format.hpp"
#include "game/vcr/flak/eventrecorder.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/window.hpp"

using afl::string::Format;

namespace {
    const char* LOG_NAME = "client.vcr";
    const size_t GREEN_THRESHOLD = 20;
    const int MAX_SMOKE_AGE = 10;
}

client::vcr::flak::PlaybackScreen::PlaybackScreen(ui::Root& root, afl::string::Translator& tx,
                                                  util::RequestSender<game::proxy::VcrDatabaseAdaptor> adaptorSender,
                                                  size_t index, afl::sys::LogListener& log)
    : m_root(root),
      m_translator(tx),
      m_adaptorSender(adaptorSender),
      m_proxy(adaptorSender, root.engine().dispatcher()),
      m_index(index),
      m_log(log),
      m_timer(root.engine().createTimer()),
      m_visState(),
      m_visSettings(),
      m_arena(root, m_visState, m_visSettings),
      m_playbackControl(root),
      m_state(Initializing),
      m_queue(),
      m_tickInterval(50),
      m_playState(Playing)
{
    m_proxy.sig_event.add(this, &PlaybackScreen::onEvent);
    m_proxy.initRequest(index);
    m_playbackControl.sig_togglePlay.add(this, &PlaybackScreen::onTogglePlay);
    m_playbackControl.sig_moveToBeginning.add(this, &PlaybackScreen::onMoveToBeginning);
    m_playbackControl.sig_moveBy.add(this, &PlaybackScreen::onMoveBy);
    m_playbackControl.sig_moveToEnd.add(this, &PlaybackScreen::onMoveToEnd);
    m_timer->sig_fire.add(this, &PlaybackScreen::onTimer);

    m_visState.setMaxSmokeAge(MAX_SMOKE_AGE);

    updatePlayState();
}

client::vcr::flak::PlaybackScreen::~PlaybackScreen()
{ }

void
client::vcr::flak::PlaybackScreen::run()
{
    // FIXME: preliminary layout
    ui::Window win(m_translator("Visual Combat Recorder"), m_root.provider(), m_root.colorScheme(), ui::BLUE_BLACK_WINDOW, ui::layout::VBox::instance5);
    win.add(m_arena);

    ui::Group g(ui::layout::HBox::instance5);
    ui::Spacer spc;
    ui::widgets::Button btnClose(m_translator("Close"), util::Key_Escape, m_root);
    g.add(m_playbackControl);
    g.add(spc);
    g.add(btnClose);
    win.add(g);

    ui::EventLoop loop(m_root);
    btnClose.sig_fire.addNewClosure(loop.makeStop(0));

    win.setExtent(m_root.getExtent());
    m_root.add(win);
    loop.run();
}

const char*
client::vcr::flak::PlaybackScreen::toString(State st)
{
    switch (st) {
     case Initializing:  return "Initializing";
     // case Jumping:       return "Jumping";
     // case BeforeJumping: return "BeforeJumping";
     // case Forwarding:    return "Forwarding";
     case Red:           return "Red";
     case Yellow:        return "Yellow";
     case Green:         return "Green";
     case Draining:      return "Draining";
     case Finished:      return "Finished";
    }
    return "?";
}

void
client::vcr::flak::PlaybackScreen::onEvent(game::proxy::FlakVcrPlayerProxy::Result_t& events, bool finished)
{
    // Assimilate the events
    m_log.write(afl::sys::LogListener::Trace, LOG_NAME, Format("-> %d blocks, finish=%d", events.size(), int(finished)));
    for (size_t i = 0, n = events.size(); i < n; ++i) {
        m_queue.pushBackNew(events.extractElement(i));
    }

    // Process
    switch (m_state) {
     case Initializing:
     case Red:
        handleEventReceptionRed(finished);
        break;

     case Yellow:
     case Green:
     case Draining:  // No events expected in this state
     case Finished:  // No events expected in this state
        handleEventReceptionYellowGreen(finished);
        break;
    }
}

void
client::vcr::flak::PlaybackScreen::onTimer()
{
    if (m_playState == Playing) {
        bool ok;
        switch (m_state) {
         case Initializing:
         case Red:
         // case Jumping:
         // case BeforeJumping:
         // case Forwarding:
            break;

         case Yellow:
            playTick();
            if (m_queue.empty()) {
                setState(Red, "Underflow");
            } else {
                m_timer->setInterval(m_tickInterval);
            }
            break;

         case Green:
            playTick();
            if (m_queue.empty()) {
                // Buffer exhausted during playback. Request events and suspend playback.
                m_proxy.eventRequest();
                setState(Red, "Underflow");
            } else {
                // Playback succeeded. Request new events if needed
                if (m_queue.size() < GREEN_THRESHOLD) {
                    m_proxy.eventRequest();
                    setState(Yellow, "Underflow");
                }
                m_timer->setInterval(m_tickInterval);
            }
            break;

         case Draining:
            ok = playTick();
            if (!ok && m_queue.empty()) {
                setState(Finished, "Underflow");
            } else {
                m_timer->setInterval(m_tickInterval);
            }
            break;

         case Finished:
            break;
        }
    }
}

void
client::vcr::flak::PlaybackScreen::onTogglePlay()
{
    switch (m_playState) {
     case Playing: onPause(); break;
     case Paused:  onPlay();  break;
    }
}

void
client::vcr::flak::PlaybackScreen::onMoveToBeginning()
{
    // FIXME
}

void
client::vcr::flak::PlaybackScreen::onMoveBy(int delta)
{
    // FIXME
    (void) delta;
}

void
client::vcr::flak::PlaybackScreen::onMoveToEnd()
{
    // FIXME
}

void
client::vcr::flak::PlaybackScreen::onPlay()
{
    if (m_playState == Paused && m_state != Finished) {
        m_playState = Playing;
        updatePlayState();
        switch (m_state) {
         case Initializing:
         case Red:
         case Finished:
         // case Jumping:
         // case BeforeJumping:
         // case Forwarding:
            // Cannot play
            break;

         case Yellow:
         case Green:
         case Draining:
            // Execute a tick
            onTimer();
            break;
        }
    }
}

void
client::vcr::flak::PlaybackScreen::onPause()
{
    if (m_playState == Playing) {
        m_playState = Paused;
        updatePlayState();
        m_timer->setInterval(afl::sys::INFINITE_TIMEOUT);
    }
}

void
client::vcr::flak::PlaybackScreen::handleEventReceptionRed(bool finished)
{
    bool play;
    if (finished) {
        if (m_queue.empty()) {
            setState(Finished, "Events");
            play = false;
        } else {
            setState(Draining, "Events");
            play = true;
        }
    } else if (m_queue.size() < GREEN_THRESHOLD) {
        // Buffer not full enough yet; load more.
        m_proxy.eventRequest();
        setState(Red, "Events");
        play = false;
    } else {
        // Buffer sufficiently full.
        setState(Green, "Events");
        play = true;
    }

    if (play) {
        // Play events
        playTick();

        // Schedule next
        if (m_playState == Playing) {
            m_timer->setInterval(m_tickInterval);
        }
    }
}

void
client::vcr::flak::PlaybackScreen::handleEventReceptionYellowGreen(bool finished)
{
    // No need to start a timer because we're in Yellow/Green state where it is already active.
    if (finished) {
        if (m_queue.empty()) {
            setState(Finished, "Events");
        } else {
            setState(Draining, "Events");
        }
    } else if (m_queue.size() < GREEN_THRESHOLD) {
        m_proxy.eventRequest();
        setState(Yellow, "Events");
    } else {
        setState(Green, "Events");
    }
}

bool
client::vcr::flak::PlaybackScreen::playTick()
{
    bool result = false;
    if (!m_queue.empty()) {
        game::vcr::flak::EventRecorder rec;
        rec.swapContent(*m_queue.front());
        rec.replay(m_visState);
        m_queue.popFront();
        result = true;
    }
    if (m_visState.animate()) {
        result = true;
    }
    m_visSettings.updateCamera(m_visState); // FIXME handle result...
    m_arena.requestRedraw();
    return result;
}

void
client::vcr::flak::PlaybackScreen::updatePlayState()
{
    m_playbackControl.setPlayState(m_playState == Playing);
}

void
client::vcr::flak::PlaybackScreen::setState(State st, const char* why)
{
    m_log.write(afl::sys::LogListener::Trace, LOG_NAME,
                Format("%s -> %s (%s, t=%d, qsz=%d)")
                << toString(m_state)
                << toString(st)
                << why
                << 424242 /*m_currentTime*/
                << m_queue.size());
    m_state = st;
}

