/**
  *  \file client/vcr/flak/playbackscreen.cpp
  *  \brief Class client::vcr::flak::PlaybackScreen
  */

#include "client/vcr/flak/playbackscreen.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/widgets/decayingmessage.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/playerproxy.hpp"
#include "game/proxy/vcrdatabaseproxy.hpp"
#include "game/vcr/flak/eventrecorder.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/skincolorscheme.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/panel.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"

using afl::string::Format;
using client::widgets::CombatUnitList;
using game::config::UserConfiguration;
using game::vcr::flak::VisualisationSettings;
using game::vcr::flak::VisualisationState;

/*
 *  This implements a state-machine to receive event blocks from the game side.
 *  This allows us smooth playback even if the game side takes time to produce data
 *  (e.g. by being blocked through something else).
 *
 *  FIXME: for now, FF/REW is very inefficient as we ALWAYS rewind to the beginning.
 *  Fixing that requires implementation of checkpoints on game side (Algorithm::StatusToken)
 *  as well as on UI side (copies of VisualisationState).
 *  In general, VisualisationState being processed on the UI side means we actually
 *  do some heavy lifting here, but that can hardly be avoided.
 */

namespace {
    // Logger name
    const char* LOG_NAME = "client.vcr";

    // Need at least this many event packages to be green; if we have fewer, request more
    const size_t GREEN_THRESHOLD = 20;

    // Maximum age of smoke
    const int MAX_SMOKE_AGE = 10;

    // Maximum time
    const int32_t MAX_TIME = 0x7FFFFFFF;

    // Movement amount for manual camera movement
    const float MOVE = 1/128.0f;
}

client::vcr::flak::PlaybackScreen::PlaybackScreen(ui::Root& root, afl::string::Translator& tx,
                                                  util::RequestSender<game::proxy::VcrDatabaseAdaptor> adaptorSender,
                                                  size_t index,
                                                  util::RequestSender<game::Session> gameSender,
                                                  afl::sys::LogListener& log)
    : m_root(root),
      m_translator(tx),
      m_adaptorSender(adaptorSender),
      m_proxy(adaptorSender, root.engine().dispatcher()),
      m_index(index),
      m_gameSender(gameSender),
      m_log(log),
      m_timer(root.engine().createTimer()),
      m_playerAdjectives(),
      m_teamSettings(),
      m_config(),
      m_visState(),
      m_visSettings(),
      m_arena(root, m_visState, m_visSettings),
      m_playbackControl(root, false),
      m_cameraControl(root, tx),
      m_unitList(root),
      m_state(Initializing),
      m_queue(),
      m_playState(Playing)
{
    m_proxy.sig_event.add(this, &PlaybackScreen::onEvent);
    m_playbackControl.sig_togglePlay.add(this, &PlaybackScreen::onTogglePlay);
    m_playbackControl.sig_moveToBeginning.add(this, &PlaybackScreen::onMoveToBeginning);
    m_playbackControl.sig_moveBy.add(this, &PlaybackScreen::onMoveBy);
    m_playbackControl.sig_moveToEnd.add(this, &PlaybackScreen::onMoveToEnd);
    m_playbackControl.sig_changeSpeed.add(this, &PlaybackScreen::onChangeSpeed);
    m_timer->sig_fire.add(this, &PlaybackScreen::onTimer);
    m_cameraControl.dispatchKeysTo(*this);

    m_visState.setMaxSmokeAge(MAX_SMOKE_AGE);

    updatePlayState();
    updateCamera();
    updateGrid();
    updateFollowedFleet();
    updateMode();
}

client::vcr::flak::PlaybackScreen::~PlaybackScreen()
{ }

void
client::vcr::flak::PlaybackScreen::run()
{
    // Load environment required for later rendering
    loadEnvironment();

    // Start up state machine after initialisation
    m_proxy.initRequest(m_index);

    // Panel [HBox, no padding so arena uses full monitor height]
    //   Arena
    //   Panel [VBox, default padding to get nice frame]
    //     Unit List
    //     Camera Control
    //     HBox (Spacer, Playback Control, Spacer)
    //     HBox (Close)
    afl::base::Deleter del;
    ui::EventLoop loop(m_root);

    ui::widgets::Panel& win = del.addNew(new ui::widgets::Panel(ui::layout::HBox::instance0, 0));
    win.setColorScheme(del.addNew(new ui::SkinColorScheme(ui::DARK_COLOR_SET, m_root.colorScheme())));
    win.setState(ui::Widget::ModalState, true);

    win.add(m_arena);

    ui::widgets::Panel& g11 = del.addNew(new ui::widgets::Panel(ui::layout::VBox::instance5, 5));
    g11.add(del.addNew(new ui::widgets::ScrollbarContainer(m_unitList, m_root)));
    g11.add(m_cameraControl);

    ui::Group& g114 = del.addNew(new ui::Group(ui::layout::HBox::instance0));
    g114.add(del.addNew(new ui::Spacer()));
    g114.add(m_playbackControl);
    g114.add(del.addNew(new ui::Spacer()));
    g11.add(g114);

    client::widgets::HelpWidget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:flak"));
    ui::Group& g115 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::Button& btnHelp = del.addNew(new ui::widgets::Button(m_translator("Help"), 'h', m_root));
    ui::widgets::Button& btnClose = del.addNew(new ui::widgets::Button(m_translator("Close"), util::Key_Escape, m_root));
    g115.add(del.addNew(new ui::Spacer()));
    g115.add(btnHelp);
    g115.add(btnClose);
    g11.add(g115);

    win.add(g11);
    win.add(del.addNew(new ui::widgets::KeyForwarder(*this)));
    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
    win.add(help);

    btnClose.sig_fire.addNewClosure(loop.makeStop(0));
    btnHelp.dispatchKeyTo(help);
    m_unitList.requestFocus();

    win.setExtent(m_root.getExtent());
    m_root.add(win);
    loop.run();
}

bool
client::vcr::flak::PlaybackScreen::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex FlakPlayWidget::handleEvent
    switch (key) {
        // FIXME: '1' = observeLeft, '2' = observeRight

     case '3':
     case util::Key_Tab:
        m_config.toggleFlakRendererMode(UserConfiguration::ThreeDMode, UserConfiguration::FlatMode);
        updateMode();
        updateConfig();
        return true;

     case 'a':
        handleChanges(m_visSettings.zoomIn());
        return true;

     case 'c':
        handleChanges(m_visSettings.toggleAutoCamera());
        return true;

     case 'f':
        onFollow();
        return true;

     case 'g':
        m_config.toggleFlakGrid();
        updateGrid();
        updateConfig();
        return true;

     case 'y':
     case 'z':
        handleChanges(m_visSettings.zoomOut());
        return true;

     case util::Key_Up + util::KeyMod_Shift:
        handleChanges(m_visSettings.move(MOVE, 0));
        return true;

     case util::Key_Down + util::KeyMod_Shift:
        handleChanges(m_visSettings.move(-MOVE, 0));
        return true;

     case util::Key_Left + util::KeyMod_Shift:
        handleChanges(m_visSettings.move(0, -MOVE));
        return true;

     case util::Key_Right + util::KeyMod_Shift:
        handleChanges(m_visSettings.move(0, MOVE));
        return true;

     default:
        return false;
    }
}

void
client::vcr::flak::PlaybackScreen::loadEnvironment()
{
    // Settings
    Downlink link(m_root, m_translator);
    game::proxy::VcrDatabaseProxy proxy(m_adaptorSender, m_root.engine().dispatcher(), m_translator, std::auto_ptr<game::spec::info::PictureNamer>());
    proxy.getTeamSettings(link, m_teamSettings);
    m_playerAdjectives = proxy.getPlayerNames(link, game::Player::AdjectiveName);

    // Configuration
    game::proxy::ConfigurationProxy configProxy(m_gameSender);
    m_config.load(link, configProxy);

    // Apply configuration to arena
    m_arena.setGrid(m_config.hasFlakGrid());
    m_arena.setMode(m_config.getFlakRendererMode());
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
        handleEventReceptionInit(finished);
        break;

     case Jumping:
     case BeforeJumping:
     case Forwarding:
        processJump(finished);
        break;

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
         case Jumping:
         case BeforeJumping:
         case Forwarding:
            break;

         case Yellow:
            playTick(false);
            if (m_queue.empty()) {
                setState(Red, "Underflow");
            } else {
                startTimer();
            }
            break;

         case Green:
            playTick(false);
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
                startTimer();
            }
            break;

         case Draining:
            ok = playTick(false);
            if (!ok && m_queue.empty()) {
                setState(Finished, "Underflow");
            } else {
                startTimer();
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
    // ex FlakPlayWidget::onPlay
    switch (m_playState) {
     case Playing: onPause(); break;
     case Paused:  onPlay();  break;
    }
}

void
client::vcr::flak::PlaybackScreen::onMoveToBeginning()
{
    // ex FlakPlayWidget::onRewindToStart
    jumpTo(0);
}

void
client::vcr::flak::PlaybackScreen::onMoveBy(int delta)
{
    // ex FlakPlayWidget::onRewind, FlakPlayWidget::onForward
    int32_t newTime = std::max(0, m_visState.getTime() + delta);
    jumpTo(newTime);
}

void
client::vcr::flak::PlaybackScreen::onMoveToEnd()
{
    // ex FlakPlayWidget::onForwardToEnd
    jumpTo(MAX_TIME);
}

void
client::vcr::flak::PlaybackScreen::onChangeSpeed(bool faster)
{
    m_config.changeSpeed(faster ? -1 : +1);
    updateConfig();
    onPlay();
    client::widgets::showDecayingMessage(m_root, Format(m_translator("Speed: %s"), Configuration::getSpeedName(m_config.getSpeed(), m_translator)));
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
         case Jumping:
         case BeforeJumping:
         case Forwarding:
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
client::vcr::flak::PlaybackScreen::onFollow()
{
    size_t fleetIndex;
    if (m_unitList.getCurrentFleet(fleetIndex)) {
        if (const VisualisationState::Fleet* f = m_visState.fleets().at(fleetIndex)) {
            if (f->isAlive) {
                handleChanges(m_visSettings.followFleet(fleetIndex, m_visState));
            }
        }
    }
}

void
client::vcr::flak::PlaybackScreen::handleEventReceptionInit(bool finished)
{
    if (m_queue.empty()) {
        // First request didn't produce any events.
        // This means southside is confused/broken.
        // Mark battle done no matter what
        setState(Finished, "Events");
    } else {
        // Play first event; this initializes the setup
        playTick(true);
        initList();
        handleChanges(m_visSettings.followPlayer(m_teamSettings.getViewpointPlayer(), m_visState));

        bool play;
        if (finished) {
            // Battle complete, we have everything we need
            setState(Draining, "Events");
            play = true;
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

        // Schedule next
        if (play && m_playState == Playing) {
            startTimer();
        }
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

    if (play && m_playState == Playing) {
        // Play events
        // Do not play if m_playState != Playing; this would mean we can never slow-backward across a block boundary
        playTick(false);
        startTimer();
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
client::vcr::flak::PlaybackScreen::playTick(bool initial)
{
    bool result = false;
    if (!m_queue.empty()) {
        game::vcr::flak::EventRecorder rec;
        rec.swapContent(*m_queue.front());
        rec.replay(m_visState);
        m_queue.popFront();
        if (initial) {
            m_arena.init();
        }
        result = true;
    }
    if (m_visState.animate()) {
        result = true;
    }
    handleChanges(m_visSettings.updateCamera(m_visState));
    m_arena.requestRedraw();
    updateList();  // FIXME: only on change
    return result;
}

bool
client::vcr::flak::PlaybackScreen::playShadow()
{
    bool result = false;
    // Special case for m_targetTime=0: we need to play the first chunk as that sets up the ships (and doesn't advance time).
    while (!m_queue.empty() && (m_shadowState.getTime() < m_targetTime || (m_targetTime == 0 && m_shadowState.ships().empty()))) {
        game::vcr::flak::EventRecorder rec;
        rec.swapContent(*m_queue.front());
        rec.replay(m_shadowState);
        m_queue.popFront();
        result = m_shadowState.animate();
    }
    return result;
}

void
client::vcr::flak::PlaybackScreen::finishShadow()
{
    m_visState = m_shadowState;
    handleChanges(m_visSettings.updateCamera(m_visState));
    m_arena.requestRedraw();
    updateList();
}

void
client::vcr::flak::PlaybackScreen::jumpTo(int32_t time)
{
    // ex FlakPlayWidget::setTime (ehm...)
    m_timer->setInterval(afl::sys::INFINITE_TIMEOUT);
    m_playState = Paused;
    updatePlayState();

    switch (m_state) {
     case Initializing:
     case Jumping:
     case BeforeJumping:
     case Forwarding:
        break;

     case Red:
     case Yellow:
        if (time == m_visState.getTime()) {
            // ok, nothing to do
        } else {
            m_targetTime = time;
            m_shadowState = m_visState;
            setState(BeforeJumping, "Jump");
        }
        break;

     case Green:
     case Draining:
     case Finished:
        if (time == m_visState.getTime()) {
            // ok, nothing to do
        } else {
            m_targetTime = time;
            m_shadowState = m_visState;
            processJump(m_state != Green);
        }
        break;
    }
}

void
client::vcr::flak::PlaybackScreen::processJump(bool finished)
{
    // If the current data brings us towards our goal, process it
    if (m_targetTime < m_shadowState.getTime()) {
        // We need to go backward. Discard all data and jump.
        // For now, the only time we can jump to is 0 because only that revives dead ships!
        // Error case: if game side goes bonkers and sends wrong times, this means we may get into an game/UI ping-pong.
        // For now, don't bother; it's interruptible (user can exit playback).
        m_queue.clear();
        m_shadowState = VisualisationState();
        setState(Jumping, "processJump");
        m_proxy.jumpRequest(0);
    } else {
        // We need to go forward. Try to use up the queue, maybe that's already ok.
        bool fx = playShadow();
        if (finished) {
            // Finished. This means we either reached the target time with possibly some events to spare,
            // or that time cannot be reached.
            finishShadow();
            if (m_queue.empty() && !fx) {
                setState(Finished, "processJump");
            } else {
                setState(Draining, "processJump");
            }
        } else if (m_targetTime == m_shadowState.getTime()) {
            // Target time reached. Continue with Red/Yellow/Green as needed
            finishShadow();
            if (m_queue.size() < GREEN_THRESHOLD) {
                setState(m_queue.empty() ? Red : Yellow, "processJump");
                m_proxy.eventRequest();
            } else {
                setState(Green, "processJump");
            }
        } else {
            // Need more data
            setState(Forwarding, "processJump");
            m_proxy.eventRequest();
        }
    }
}

void
client::vcr::flak::PlaybackScreen::startTimer()
{
    int interval = m_config.getTickInterval() * m_config.getNumTicksPerBattleCycle();
    m_timer->setInterval(interval);
}

void
client::vcr::flak::PlaybackScreen::updatePlayState()
{
    m_playbackControl.setPlayState(m_playState == Playing);
}

void
client::vcr::flak::PlaybackScreen::initList()
{
    m_unitList.clear();

    afl::base::Memory<const VisualisationState::Ship> ships = m_visState.ships();
    afl::base::Memory<const VisualisationState::Fleet> fleets = m_visState.fleets();
    size_t fleetNr = 0;
    while (const VisualisationState::Fleet* f = fleets.eat()) {
        // Color
        const util::SkinColor::Color color = m_teamSettings.getPlayerColor(f->player);

        // Fleet header
        const VisualisationState::Ship* sh = ships.at(f->firstShip);
        const String_t fleetName = (f->numShips == 1 && sh != 0 && sh->isPlanet
                                    ? Format(m_translator("%s planet"), m_playerAdjectives.get(f->player))
                                    : Format(m_translator("%s fleet"), m_playerAdjectives.get(f->player)));
        m_unitList.addItem(CombatUnitList::Fleet, fleetNr, fleetName, CombatUnitList::Flags_t(), color);

        // Ships
        for (size_t i = 0, n = f->numShips; i < n; ++i) {
            const size_t shipNr = i + f->firstShip;
            const VisualisationState::Ship* sh = ships.at(shipNr);
            if (sh != 0) {
                m_unitList.addItem(CombatUnitList::Unit, shipNr,
                                   Format("%s (#%d)", sh->name, sh->id),
                                   CombatUnitList::Flags_t(), color);
            }
        }

        ++fleetNr;
    }
}

void
client::vcr::flak::PlaybackScreen::updateList()
{
    afl::base::Memory<const VisualisationState::Ship> ships = m_visState.ships();
    afl::base::Memory<const VisualisationState::Fleet> fleets = m_visState.fleets();
    for (size_t i = 0, n = m_unitList.getNumItems(); i < n; ++i) {
        CombatUnitList::Kind k;
        size_t slot;
        if (m_unitList.getItem(i, k, slot)) {
            switch (k) {
             case CombatUnitList::Fleet:
                if (const VisualisationState::Fleet* f = fleets.at(slot)) {
                    m_unitList.setFlagByIndex(i, CombatUnitList::Dead, !f->isAlive);
                }
                break;
             case CombatUnitList::Unit:
                if (const VisualisationState::Ship* sh = ships.at(slot)) {
                    m_unitList.setFlagByIndex(i, CombatUnitList::Dead, !sh->isAlive);
                }
                break;
            }
        }
    }
}

void
client::vcr::flak::PlaybackScreen::updateCamera()
{
    m_cameraControl.setAutoCamera(m_visSettings.isAutoCamera());
}

void
client::vcr::flak::PlaybackScreen::updateGrid()
{
    m_arena.setGrid(m_config.hasFlakGrid());
    m_cameraControl.setGrid(m_config.hasFlakGrid());
}

void
client::vcr::flak::PlaybackScreen::updateFollowedFleet()
{
    for (size_t i = 0, n = m_unitList.getNumItems(); i < n; ++i) {
        CombatUnitList::Kind k;
        size_t slot;
        if (m_unitList.getItem(i, k, slot)) {
            if (k == CombatUnitList::Fleet) {
                m_unitList.setFlagByIndex(i, CombatUnitList::Tagged, slot == m_visSettings.getFollowedFleet());
            }
        }
    }
}

void
client::vcr::flak::PlaybackScreen::updateMode()
{
    m_arena.setMode(m_config.getFlakRendererMode());
    m_cameraControl.setModeName(ArenaWidget::toString(m_config.getFlakRendererMode(), m_translator));
}

void
client::vcr::flak::PlaybackScreen::updateConfig()
{
    game::proxy::ConfigurationProxy configProxy(m_gameSender);
    m_config.save(configProxy);
}

void
client::vcr::flak::PlaybackScreen::handleChanges(game::vcr::flak::VisualisationSettings::Changes_t ch)
{
    if (ch.contains(VisualisationSettings::ParameterChange)) {
        m_arena.requestRedraw();
    }
    if (ch.contains(VisualisationSettings::CameraChange)) {
        updateCamera();
    }
    if (ch.contains(VisualisationSettings::FollowChange)) {
        updateFollowedFleet();
        m_arena.requestRedraw();
    }
}

const char*
client::vcr::flak::PlaybackScreen::toString(State st)
{
    switch (st) {
     case Initializing:  return "Initializing";
     case Jumping:       return "Jumping";
     case BeforeJumping: return "BeforeJumping";
     case Forwarding:    return "Forwarding";
     case Red:           return "Red";
     case Yellow:        return "Yellow";
     case Green:         return "Green";
     case Draining:      return "Draining";
     case Finished:      return "Finished";
    }
    return "?";
}

void
client::vcr::flak::PlaybackScreen::setState(State st, const char* why)
{
    m_log.write(afl::sys::LogListener::Trace, LOG_NAME,
                Format("%s -> %s (%s, t=%d, qsz=%d)")
                << toString(m_state)
                << toString(st)
                << why
                << m_visState.getTime()
                << m_queue.size());
    m_state = st;
}
