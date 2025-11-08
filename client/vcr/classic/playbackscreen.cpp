/**
  *  \file client/vcr/classic/playbackscreen.cpp
  *  \brief Class client::vcr::classic::PlaybackScreen
  */

#include <algorithm>
#include "client/vcr/classic/playbackscreen.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/imageloader.hpp"
#include "client/widgets/decayingmessage.hpp"
#include "game/root.hpp"
#include "game/vcr/classic/battle.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/vcr/classic/eventlistener.hpp"
#include "game/vcr/classic/eventrecorder.hpp"
#include "game/vcr/classic/interleavedscheduler.hpp"
#include "game/vcr/classic/standardscheduler.hpp"
#include "game/vcr/classic/traditionalscheduler.hpp"
#include "game/vcr/classic/utils.hpp"
#include "gfx/anim/controller.hpp"
#include "gfx/anim/pixmapsprite.hpp"
#include "gfx/complex.hpp"
#include "gfx/gen/texture.hpp"
#include "gfx/rgbapixmap.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/draw.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/res/resid.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/panel.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/spritewidget.hpp"
#include "util/rich/text.hpp"

namespace gvc = game::vcr::classic;
using afl::string::Format;
using gvc::Side;
using gvc::Time_t;
using gvc::FighterStatus;
using gvc::LeftSide;
using gvc::ScheduledEvent;
using game::config::UserConfiguration;

namespace {
    /* Number of battle ticks to have buffered before starting playback.
       Playback will not start before this value is reached (to avoid immediately blocking on empty buffer again).
       More events will be requested if the buffer level drops below this value. */
    const int BUFFER_TIME = 50;

    const int32_t MAX_TIME = 0x7FFFFFFF;

    const char* LOG_NAME = "client.vcr";

    String_t makeUnitResource(gvc::Side side, bool isPlanet, int shipPictureNumber)
    {
        if (isPlanet) {
            return "vcr.planet";
        } else {
            return afl::string::Format("%s.%d", (side==gvc::LeftSide ? ui::res::VCR_LSHIP : ui::res::VCR_RSHIP), shipPictureNumber);
        }

    }

    /*
     *  Preloading all images. Visualisation will not wait for images being loaded.
     *  We therefore preload everything.
     *
     *  FIXME: this assumes a lot of knowledge that the Renderer and EventVisualizer have.
     *  Can we reorganize that a bit?
     */
    class ImageQuery : public util::Request<game::proxy::VcrDatabaseAdaptor> {
     public:
        virtual void handle(game::proxy::VcrDatabaseAdaptor& adaptor)
            {
                gvc::Database* db = dynamic_cast<gvc::Database*>(&*adaptor.getBattles());
                if (db != 0) {
                    for (size_t i = 0, n = db->getNumBattles(); i < n; ++i) {
                        if (gvc::Battle* b = db->getBattle(i)) {
                            handleBattle(*b, *adaptor.getShipList(), adaptor.getRoot()->hostConfiguration());
                        }
                    }
                }
            }

        void handleBattle(gvc::Battle& battle, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config)
            {
                handleSide(gvc::LeftSide, battle.left(), shipList, config);
                handleSide(gvc::RightSide, battle.right(), shipList, config);
            }

        void handleSide(gvc::Side side, const game::vcr::Object& obj, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config)
            {
                addImage(makeUnitResource(side, obj.isPlanet(), obj.getGuessedShipPicture(shipList.hulls())));
                int race = config.getPlayerRaceNumber(obj.getOwner());
                addImage(Format("vcr.lftr%d", race));
                addImage(Format("vcr.rftr%d", race));
            }

        void addImage(const String_t& img)
            {
                if (std::find(m_imageIds.begin(), m_imageIds.end(), img) == m_imageIds.end()) {
                    m_imageIds.push_back(img);
                }
            }

        const std::vector<String_t>& getResult() const
            { return m_imageIds; }

     private:
        std::vector<String_t> m_imageIds;
    };


    class PlaybackPanel : public ui::Widget {
     public:
        PlaybackPanel(ui::Root& root,
                      ui::Widget& spriteWidget,
                      ui::Widget& leftStatus,
                      ui::Widget& rightStatus,
                      ui::Widget& control)
            : m_root(root), m_spriteWidget(spriteWidget), m_leftStatus(leftStatus), m_rightStatus(rightStatus), m_control(control)
            {
                addChild(spriteWidget, 0);
                addChild(leftStatus, 0);
                addChild(rightStatus, 0);
                addChild(control, 0);
                setState(ModalState, true);

                // Texture
                // FIXME: this generates the texture. Store them as an asset instead.
                util::RandomNumberGenerator rng(0);
                afl::base::Ref<gfx::RGBAPixmap> pix(gfx::RGBAPixmap::create(120, 120));
                pix->pixels().fill(COLORQUAD_FROM_RGB(30,30,30));
                gfx::gen::Texture tex(*pix);
                tex.renderBrush(gfx::gen::ColorRange(COLORQUAD_FROM_RGB(30,30,30),
                                                     COLORQUAD_FROM_RGB(35,35,35)),
                                1000, 0, rng);
                m_texture = pix->makeCanvas().asPtr();
            }

        virtual void draw(gfx::Canvas& can)
            {
                gfx::Rectangle area(getExtent());
                area.consumeY(m_spriteWidget.getExtent().getHeight());

                gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
                ui::drawTiledArea(ctx, area, m_texture, ui::Color_Grayscale+1, 0);
                defaultDrawChildren(can);
            }
        virtual void handleStateChange(State /*st*/, bool /*enable*/)
            { }
        virtual void requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
            { requestRedraw(area); }
        virtual void handleChildAdded(Widget& /*child*/)
            { }
        virtual void handleChildRemove(Widget& /*child*/)
            { }
        virtual void handlePositionChange()
            { doLayout(); }
        virtual void handleChildPositionChange(Widget& /*child*/, const gfx::Rectangle& /*oldPosition*/)
            { }
        virtual ui::layout::Info getLayoutInfo() const
            { return ui::layout::Info(); }
        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
            { return defaultHandleMouse(pt, pressedButtons); }

     private:
        void doLayout()
            {
                // Sprites take top half
                gfx::Rectangle area = getExtent();
                m_spriteWidget.setExtent(area.splitY(area.getHeight()/2));

                // Left/Right status go into corners
                {
                    ui::layout::Info leftInfo = m_leftStatus.getLayoutInfo();
                    gfx::Rectangle leftArea(gfx::Point(), leftInfo.getPreferredSize());
                    leftArea.moveToEdge(area, gfx::LeftAlign, gfx::TopAlign, 10);
                    m_leftStatus.setExtent(leftArea);
                }
                {
                    ui::layout::Info rightInfo = m_rightStatus.getLayoutInfo();
                    gfx::Rectangle rightArea(gfx::Point(), rightInfo.getPreferredSize());
                    rightArea.moveToEdge(area, gfx::RightAlign, gfx::TopAlign, 10);
                    m_rightStatus.setExtent(rightArea);
                }

                // Controls go to center bottom
                {
                    ui::layout::Info controlInfo = m_control.getLayoutInfo();
                    gfx::Rectangle controlArea(gfx::Point(), controlInfo.getPreferredSize());
                    controlArea.moveToEdge(area, gfx::CenterAlign, gfx::BottomAlign, 10);
                    m_control.setExtent(controlArea);
                }
            }

        ui::Root& m_root;
        ui::Widget& m_spriteWidget;
        ui::Widget& m_leftStatus;
        ui::Widget& m_rightStatus;
        ui::Widget& m_control;
        afl::base::Ptr<gfx::Canvas> m_texture;
    };
}


/*
 *  PlaybackScreen
 */

client::vcr::classic::PlaybackScreen::PlaybackScreen(ui::Root& root, afl::string::Translator& tx,
                                                     util::RequestSender<game::proxy::VcrDatabaseAdaptor> adaptorSender,
                                                     size_t index,
                                                     game::proxy::ConfigurationProxy& confProxy,
                                                     afl::sys::LogListener& log)
    : m_root(root),
      m_translator(tx),
      m_adaptorSender(adaptorSender),
      m_proxy(adaptorSender, root.engine().dispatcher()),
      m_index(index),
      m_configProxy(confProxy),
      m_log(log),
      m_loop(root),
      m_config(),
      m_spriteWidget(),
      m_leftStatus(root, tx),
      m_rightStatus(root, tx),
      m_playbackControl(root, true),
      m_renderer(),
      m_state(Initializing),
      m_targetTime(0),
      m_scheduler(),
      m_timer(root.engine().createTimer()),
      m_ticks(0),
      m_playState(Playing),
      m_events(),
      m_currentTime(0),
      m_queuedTime(0)
{
    m_proxy.sig_event.add(this, &PlaybackScreen::handleEvents);
    m_proxy.sig_error.add(this, &PlaybackScreen::handleError);
}

client::vcr::classic::PlaybackScreen::~PlaybackScreen()
{
}

void
client::vcr::classic::PlaybackScreen::run()
{
    if (!prepare()) {
        return;
    }
    if (m_scheduler.get() == 0) {
        // Cannot happen
        return;
    }

    ui::widgets::Button btn("OK", util::Key_Escape, m_root);
    btn.sig_fire.addNewClosure(m_loop.makeStop(1));
    m_timer->sig_fire.add(this, &PlaybackScreen::onTick);

    m_playbackControl.sig_togglePlay.add(this, &PlaybackScreen::onTogglePlay);
    m_playbackControl.sig_moveToBeginning.add(this, &PlaybackScreen::onMoveToBeginning);
    m_playbackControl.sig_moveBy.add(this, &PlaybackScreen::onMoveBy);
    m_playbackControl.sig_moveToEnd.add(this, &PlaybackScreen::onMoveToEnd);
    m_playbackControl.sig_changeSpeed.add(this, &PlaybackScreen::onChangeSpeed);

    ui::Group g(ui::layout::VBox::instance5);
    g.add(btn);

    ui::Group gg(ui::layout::HBox::instance5);
    ui::Spacer spc1, spc2, spc3;
    gg.add(spc1);
    gg.add(m_playbackControl);
    gg.add(spc2);
    g.add(spc3);
    g.add(gg);

    ui::widgets::Quit quit(m_root, m_loop);
    g.add(quit);

    PlaybackPanel panel(m_root, m_spriteWidget, m_leftStatus, m_rightStatus, g);
    panel.setExtent(m_root.getExtent());

    m_renderer.reset(new Renderer(m_spriteWidget.controller(), m_root, m_translator, 1 - m_config.getEffectsMode()));
    m_renderer->setExtent(m_spriteWidget.getExtent());

    m_root.add(panel);

    m_proxy.initRequest(m_index);
    m_spriteWidget.tick();

    m_loop.run();

    m_renderer.reset();
}

void
client::vcr::classic::PlaybackScreen::handleError(String_t msg)
{
    /* Callback for ClassicVcrPlayerProxy::sig_error: stop playback */
    util::rich::Text text = m_translator("This battle cannot be played.");
    text += "\n\n";
    text += util::rich::Text(Format(m_translator("Most likely, your local configuration does not match the host's, or your host software is incompatible with PCC2.\n\n"
                                                 "Error message provided by playback engine: %s"), msg)).withStyle(util::rich::StyleAttribute::Small);

    ui::dialogs::MessageBox(text, m_translator("Visual Combat Recorder"), m_root).doOkDialog(m_translator);
    m_loop.stop(1);
}

void
client::vcr::classic::PlaybackScreen::handleEvents(util::StringInstructionList& list, bool finish)
{
    /* Callback for ClassicVcrPlayerProxy::sig_event: process events */
    gvc::EventRecorder r;
    r.swapContent(list);
    m_log.write(afl::sys::LogListener::Trace, LOG_NAME, Format("-> %d events", r.size()));

    switch (m_state) {
     case Initializing:
        r.replay(*m_scheduler);
        if (m_renderer.get() != 0 && m_renderer->isInitialized()) {
            handleEventReceptionRed(finish);
        } else {
            if (finish) {
                setState(Draining, "Events (no renderer)");
            }
        }
        break;

     case Jumping:
     case Forwarding:
        m_scheduler->removeAnimations();
        r.replay(*m_scheduler);
        handleEventReceptionForwarding(finish);
        break;

     case BeforeJumping:
        r.replay(*m_scheduler);
        m_events = std::queue<ScheduledEvent>();
        m_currentTime = -1;
        m_queuedTime = 0;
        setState(Jumping, "Events");
        break;

     case Red:
        r.replay(*m_scheduler);
        handleEventReceptionRed(finish);
        break;

     case Yellow:
     case Green:
     case Finished:  // Cannot happen, but normally follows Green
     case Draining:  // Cannot happen, but normally follows Green
        r.replay(*m_scheduler);
        handleEventReceptionYellowGreen(finish);
        break;
    }
}

bool
client::vcr::classic::PlaybackScreen::prepare()
{
    // Query images
    ImageQuery q;
    Downlink link(m_root, m_translator);
    link.call(m_adaptorSender, q);

    // Load images
    ImageLoader loader(m_root, m_translator);
    const std::vector<String_t>& images = q.getResult();
    for (size_t i = 0, n = images.size(); i < n; ++i) {
        loader.loadImage(images[i]);
    }
    if (!loader.wait()) {
        return false;
    }

    // Load config
    m_config.load(link, m_configProxy);
    switch (m_config.getRendererMode()) {
     case UserConfiguration::TraditionalRenderer:
        m_scheduler.reset(new game::vcr::classic::TraditionalScheduler(*this));
        break;
     case UserConfiguration::StandardRenderer:
        m_scheduler.reset(new game::vcr::classic::StandardScheduler(*this));
        break;
     case UserConfiguration::InterleavedRenderer:
        m_scheduler.reset(new game::vcr::classic::InterleavedScheduler(*this));
        break;
    }
    return true;
}

inline void
client::vcr::classic::PlaybackScreen::requestEvents()
{
    m_proxy.eventRequest();
}

inline void
client::vcr::classic::PlaybackScreen::requestJump(int32_t time)
{
    m_proxy.jumpRequest(time);
}

void
client::vcr::classic::PlaybackScreen::placeObject(game::vcr::classic::Side side, const game::vcr::classic::EventListener::UnitInfo& info)
{
    if (Renderer* p = m_renderer.get()) {
        p->placeObject(side, info);
    }

    // Main status
    UnitStatusWidget& st = unitStatus(side);
    UnitStatusWidget::Data d;
    d.unitName      = info.object.getName();
    d.ownerName     = info.ownerName;
    d.beamName      = info.beamName;
    d.launcherName  = info.launcherName;
    d.unitImageName = info.object.isPlanet() ? ui::res::PLANET : ui::res::makeResourceId(ui::res::SHIP, info.object.getPicture(), info.object.getId());
    d.numBeams      = info.object.getNumBeams();
    d.numLaunchers  = info.object.getNumLaunchers();
    d.numBays       = info.object.getNumBays();
    d.relation      = info.relation;
    d.isPlanet      = info.object.isPlanet();
    st.setData(d);

    // Levels
    st.setProperty(UnitStatusWidget::Shield, info.object.getShield());
    st.setProperty(UnitStatusWidget::Damage, info.object.getDamage());
    st.setProperty(UnitStatusWidget::Crew, info.object.getCrew());
    st.setProperty(UnitStatusWidget::NumTorpedoes, info.object.getNumTorpedoes());
    st.setProperty(UnitStatusWidget::NumFighters, info.object.getNumFighters());
}

void
client::vcr::classic::PlaybackScreen::pushEvent(ScheduledEvent e)
{
    m_events.push(e);
    if (e.type == ScheduledEvent::UpdateTime) {
        m_queuedTime = e.a;
    }
}

void
client::vcr::classic::PlaybackScreen::removeAnimations(int32_t from, int32_t to)
{
    if (Renderer* p = m_renderer.get()) {
        p->removeAnimations(from, to);
    }
}

void
client::vcr::classic::PlaybackScreen::onTogglePlay()
{
    if (m_playState == Paused) {
        onPlay();
    } else {
        onPause();
    }
}

void
client::vcr::classic::PlaybackScreen::onMoveToBeginning()
{
    jumpTo(0);
}

void
client::vcr::classic::PlaybackScreen::onMoveToEnd()
{
    jumpTo(MAX_TIME);
}

void
client::vcr::classic::PlaybackScreen::onChangeSpeed(bool faster)
{
    m_config.changeSpeed(faster ? -1 : +1);
    m_config.save(m_configProxy);
    onPlay();
    client::widgets::showDecayingMessage(m_root, Format(m_translator("Speed: %s"), Configuration::getSpeedName(m_config.getSpeed(), m_translator)));
}

void
client::vcr::classic::PlaybackScreen::onMoveBy(int delta)
{
    int32_t newTime = std::max(0, m_currentTime + delta);
    jumpTo(newTime);
}

void
client::vcr::classic::PlaybackScreen::jumpTo(int32_t t)
{
    // Cancel timer which should not be active during jump.
    // If it fires anyway because it got ready before this call, that will be grounded by onTick being ignored in state Jumping/BeforeJumping.
    switch (m_state) {
     case Initializing:
     case Jumping:
     case BeforeJumping:
     case Forwarding:
        break;

     case Red:
     case Yellow:
        m_timer->setInterval(afl::sys::INFINITE_TIMEOUT);
        m_playState = Paused;
        m_targetTime = t;
        setState(BeforeJumping, "Jump");
        requestJump(std::max(0, t - 10));
        break;

     case Green:
     case Draining:
     case Finished:
        m_renderer->setResultVisible(false);
        m_timer->setInterval(afl::sys::INFINITE_TIMEOUT);
        m_playState = Paused;
        m_targetTime = t;
        setState(Jumping, "Jump");
        m_events = std::queue<ScheduledEvent>();
        m_currentTime = -1;
        m_queuedTime = 0;
        requestJump(std::max(0, t - 10));
        break;
    }
}

void
client::vcr::classic::PlaybackScreen::onPlay()
{
    // ex VcrSpriteVisualizer::startPlaying
    // FIXME: when finished, restart
    if (m_playState == Paused && m_state != Finished) {
        m_playbackControl.setPlayState(true);
        m_playState = Playing;
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
            onTick();
            break;
        }
    }
}

void
client::vcr::classic::PlaybackScreen::onPause()
{
    // ex VcrSpriteVisualizer::stopPlaying
    if (m_playState == Playing) {
        m_playbackControl.setPlayState(false);
        m_playState = Paused;
        m_timer->setInterval(afl::sys::INFINITE_TIMEOUT);
    }
}

void
client::vcr::classic::PlaybackScreen::onTick()
{
    // ex VcrSpriteVisualizer::playTick [sort-of]
    if (m_playState == Playing) {
        switch (m_state) {
         case Initializing:
         case Red:
         case Jumping:
         case BeforeJumping:
         case Forwarding:
            break;

         case Yellow:
            ++m_ticks;
            if (executeEvents(MAX_TIME)) {
                m_spriteWidget.tick();
            }

            if (m_events.empty()) {
                setState(Red, "Underflow");
            } else {
                m_timer->setInterval(m_config.getTickInterval());
            }
            break;

         case Green:
            ++m_ticks;
            if (executeEvents(MAX_TIME)) {
                m_spriteWidget.tick();
            }

            if (m_events.empty()) {
                // Buffer exhausted during playback. Request events and suspend playback.
                requestEvents();
                setState(Red, "Underflow");
            } else {
                // Playback succeeded. Request new events if needed
                if (m_queuedTime < m_currentTime + BUFFER_TIME) {
                    requestEvents();
                    setState(Yellow, "Underflow");
                }
                m_timer->setInterval(m_config.getTickInterval());
            }
            break;

         case Draining:
            ++m_ticks;
            if (executeEvents(MAX_TIME)) {
                m_spriteWidget.tick();
            }

            if (m_events.empty()) {
                m_renderer->setResultVisible(true);
                m_spriteWidget.tick();      // FIXME? Needed to make the last sprite visible
                setState(Finished, "Underflow");
            } else {
                m_timer->setInterval(m_config.getTickInterval());
            }
            break;

         case Finished:
            break;
        }
    }
}

bool
client::vcr::classic::PlaybackScreen::executeEvents(int32_t timeLimit)
{
    // return true when we need to wait for a tick
    while (!m_events.empty()) {
        const ScheduledEvent& e = m_events.front();
        // m_log.write(afl::sys::LogListener::Trace, LOG_NAME, Format("Event(%s, %d, %d)", Event::toString(e.type), int(e.side), e.a));
        switch (e.type) {
         case ScheduledEvent::UpdateTime:
            m_currentTime = e.a;
            if (m_renderer.get() != 0) {
                m_renderer->updateTime(e.a);
            }
            break;
         case ScheduledEvent::UpdateDistance:
            if (m_renderer.get() != 0) {
                m_renderer->updateDistance(e.a);
            }
            break;
         case ScheduledEvent::MoveObject:
            if (m_renderer.get() != 0) {
                m_renderer->moveObject(e.side, e.a);
            }
            break;

         case ScheduledEvent::StartFighter:
            if (m_renderer.get() != 0) {
                m_renderer->startFighter(e.side, e.a, e.b, e.c);
            }
            break;
         case ScheduledEvent::RemoveFighter:
            if (m_renderer.get() != 0) {
                m_renderer->removeFighter(e.side, e.a);
            }
            break;
         case ScheduledEvent::UpdateNumFighters:
            unitStatus(e.side).addProperty(UnitStatusWidget::NumFighters, e.a);
            break;
         case ScheduledEvent::FireBeamShipFighter:
            if (m_renderer.get() != 0) {
                m_renderer->fireBeamShipFighter(e.side, e.a, e.b, e.c);
            }
            break;
         case ScheduledEvent::FireBeamShipShip:
            if (m_renderer.get() != 0) {
                m_renderer->fireBeamShipShip(e.side, e.a, e.b);
            }
            break;
         case ScheduledEvent::FireBeamFighterFighter:
            if (m_renderer.get() != 0) {
                m_renderer->fireBeamFighterFighter(e.side, e.a, e.b, e.c);
            }
            break;
         case ScheduledEvent::FireBeamFighterShip:
            if (m_renderer.get() != 0) {
                m_renderer->fireBeamFighterShip(e.side, e.a, e.b);
            }
            break;
         case ScheduledEvent::BlockBeam:
            unitStatus(e.side).setWeaponStatus(UnitStatusWidget::Beam, e.a, true);
            break;
         case ScheduledEvent::UnblockBeam:
            unitStatus(e.side).setWeaponStatus(UnitStatusWidget::Beam, e.a, false);
            break;
         case ScheduledEvent::UpdateBeam:
            unitStatus(e.side).setWeaponLevel(UnitStatusWidget::Beam, e.a, e.b);
            break;
         case ScheduledEvent::BlockLauncher:
            unitStatus(e.side).setWeaponStatus(UnitStatusWidget::Launcher, e.a, true);
            break;
         case ScheduledEvent::UnblockLauncher:
            unitStatus(e.side).setWeaponStatus(UnitStatusWidget::Launcher, e.a, false);
            break;
         case ScheduledEvent::UpdateLauncher:
            unitStatus(e.side).setWeaponLevel(UnitStatusWidget::Launcher, e.a, e.b);
            break;
         case ScheduledEvent::FireTorpedo:
            if (m_renderer.get() != 0) {
                m_renderer->fireTorpedo(e.side, e.a, e.b, e.c, e.d);
            }
            break;
         case ScheduledEvent::UpdateNumTorpedoes:
            unitStatus(e.side).addProperty(UnitStatusWidget::NumTorpedoes, e.a);
            break;
         case ScheduledEvent::MoveFighter:
            if (m_renderer.get() != 0) {
                m_renderer->moveFighter(e.side, e.a, e.b, e.c, e.d);
            }
            break;
         case ScheduledEvent::UpdateFighter:
            if (m_renderer.get() != 0) {
                m_renderer->updateFighter(e.side, e.a, e.b, e.c, e.d);
            }
            break;
         case ScheduledEvent::ExplodeFighter:
            if (m_renderer.get() != 0) {
                m_renderer->explodeFighter(e.side, e.a, e.b);
            }
            break;

         case ScheduledEvent::UpdateObject:
            unitStatus(e.side).setProperty(UnitStatusWidget::Damage, e.a);
            unitStatus(e.side).setProperty(UnitStatusWidget::Crew, e.b);
            unitStatus(e.side).setProperty(UnitStatusWidget::Shield, e.c);
            unitStatus(e.side).unblockAllWeapons();
            break;

         case ScheduledEvent::UpdateAmmo:
            unitStatus(e.side).setProperty(UnitStatusWidget::NumTorpedoes, e.a);
            unitStatus(e.side).setProperty(UnitStatusWidget::NumFighters, e.b);
            break;

         case ScheduledEvent::HitObject:
            if (m_renderer.get() != 0) {
                m_renderer->hitObject(e.side, e.a, e.b, e.c, e.d);
            }
            unitStatus(e.side).addProperty(UnitStatusWidget::Damage,  e.a);
            unitStatus(e.side).addProperty(UnitStatusWidget::Crew,   -e.b);
            unitStatus(e.side).addProperty(UnitStatusWidget::Shield, -e.c);
            break;

         case ScheduledEvent::SetResult:
            if (m_renderer.get() != 0) {
                m_renderer->setResult(game::vcr::classic::BattleResult_t::fromInteger(e.a));
            }
            break;

         case ScheduledEvent::WaitTick:
            // m_log.write(afl::sys::LogListener::Trace, LOG_NAME, Format("WaitTick: ticks=%d / %d", m_ticks, m_ticksPerBattleCycle));
            if (m_ticks < m_config.getNumTicksPerBattleCycle()) {
                return true;
            }
            m_ticks = 0;
            if (m_currentTime >= timeLimit) {
                return true;
            }
            break;

         case ScheduledEvent::WaitAnimation:
            if (m_renderer.get() && m_renderer->hasAnimation(e.a)) {
                return true;
            }
            break;
        }
        m_events.pop();
    }
    return false;
}


void
client::vcr::classic::PlaybackScreen::handleEventReceptionRed(bool finish)
{
    bool play;
    if (m_events.empty() || m_queuedTime < m_currentTime + BUFFER_TIME) {
        // Buffer not full enough yet; load more.
        if (finish) {
            setState(Draining, "Events");
            play = true;
        } else {
            requestEvents();
            setState(Red, "Events");
            play = false;
        }
    } else {
        // Buffer sufficiently full.
        setState(Green, "Events");
        play = true;
    }

    if (play) {
        // Start events.
        if (executeEvents(MAX_TIME)) {
            // We need to wait, so draw everything.
            m_spriteWidget.tick();

            // If we ought to play, do so.
            if (m_playState == Playing) {
                m_timer->setInterval(m_config.getTickInterval());
            }
        } else {
            // Events exhausted. Do NOT draw, the frame is incomplete.
        }
    }
}

void
client::vcr::classic::PlaybackScreen::handleEventReceptionYellowGreen(bool finish)
{
    // No need to start a timer because we're in Yellow/Green state where it is already active.
    // No need to handle m_events.empty(); we do not enter Yellow/Green with no active event.
    if (m_queuedTime < m_currentTime + BUFFER_TIME) {
        if (finish) {
            setState(Draining, "Events");
        } else {
            requestEvents();
            setState(Yellow, "Events");
        }
    } else {
        setState(Green, "Events");
    }
}

void
client::vcr::classic::PlaybackScreen::handleEventReceptionForwarding(bool finish)
{
    // Set state to forwarding so everyone knows when called from here
    setState(Forwarding, "Events");

    // Advance until time reached or events exhausted
    while (m_currentTime < m_targetTime && executeEvents(m_targetTime)) {
        m_spriteWidget.controller().tick();
        ++m_ticks;
    }

    // Pick next state
    bool play;
    if (finish) {
        setState(Draining, "Events");
        play = true;
    } else if (m_currentTime >= m_targetTime) {
        if (m_queuedTime < m_currentTime + BUFFER_TIME) {
            requestEvents();
            setState(Yellow, "Events");
        } else {
            setState(Green, "Events");
        }
        play = true;
    } else {
        requestEvents();
        play = false;
    }

    // If we ought to play, do so.
    if (play) {
        m_spriteWidget.tick();
        m_spriteWidget.requestRedraw();
        if (m_playState == Playing) {
            m_timer->setInterval(m_config.getTickInterval());
        }
    }
}

client::vcr::UnitStatusWidget&
client::vcr::classic::PlaybackScreen::unitStatus(game::vcr::classic::Side side)
{
    return side == gvc::LeftSide ? m_leftStatus : m_rightStatus;
}

const char*
client::vcr::classic::PlaybackScreen::toString(State st)
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
client::vcr::classic::PlaybackScreen::setState(State st, const char* why)
{
    m_log.write(afl::sys::LogListener::Trace, LOG_NAME,
                Format("%s -> %s (%s, t=%d, q=%d)")
                << toString(m_state)
                << toString(st)
                << why
                << m_currentTime
                << m_queuedTime);
    m_state = st;
}

