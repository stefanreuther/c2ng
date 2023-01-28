/**
  *  \file ui/root.cpp
  *  \brief Class ui::Root
  */

#include "ui/root.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "gfx/nullcolorscheme.hpp"
#include "ui/cardgroup.hpp"
#include "ui/spacer.hpp"

namespace {
    class KeyPoster : public ui::Root::EventTask_t {
     public:
        KeyPoster(util::Key_t key, int prefix)
            : m_key(key),
              m_prefix(prefix)
            { }
        virtual void call(gfx::EventConsumer& c)
            { c.handleKey(m_key, m_prefix); }
     private:
        util::Key_t m_key;
        int m_prefix;
    };
}

ui::Root::Root(gfx::Engine& engine, gfx::ResourceProvider& provider, const gfx::WindowParameters& param)
    : Widget(),
      m_engine(engine),
      m_engineWindowParameters(param),
      m_window(),
      m_filter(),
      m_localTaskQueue(),
      m_colorScheme(),
      m_provider(provider),
      m_mouseEventKnown(false),
      m_mouseEventRequested(false),
      m_mousePosition(),
      m_mouseButtons(),
      m_mousePrefix(0),
      m_mousePrefixPosted(false)
{
    initWindow();
    setState(FocusedState, true);
    setState(ModalState, true);
}

ui::Root::~Root()
{ }

void
ui::Root::draw(gfx::Canvas& can)
{
    gfx::MultiClipFilter filter(can);
    filter.add(getExtent());
    for (Widget* p = getFirstChild(); p != 0; p = p->getNextSibling()) {
        p->draw(filter);
        filter.remove(p->getExtent());
    }
    filter.drawBar(getExtent(), 0, 0, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
}

void
ui::Root::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::Root::requestChildRedraw(Widget& child, const gfx::Rectangle& area)
{
    gfx::RectangleSet set(area);
    for (Widget* p = getFirstChild(); p != 0 && p != &child; p = p->getNextSibling()) {
        set.remove(p->getExtent());
    }
    for (gfx::RectangleSet::Iterator_t it = set.begin(); it != set.end(); ++it) {
        m_filter->add(*it);
    }
}

void
ui::Root::handleChildAdded(Widget& child)
{
    setActiveChild(&child);
    setFocusedChild(&child);
    m_filter->add(child.getExtent());
}

void
ui::Root::handleChildRemove(Widget& child)
{
    m_filter->add(child.getExtent());
}

void
ui::Root::handlePositionChange()
{
    // I do not change my position.
}

void
ui::Root::handleChildPositionChange(Widget& child, const gfx::Rectangle& oldPosition)
{
    m_filter->add(oldPosition);
    m_filter->add(child.getExtent());
}

ui::layout::Info
ui::Root::getLayoutInfo() const
{
    return ui::layout::Info();
}

bool
ui::Root::handleKey(util::Key_t key, int prefix)
{
    // If key is a perceived keypress, clear the prefix argument
    // (which is for the mouse only).
    if (util::classifyKey(key) != util::ModifierKey) {
        consumeMousePrefixArgument();
    }

    switch (key ^ (util::KeyMod_Ctrl + util::KeyMod_Shift)) {
     case 's':
        saveScreenshot();
        return true;

     case 'q':
        return defaultHandleKey(util::Key_Quit, 0);

     case 'f':
        if (Widget* p = getFirstChild()) {
            drawFrames(*m_window, *p);
        }
        return true;

     case 'l':
        // FIXME: pending_layout_query = !pending_layout_query;
        return true;

     case 'r':
        m_filter->add(getExtent());
        return true;
    }

    return defaultHandleKey(key, prefix);
}

bool
ui::Root::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // Save mouse event for later postMouseEvent().
    // We must discard the DoubleClick bit to avoid generating repeated double clicks.
    m_mouseEventKnown = true;
    m_mousePosition = pt;
    m_mouseButtons = pressedButtons - DoubleClick;

    // If this is a button release, it must either consume the prefix or post a new one.
    // Otherwise, we assume the user clicked at a non prefix-capable widget and discard it below.
    m_mousePrefixPosted = false;

    // Process the event
    bool did = defaultHandleMouse(pt, pressedButtons);

    // Discard prefix if unused.
    if (!m_mousePrefixPosted && m_mouseButtons.empty()) {
        consumeMousePrefixArgument();
    }

    return did;
}

void
ui::Root::handleEvent()
{
    // Perform deferred redraws
    performDeferredRedraws();

    // Process an event
    std::auto_ptr<EventTask_t> t(m_localTaskQueue.extractFront());
    if (t.get() != 0) {
        t->call(*this);
    } else if (m_mouseEventRequested) {
        m_mouseEventRequested = false;
        handleMouse(m_mousePosition, m_mouseButtons);
    } else {
        m_engine.handleEvent(*this, false);
    }
}

void
ui::Root::handleEventRelative(EventConsumer& consumer)
{
    performDeferredRedraws();
    std::auto_ptr<EventTask_t> t(m_localTaskQueue.extractFront());
    if (t.get() != 0) {
        t->call(consumer);
    } else {
        m_engine.handleEvent(consumer, true);
    }
}

void
ui::Root::postMouseEvent()
{
    if (m_mouseEventKnown) {
        m_mouseEventRequested = true;
    }
}

void
ui::Root::postKeyEvent(util::Key_t key, int prefix)
{
    m_localTaskQueue.pushBackNew(new KeyPoster(key, prefix));
}

void
ui::Root::ungetKeyEvent(util::Key_t key, int prefix)
{
    m_localTaskQueue.pushFrontNew(new KeyPoster(key, prefix));
}

void
ui::Root::setMousePrefixArgument(int prefix)
{
    // ex UIBaseWidget::setPrefixArg
    m_mousePrefix = prefix;
    m_mousePrefixPosted = true;
}

int
ui::Root::consumeMousePrefixArgument()
{
    // ex UIBaseWidget::consumePrefixArg
    int result = m_mousePrefix;
    m_mousePrefix = 0;
    m_mousePrefixPosted = false;
    return result;
}

void
ui::Root::add(Widget& child)
{
    // ex UIRoot::add
    addChild(child, 0);
    setFocusedChild(&child);
}

void
ui::Root::remove(Widget& child)
{
    // ex UIRoot::close
    removeChild(child);
}

void
ui::Root::centerWidget(Widget& widget)
{
    // ex UIRoot::centerWidget
    // FIXME: could be member of Widget
    gfx::Rectangle widgetPos = widget.getExtent();
    widgetPos.centerWithin(getExtent());
    widget.setExtent(widgetPos);
}

// Move widget to screen edge.
void
ui::Root::moveWidgetToEdge(Widget& widget, gfx::HorizontalAlignment xPos, gfx::VerticalAlignment yPos, int offset)
{
    // ex UIRoot::moveWidgetToEdge
    // FIXME: could be member of Widget
    gfx::Rectangle widgetPos = widget.getExtent();
    widgetPos.moveToEdge(getExtent(), xPos, yPos, offset);
    widget.setExtent(widgetPos);
}

// Save a screenshot.
void
ui::Root::saveScreenshot()
{
    sig_screenshot.raise(*m_window);
}

void
ui::Root::initWindow()
{
    // Set up window
    m_window = m_engine.createWindow(m_engineWindowParameters).asPtr();
    setExtent(gfx::Rectangle(gfx::Point(0, 0), m_engineWindowParameters.size));

    // Set up drawing filter
    m_filter.reset(new gfx::MultiClipFilter(*m_window));

    // Palette
    m_colorScheme.init(*m_window);
    setColorScheme(gfx::NullColorScheme<util::SkinColor::Color>::instance);
}

void
ui::Root::performDeferredRedraws()
{
    m_filter->clipRegionAtRectangle(getExtent());
    if (!m_filter->empty()) {
        // Exchange the filter.
        // A widget might detect during partial redraw that it wants a full redraw, and add to the filter.
        // This must lead to a full redraw cycle.
        std::auto_ptr<gfx::MultiClipFilter> mc(m_filter);
        m_filter.reset(new gfx::MultiClipFilter(*m_window));

        // Draw
        draw(*mc);
    }
}

/** Draw frames around current window's widgets.
    This is used to visualize layout management.
    \param can Screen canvas
    \param widget Widget to draw

    Widgets are color-coded as follows:
    - red: Group (all descendants shown)
    - white: CardGroup (only visible descendant shown)
    - green, slashed: Spacer
    - yellow: regular widgets */
void
ui::Root::drawFrames(gfx::Canvas& can, Widget& widget)
{
    gfx::Context<uint8_t> ctx(can, m_colorScheme);

    if (CardGroup* c = dynamic_cast<CardGroup*>(&widget)) {
        if (Widget* ch = c->getFocusedChild()) {
            drawFrames(can, *ch);
        }
        ctx.setColor(Color_White);
    } else if (Widget* child = widget.getFirstChild()) {
        while (child != 0) {
            drawFrames(can, *child);
            child = child->getNextSibling();
        }
        ctx.setColor(Color_Red);
    } else if (dynamic_cast<Spacer*>(&widget) != 0) {
        ctx.setFillPattern(gfx::FillPattern::LTSLASH);
        ctx.setColor(Color_Green);
        drawBar(ctx, widget.getExtent());
    } else {
        ctx.setColor(Color_Yellow);
    }
    drawRectangle(ctx, widget.getExtent());
}
