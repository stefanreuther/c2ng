/**
  *  \file client/map/screen.cpp
  */

#include <algorithm>
#include <cmath>
#include "client/map/screen.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/mutexguard.hpp"
#include "client/map/keymapoverlay.hpp"
#include "client/map/messageoverlay.hpp"
#include "client/map/prefixoverlay.hpp"
#include "client/map/starchartoverlay.hpp"
#include "client/si/userside.hpp"
#include "client/tiles/tilefactory.hpp"
#include "client/widgets/keymapwidget.hpp"
#include "client/widgets/referencelistbox.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/game.hpp"
#include "game/interface/contextprovider.hpp"
#include "game/interface/planetcontext.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/turn.hpp"
#include "gfx/complex.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/contextreceiver.hpp"
#include "interpreter/values.hpp"
#include "ui/draw.hpp"
#include "ui/layout/vbox.hpp"
#include "util/math.hpp"
#include "util/unicodechars.hpp"

using game::config::UserConfiguration;
using game::proxy::LockProxy;

namespace {
    const char*const LOG_NAME = "client.map.screen";

    /* Effect timer interval. 20 ms = 50 Hz */
    const uint32_t EFFECT_TIMER_INTERVAL = 20;

    bool isShortMovement(gfx::Point pt, int limit)
    {
        return util::squareInteger(pt.getX()) + util::squareInteger(pt.getY()) <= util::squareInteger(limit);
    }

    /* Configuration proxy Ids */
    enum {
        IdMouseStickiness,
        IdMouseWheelMode,
        IdAnimThreshold
    };


    /*
     *  ContextProvider implementation for starchart: create context according to a game::Reference
     *  FIXME: should this be in a public place?
     */
    class ContextProvider : public game::interface::ContextProvider {
     public:
        ContextProvider(game::Reference ref)
            : m_ref(ref)
            { }
        virtual void createContext(game::Session& session, interpreter::ContextReceiver& recv)
            {
                if (game::Game* g = session.getGame().get()) {
                    switch (m_ref.getType()) {
                     case game::Reference::Ship:
                        if (interpreter::Context* ctx = game::interface::ShipContext::create(m_ref.getId(), session, *g, g->viewpointTurn())) {
                            recv.pushNewContext(ctx);
                        }
                        break;

                     case game::Reference::Planet:
                     case game::Reference::Starbase:
                        if (interpreter::Context* ctx = game::interface::PlanetContext::create(m_ref.getId(), session, *g, g->viewpointTurn())) {
                            recv.pushNewContext(ctx);
                        }
                        break;

                     default:
                        break;
                    }
                }
            }
     private:
        game::Reference m_ref;
    };

    /* UI-side canonicalisation of tag names: "0" and "" are the same.
       Not 100% bullet-proof, but covers the usual case. */
    String_t wrapZero(const String_t& tagName)
    {
        return (tagName == "0" ? String_t() : tagName);
    }
}


class client::map::Screen::SharedState : public afl::base::RefCounted {
 public:
    SharedState()
        { }
    ~SharedState()
        { }

    void setPosition(game::map::Point pt)
        {
            afl::sys::MutexGuard g(m_mutex);
            m_pos = pt;
        }

    game::map::Point getPosition()
        {
            afl::sys::MutexGuard g(m_mutex);
            return m_pos;
        }

    void setKeymapName(const String_t& name)
        {
            afl::sys::MutexGuard g(m_mutex);
            m_keymapName = name;
        }

    String_t getKeymapName()
        {
            afl::sys::MutexGuard g(m_mutex);
            return m_keymapName;
        }

 private:
    afl::sys::Mutex m_mutex;
    game::map::Point m_pos;
    String_t m_keymapName;
};


class client::map::Screen::Properties : public game::interface::UserInterfacePropertyAccessor {
 public:
    Properties(game::Session& session, afl::base::Ref<SharedState> sharedState)
        : m_sharedState(sharedState),
          m_session(session)
        {
            m_session.uiPropertyStack().add(*this);
        }
    virtual ~Properties()
        {
            m_session.uiPropertyStack().remove(*this);
        }

    virtual bool get(game::interface::UserInterfaceProperty prop, std::auto_ptr<afl::data::Value>& result)
        {
            // ex StarchartWidget::getProperty
            switch (prop) {
             case game::interface::iuiScreenNumber:
                result.reset(interpreter::makeIntegerValue(4));
                return true;
             case game::interface::iuiScreenRegistered:
                result.reset(interpreter::makeIntegerValue(1));
                return true;
             case game::interface::iuiIterator:
             case game::interface::iuiAutoTask:
                result.reset();
                return true;
             case game::interface::iuiSimFlag:
                result.reset(interpreter::makeBooleanValue(0));
                return true;
             case game::interface::iuiScanX:
             case game::interface::iuiChartX:
                result.reset(interpreter::makeIntegerValue(m_sharedState->getPosition().getX()));
                return true;
             case game::interface::iuiScanY:
             case game::interface::iuiChartY:
                result.reset(interpreter::makeIntegerValue(m_sharedState->getPosition().getY()));
                return true;
             case game::interface::iuiKeymap:
                result.reset(interpreter::makeStringValue(m_sharedState->getKeymapName()));
                return true;
            }
            return false;
        }
    virtual bool set(game::interface::UserInterfaceProperty prop, const afl::data::Value* p)
        {
            // ex StarchartWidget::setProperty
            int32_t iv;
            switch (prop) {
             case game::interface::iuiScanX:
             case game::interface::iuiChartX:
                if (interpreter::checkIntegerArg(iv, p, 0, 10000)) {
                    setPosition(game::map::Point(iv, m_sharedState->getPosition().getY()));
                }
                return true;
             case game::interface::iuiScanY:
             case game::interface::iuiChartY:
                if (interpreter::checkIntegerArg(iv, p, 0, 10000)) {
                    setPosition(game::map::Point(m_sharedState->getPosition().getX(), iv));
                }
                return true;
             default:
                return false;
            }
        }
    void setPosition(game::map::Point pt)
        {
            m_sharedState->setPosition(pt);
            if (game::Game* pGame = m_session.getGame().get()) {
                pGame->cursors().location().set(pt);
            }
        }
 private:
    afl::base::Ref<SharedState> m_sharedState;
    game::Session& m_session;
};


class client::map::Screen::PropertiesFromSession : public afl::base::Closure<Properties*(game::Session&)> {
 public:
    PropertiesFromSession(const afl::base::Ref<SharedState>& sharedState)
        : m_sharedState(sharedState)
        { }
    virtual Properties* call(game::Session& session)
        { return new Properties(session, m_sharedState); }
 private:
    afl::base::Ref<SharedState> m_sharedState;
};



client::map::Screen::Screen(client::si::UserSide& userSide,
                            ui::Root& root,
                            afl::string::Translator& tx,
                            util::RequestSender<game::Session> gameSender)
    : Widget(),
      Control(userSide),
      m_root(root),
      m_gameSender(gameSender),
      m_replyReceiver(root.engine().dispatcher(), *this),
      m_widget(gameSender, root, root.getExtent().getSize()),
      m_tileContainer(ui::layout::VBox::instance5),
      m_tileHolder(),
      m_sharedState(*new SharedState()),
      m_effectTimer(root.engine().createTimer()),
      m_location(*this, userSide.mainLog()),
      m_locationCycleBreaker(0),
      m_haveInitialPosition(false),
      m_movement(),
      m_pendingMovement(),
      m_mouseStickyness(5),
      m_mouseWheelMode(UserConfiguration::WheelZoom),
      m_locationProxy(gameSender, root.engine().dispatcher()),
      m_refListProxy(gameSender, root.engine().dispatcher()),
      m_keymapProxy(gameSender, root.engine().dispatcher()),
      m_observerProxy(gameSender),
      m_drawingProxy(gameSender, root.engine().dispatcher()),
      m_lockProxy(gameSender, root.engine().dispatcher()),
      m_configProxy(gameSender, root.engine().dispatcher()),
      m_propertyProxy(gameSender.makeTemporary(new PropertiesFromSession(m_sharedState))),
      m_refList(),
      m_currentObject(),
      m_drawingTagFilter(),
      m_drawingTagFilterName(),
      m_viewName(),
      m_keymapName(),
      m_keymapKeys(),
      m_outputState(),
      m_stopped(false)
{
    // Add widgets so that their callbacks work
    addChild(m_widget, 0);
    addChild(m_tileContainer, 0);

    // Connect signals
    m_locationProxy.sig_locationResult.add(this, &Screen::onLocationResult);
    m_locationProxy.sig_configChange.add(this, &Screen::onMapConfigChange);
    m_locationProxy.sig_positionChange.add(this, &Screen::onPositionChange);
    m_locationProxy.sig_browseResult.add(this, &Screen::onBrowseResult);
    m_refListProxy.sig_listChange.add(this, &Screen::onListChange);
    m_refListProxy.sig_finish.add(this, &Screen::onListFinish);
    m_location.sig_positionChange.add(this, &Screen::onLocationChange);
    m_location.sig_objectChange.add(this, &Screen::onObjectChanged);
    m_keymapProxy.setListener(*this);
    m_lockProxy.sig_result.add(this, &Screen::onLockResult);
    m_effectTimer->sig_fire.add(this, &Screen::onEffectTimer);
    m_effectTimer->setInterval(EFFECT_TIMER_INTERVAL);
    m_configProxy.sig_intOptionChange.add(this, &Screen::onConfigChange);
    setColorScheme(*this);

    // Request configuration
    m_configProxy.observeOption(IdMouseStickiness, UserConfiguration::ChartMouseStickiness);
    m_configProxy.observeOption(IdMouseWheelMode,  UserConfiguration::ChartWheel);
    m_configProxy.observeOption(IdAnimThreshold,   UserConfiguration::ChartAnimThreshold);

    // Initialize
    setContextFromObject();
    setNewOverlay(BaseLayer, new StarchartOverlay(m_root, tx, m_location, *this));
}

client::map::Screen::~Screen()
{ }

void
client::map::Screen::draw(gfx::Canvas& can)
{
    // ex StarchartWidget::drawChart (sort-of)
    m_widget.draw(can);
}

void
client::map::Screen::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::map::Screen::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::map::Screen::handleChildAdded(Widget& /*child*/)
{ }

void
client::map::Screen::handleChildRemove(Widget& /*child*/)
{ }

void
client::map::Screen::handlePositionChange()
{
    m_widget.setExtent(getExtent());
    setTilePosition();
}

void
client::map::Screen::handleChildPositionChange(Widget& /*child*/, const gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
client::map::Screen::getLayoutInfo() const
{
    return m_root.getExtent().getSize();
}

bool
client::map::Screen::handleKey(util::Key_t key, int prefix)
{
    // Key reset pending movement/sticky mouse
    if (util::classifyKey(key) != util::ModifierKey) {
        m_pendingMovement = gfx::Point();
    }

    if (m_widget.handleKey(key, prefix)) {
        // This dispatches the keys into the overlays.
        // StarchartOverlay handles keymap keys.
        return true;
    } else {
        // Global keys
        switch (key) {
         case util::KeyMod_Ctrl + util::KeyMod_Shift + 's':
            m_root.saveScreenshot();
            return true;

         default:
            if (key >= '1' && key <= '9') {
                startPrefixArgument(key - '0');
                return true;
            } else {
                return false;
            }
        }
    }
}

bool
client::map::Screen::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    // Regular UI mouse handler: ignore
    return false;
}

bool
client::map::Screen::handleMouseRelative(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // Clicking closes message (UI.Overlay) and prefix (prefix, UseKeymap) overlays
    if (!pressedButtons.empty()) {
        if (m_overlays[PrefixLayer].get() != 0) {
            setNewOverlay(PrefixLayer, 0);
        }
        if (m_overlays[MessageLayer].get() != 0) {
            setNewOverlay(MessageLayer, 0);
        }
        m_pendingMovement = gfx::Point();
    }

    // Perform locking
    if (pressedButtons.contains(LeftButton)) {
        LockProxy::Flags_t flags;
        flags += LockProxy::Left;
        if (pressedButtons.contains(CtrlKey)) {
            flags += LockProxy::MarkedOnly;
        }
        lockObject(flags);
    } else if (pressedButtons.contains(RightButton)) {
        LockProxy::Flags_t flags;
        if (pressedButtons.contains(CtrlKey)) {
            flags += LockProxy::MarkedOnly;
        }
        lockObject(flags);
    } else {
        // Not locking
    }

    // Perform movement
    m_pendingMovement += pt;
    game::map::Point movement(+m_widget.renderer().unscale(m_pendingMovement.getX()),
                              -m_widget.renderer().unscale(m_pendingMovement.getY()));

    if (m_location.getNumObjects() != 0 && isShortMovement(m_pendingMovement, m_mouseStickyness)) {
        // Sticky mouse: cancel movement; keep accumulating
        movement = game::map::Point();
    }

    if (movement != game::map::Point()) {
        // Perform movement and reset accumulator.
        // If accumulated movement did not translate into a move (by sticky-mouse or high zoom factor), keep accumulating.
        m_location.moveRelative(movement.getX(), movement.getY());
        m_pendingMovement = gfx::Point();
    }

    return true;
}

void
client::map::Screen::handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
{
    using client::si::OutputState;
    switch (target) {
     case OutputState::NoChange:
     case OutputState::Starchart:
        interface().continueProcess(link);
        break;

     case OutputState::ShipScreen:
     case OutputState::PlanetScreen:
     case OutputState::BaseScreen:
     case OutputState::HistoryScreen:
     case OutputState::FleetScreen:
     case OutputState::ShipTaskScreen:
     case OutputState::PlanetTaskScreen:
     case OutputState::BaseTaskScreen:
     case OutputState::ExitProgram:
     case OutputState::ExitGame:
     case OutputState::PlayerScreen:
        interface().detachProcess(link);
        m_outputState.set(link, target);
        m_stopped = true;
        break;
    }
}

void
client::map::Screen::handleEndDialog(client::si::RequestLink2 link, int /*code*/)
{
    // This is not a dialog, just proceed the process
    interface().continueProcess(link);
}

void
client::map::Screen::handlePopupConsole(client::si::RequestLink2 link)
{
    defaultHandlePopupConsole(link);
}

void
client::map::Screen::handleScanKeyboardMode(client::si::RequestLink2 link)
{
    // In the starchart, keyboard mode is always active.
    interface().continueProcess(link);
}

void
client::map::Screen::handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
{
    setViewName(name);
    if (name.empty()) {
        // Special case: 'Chart.SetView ""' disables the view and keymap, even though there is no keymap named ""
        setKeymapName("STARCHART");
    } else {
        if (withKeymap) {
            setKeymapName(name);
        }
    }
    interface().continueProcess(link);
}

void
client::map::Screen::handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
{
    // ex WKeymapChartMode::startKeymapMode (sort-of)
    setNewOverlay(PrefixLayer, new KeymapOverlay(*this, name, prefix));
    interface().continueProcess(link);
}

void
client::map::Screen::handleOverlayMessage(client::si::RequestLink2 link, String_t text)
{
    // ex WMessageChartMode::showMessage (sort-of)
    setNewOverlay(MessageLayer, new MessageOverlay(*this, text));
    interface().continueProcess(link);
}

afl::base::Optional<game::Id_t>
client::map::Screen::getFocusedObjectId(game::Reference::Type type) const
{
    if (type == m_currentObject.getType()) {
        return m_currentObject.getId();
    } else {
        return 0;
    }
}

game::interface::ContextProvider*
client::map::Screen::createContextProvider()
{
    // ex StarchartWidget::enumContexts (sort-of)
    return new ContextProvider(m_currentObject);
}

void
client::map::Screen::requestObjectList(game::map::Point pos)
{
    class Initializer : public game::proxy::ReferenceListProxy::Initializer_t {
     public:
        Initializer(game::map::Point pos)
            : m_pos(pos)
            { }
        virtual void call(game::Session& session, game::ref::ListObserver& obs)
            {
                using game::ref::List;
                using game::Game;
                using game::Turn;

                obs.setSession(session);

                List list;
                if (Game* pGame = session.getGame().get()) {
                    list.addObjectsAt(pGame->viewpointTurn().universe(), pGame->mapConfiguration().getCanonicalLocation(m_pos), List::Options_t() + List::IncludeForeignShips + List::IncludePlanet, 0);
                }
                obs.setList(list);
            }
     private:
        game::map::Point m_pos;
    };

    m_refListProxy.setContentNew(std::auto_ptr<game::proxy::ReferenceListProxy::Initializer_t>(new Initializer(pos)));
}

void
client::map::Screen::requestLockObject(game::map::Point pos, game::proxy::LockProxy::Flags_t flags)
{
    m_lockProxy.requestPosition(pos, flags);
}

void
client::map::Screen::updateKeyList(util::KeySet_t& keys)
{
    m_keymapKeys.swap(keys);
}

gfx::Color_t
client::map::Screen::getColor(util::SkinColor::Color index)
{
    if (size_t(index) < util::SkinColor::NUM_COLORS) {
        return m_root.colorScheme().getColor(ui::DARK_COLOR_SET[index]);
    }
    return 0;
}

void
client::map::Screen::drawBackground(gfx::Canvas& /*can*/, const gfx::Rectangle& /*area*/)
{
    // ex WStarchartTileSkin::drawBackground
    // Leave this empty. Tiles are expect not to draw in multiple passes;
    // when they use it to refresh themselves, we redraw anyway.
}

void
client::map::Screen::drawPanel(gfx::Canvas& can, gfx::Rectangle area)
{
    // ex WStarchartTileSkin::drawFancyBar (sort-of)
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    area.grow(3, 3);

    if (can.getBitsPerPixel() >= 16) {
        /* 24-bit version */
        ctx.setAlpha(192);
        drawSolidBar(ctx, area, ui::Color_PanelBack24);
        area.grow(1, 1);
        ctx.setColor(ui::Color_PanelFrame24);
        drawRectangle(ctx, area);
        area.grow(1, 1);
        ctx.setColor(ui::Color_PanelBack24);
        gfx::drawRectangle(ctx, area);
    } else {
        /* 8-bit version */
        ctx.setFillPattern(gfx::FillPattern::GRAY50);
        ctx.setColor(ui::Color_PanelBack8);
        drawBar(ctx, area);
        area.grow(1, 1);
        ctx.setColor(ui::Color_PanelFrame8);
        gfx::drawRectangle(ctx, area);
    }
}

void
client::map::Screen::drawTiles(gfx::Canvas& can)
{
    // For use by StarchartOverlay
    if (m_tileContainer.getExtent().exists()) {
        drawPanel(can, m_tileContainer.getExtent());
        m_tileContainer.draw(can);
    }
}

void
client::map::Screen::drawObjectList(gfx::Canvas& can)
{
    // For use by StarchartOverlay
    // ex client/chart/standardmode.cc:drawObjectList
    const size_t LINE_LIMIT = 17;
    const size_t totalObjects = m_location.getNumObjects();
    size_t numObjects = totalObjects;
    size_t firstObject = 0;
    const size_t currentIndex = m_location.getCurrentObjectIndex();
    if (numObjects > LINE_LIMIT) {
        if (currentIndex < LINE_LIMIT/2) {
            // keep firstObject=0
        } else if (currentIndex >= numObjects-LINE_LIMIT/2) {
            firstObject = numObjects - LINE_LIMIT;
        } else {
            firstObject = currentIndex - LINE_LIMIT/2;
        }
        numObjects = LINE_LIMIT;
    }

    if (numObjects > 0) {
        afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());
        gfx::Point pt = getExtent().getCenter();
        const int lineHeight = font->getLineHeight();
        const int height = lineHeight * int(numObjects);
        const int width = font->getEmWidth() * 20;
        int y = pt.getY() - height/2;
        int x = pt.getX() - width - 50;

        gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());

        drawPanel(can, gfx::Rectangle(x, y, width, height));
        for (size_t i = 0; i < numObjects; ++i) {
            const size_t thisIndex = firstObject + i;
            gfx::Rectangle area(x, y, width, lineHeight);

            // Annotations
            String_t annotation;
            if (i == 0 && firstObject != 0) {
                annotation = UTF_UP_ARROW;
            }
            if (i+1 == numObjects && thisIndex+1 < totalObjects) {
                annotation = UTF_DOWN_ARROW;
            }
            if (!annotation.empty()) {
                int annotationWidth = std::min(area.getWidth(), font->getTextWidth(annotation) + 5);
                ctx.useFont(*font);
                ctx.setColor(util::SkinColor::Static);
                ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
                outText(ctx, gfx::Point(area.getRightX(), area.getTopY()), annotation);
                area.setWidth(area.getWidth() - annotationWidth);
            }

            // Focus bar
            if (thisIndex == currentIndex) {
                if (can.getBitsPerPixel() >= 16) {
                    can.drawBar(area, m_root.colorScheme().getColor(ui::Color_PanelFrame24), gfx::TRANSPARENT_COLOR, gfx::FillPattern::SOLID, 128);
                } else  {
                    can.drawBar(area, m_root.colorScheme().getColor(ui::Color_PanelFrame8), gfx::TRANSPARENT_COLOR, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
                }
            }

            // Actual item
            if (const game::ref::UserList::Item* pItem = m_location.getObjectByIndex(thisIndex)) {
                client::widgets::ReferenceListbox::drawItem(ctx, area, *pItem, m_root.provider());
            }

            y += lineHeight;
        }
    }
}


void
client::map::Screen::setNewOverlay(Layer layer, Overlay* pOverlay)
{
    // ex WChartModeList::pushMode, WChartModeList::removeMode
    // Update overlay. A possible previous overlay will remove itself from the widget.
    m_overlays[layer].reset(pOverlay);

    // Update the widget with all overlays.
    // Remove all, then add again. Topmost needs to be added last.
    for (size_t i = 0; i < NUM_LAYERS; ++i) {
        if (m_overlays[i].get() != 0) {
            m_widget.removeOverlay(*m_overlays[i]);
        }
    }
    for (size_t i = 0; i < NUM_LAYERS; ++i) {
        if (m_overlays[i].get() != 0) {
            m_widget.addOverlay(*m_overlays[i]);
        }
    }
    requestRedraw();
}

void
client::map::Screen::removeOverlay(Overlay* pOverlay)
{
    for (size_t i = 0; i < NUM_LAYERS; ++i) {
        if (m_overlays[i].get() == pOverlay) {
            setNewOverlay(Layer(i), 0);
            break;
        }
    }
}

bool
client::map::Screen::hasOverlay(Layer layer) const
{
    // ex WChartMode::getParent (sort-of)
    return m_overlays[layer].get() != 0;
}

void
client::map::Screen::setDrawingTagFilter(util::Atom_t tag, String_t tagName)
{
    const util::Atom_t* p = m_drawingTagFilter.get();
    if (p == 0 || *p != tag) {
        m_drawingTagFilter = tag;
        m_drawingTagFilterName = tagName;
        m_lockProxy.setDrawingTagFilter(m_drawingTagFilter);
        m_widget.setDrawingTagFilter(tag);
        requestRedraw();
    }
}

void
client::map::Screen::clearDrawingTagFilter()
{
    if (m_drawingTagFilter.isValid()) {
        m_drawingTagFilter = afl::base::Nothing;
        m_lockProxy.setDrawingTagFilter(m_drawingTagFilter);
        m_widget.clearDrawingTagFilter();
        requestRedraw();
    }
}

void
client::map::Screen::ensureDrawingTagVisible(const String_t& tagName)
{
    if (m_drawingTagFilter.isValid() && wrapZero(tagName) != wrapZero(m_drawingTagFilterName)) {
        clearDrawingTagFilter();
    }
    if (!hasVisibleDrawings()) {
        m_widget.toggleOptions(game::map::RenderOptions::Options_t(game::map::RenderOptions::ShowDrawings));
    }
}

bool
client::map::Screen::hasDrawingTagFilter() const
{
    return m_drawingTagFilter.isValid();
}

const afl::base::Optional<util::Atom_t>&
client::map::Screen::getDrawingTagFilter() const
{
    return m_drawingTagFilter;
}

const String_t&
client::map::Screen::getDrawingTagFilterName() const
{
    return m_drawingTagFilterName;
}

void
client::map::Screen::selectNearestVisibleDrawing()
{
    m_drawingProxy.selectNearestVisibleDrawing(m_location.getPosition(), NEAR_DISTANCE, m_drawingTagFilter);
}

bool
client::map::Screen::hasVisibleDrawings() const
{
    return m_widget.getOptions().getOption(game::map::RenderOptions::ShowDrawings) != game::map::RenderOptions::Disabled;
}

void
client::map::Screen::lockObject(game::proxy::LockProxy::Flags_t flags)
{
    if (!hasVisibleDrawings()) {
        flags += game::proxy::LockProxy::NoDrawings;
    }
    m_location.lockObject(flags);
}

void
client::map::Screen::browse(game::map::Location::BrowseFlags_t flags)
{
    if (m_location.startJump()) {
        m_locationProxy.browse(flags);
    }
}

bool
client::map::Screen::handleKeymapKey(util::Key_t key, int prefix)
{
    if (m_keymapKeys.find(key) != m_keymapKeys.end()) {
        // Keymap
        // (ex WStandardChartMode::handleEvent, part)
        executeKeyCommandWait(m_keymapName, key, prefix);
        return true;
    } else {
        return false;
    }
}

int
client::map::Screen::getMouseWheelMode() const
{
    return m_mouseWheelMode;
}

void
client::map::Screen::run(client::si::InputState& in, client::si::OutputState& out)
{
    // Proxy to dispatch events we read in relative-movement-mode to the correct methods
    class EventProxy : public gfx::EventConsumer {
     public:
        EventProxy(Screen& parent)
            : m_parent(parent)
            { }
        virtual bool handleKey(util::Key_t key, int prefix)
            { return m_parent.handleKey(key, prefix); }
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
            { return m_parent.handleMouseRelative(pt, pressedButtons); }
     public:
        Screen& m_parent;
    };
    EventProxy proxy(*this);

    // Make us visible
    setExtent(m_root.getExtent());
    m_root.add(*this);

    // Get things started by asking netherworld for current position stored in Session > Game > Cursors > Location
    m_locationProxy.postQueryLocation();

    // Continue possible inbound process
    continueProcessWait(in.getProcess());

    // Event loop
    while (!m_stopped) {
        m_root.handleEventRelative(proxy);
    }

    // Shutdown
    m_root.remove(*this);
    out = m_outputState;
}

void
client::map::Screen::updateCenter()
{
    if (m_haveInitialPosition) {
        if (m_movement.update(m_location.configuration(), 1)) {
            m_widget.setCenter(m_movement.getCurrentPosition());
        }
    }
}

void
client::map::Screen::onLocationResult(game::Reference ref, game::map::Point pt, game::map::Configuration config)
{
    // m_locationProxy.sig_locationResult as response to postQueryLocation()
    m_haveInitialPosition = true;
    m_location.setConfiguration(config);
    m_location.setPosition(pt);
    m_location.setFocusedObject(ref);
}

void
client::map::Screen::onMapConfigChange(game::map::Configuration config)
{
    // m_locationProxy.sig_configChange as response to config changes
    m_location.setConfiguration(config);
}

void
client::map::Screen::onPositionChange(game::map::Point pt)
{
    // m_location.sig_positionChange as response to nether position change
    if (pt != m_location.getPosition()) {
        ++m_locationCycleBreaker;
        m_location.setPosition(pt);
    }
}

void
client::map::Screen::onBrowseResult(game::Reference ref, game::map::Point pt)
{
    // m_location.sig_browseResult as response to browse()
    m_location.setPosition(pt);
    if (ref.isSet()) {
        m_location.setFocusedObject(ref);
    }
}

void
client::map::Screen::onListChange(const game::ref::UserList& list)
{
    // m_refListProxy.sig_listChange as response to explicit or implicit list change

    // Stash away list. We may get any number of onListChange callbacks (including none at all) for each request.
    m_refList = list;

    // If the ReferenceListProxy is idle, this is an unsolicited request, i.e. netherworld state change. Pass it on directly.
    if (m_refListProxy.isIdle()) {
        onListFinish();
    }
}

void
client::map::Screen::onListFinish()
{
    // m_refListProxy.sig_finish as confirmation for explicit list change
    m_location.setObjectList(m_refList);
}

void
client::map::Screen::onLocationChange(game::map::Point pt)
{
    // m_location.sig_positionChange
    m_movement.setTargetPosition(pt);
    updateCenter();

    interface().history().push(ScreenHistory::Reference(ScreenHistory::Starchart, pt.getX(), pt.getY()));
    m_sharedState->setPosition(pt);

    // If the move was initiated by a game-side change (onPositionChange()), do NOT send a request down.
    // Game-side will already have current data, so sending the request is unnecessary.
    // Moreover, game-side may have more current data than we do (for example, if that was the first
    // half of a "Chart.X := 1234; Chart.Y := 2345" command), so this request would cancel half of the
    // change, leading to an (interruptible) infinite loop between both sides battling over the position.
    if (m_locationCycleBreaker == 0) {
        m_locationProxy.setPosition(pt);
    } else {
        --m_locationCycleBreaker;
    }
}

void
client::map::Screen::onObjectChanged(game::Reference ref)
{
    // m_location.sig_objectChange

    // ex WStandardChartMode::onMove (sort-of)
    m_currentObject = ref;
    setContextFromObject();

    // Update reference. If setContextFromObject() changed the view, this is a no-op.
    // Otherwise, this will update display.
    m_observerProxy.setReference(m_currentObject);
    if (ref.isSet()) {
        m_locationProxy.setPosition(ref);
    }

    // If this is a ship, show its trail; if no object at all, keep last ship
    if (ref.isSet()) {
        if (ref.getType() == game::Reference::Ship) {
            m_widget.setShipTrailId(ref.getId());
            m_location.setPreferredObject(ref);
        } else {
            m_widget.setShipTrailId(0);
            m_location.setPreferredObject(game::Reference());
        }
    }
}

void
client::map::Screen::onLockResult(game::map::Point pt)
{
    // m_lockProxy.sig_result
    m_location.setPosition(pt);
}

void
client::map::Screen::onEffectTimer()
{
    // m_effectTimer.sig_fire
    updateCenter();
    sig_effectTimer.raise();
    m_effectTimer->setInterval(EFFECT_TIMER_INTERVAL);
}

void
client::map::Screen::onConfigChange(int id, int32_t value)
{
    // m_configProxy.sig_intOptionChange: just take over config
    switch (id) {
     case IdMouseStickiness:
        if (value >= 0 && value <= 1000) {
            m_mouseStickyness = value;
        }
        break;

     case IdMouseWheelMode:
        m_mouseWheelMode = value;
        break;

     case IdAnimThreshold:
        m_movement.setAnimationThreshold(value);
        break;
    }
}

void
client::map::Screen::setContextFromObject()
{
    switch (m_currentObject.getType()) {
     case game::Reference::Ship:
        setKeymapName("SHIPLOCK");
        setViewName("SHIPLOCK");
        break;

     case game::Reference::Planet:
     case game::Reference::Starbase:
        setKeymapName("PLANETLOCK");
        setViewName("PLANETLOCK");
        // FIXME: or UNKNOWNPLANETLOCK
        break;

     default:
        setKeymapName("STARCHART");
        setViewName(String_t());
        break;
    }
}

void
client::map::Screen::setKeymapName(const String_t& name)
{
    if (name != m_keymapName) {
        m_keymapName = name;
        m_keymapProxy.setKeymapName(name);
        m_sharedState->setKeymapName(name);
    }
}

void
client::map::Screen::setViewName(const String_t& name)
{
    if (name != m_viewName) {
        m_viewName = name;

        // Delete all tiles. This will remove them from m_tileContainer.
        m_tileHolder.clear();

        // Remove all listeners. FIXME: Tiles should do that themselves
        m_observerProxy.removeAllListeners();

        // Update reference so new tiles start looking at the right stuff.
        m_observerProxy.setReference(m_currentObject);

        // Tiles need a KeymapWidget, so give them one.
        client::widgets::KeymapWidget& keys = m_tileHolder.addNew(new client::widgets::KeymapWidget(m_gameSender, m_root.engine().dispatcher(), *this));

        // Build tiles
        client::tiles::TileFactory(interface(), keys, m_observerProxy)
            .createLayout(m_tileContainer, m_viewName, m_tileHolder);

        // Place it
        setTilePosition();
    }
}

void
client::map::Screen::startPrefixArgument(int initialValue)
{
    // ex WPrefixChartMode::startPrefixArg
    setNewOverlay(PrefixLayer, new PrefixOverlay(*this, initialValue));
}

void
client::map::Screen::setTilePosition()
{
    // Preferred size
    gfx::Point preferredSize = m_tileContainer.getLayoutInfo().getPreferredSize();

    // Available size: right half of screen, sans a bit
    gfx::Rectangle area = getExtent();
    area.consumeX(area.getWidth() / 2 + 50);

    // Adjust
    area.consumeY(std::max(0, (area.getHeight() - preferredSize.getY()) / 2));
    area.setWidth(std::min(area.getWidth(), preferredSize.getX()));
    area.setHeight(std::min(area.getHeight(), preferredSize.getY()));

    // Set position. We need to explicitly call doLayout() here to force re-layout, in case the widget does not change in size.
    m_tileContainer.setExtent(area);
    m_tileContainer.doLayout();
}
