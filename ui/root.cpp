/**
  *  \file ui/root.cpp
  *  \brief Class ui::Root
  */

#include "ui/root.hpp"
#include "ui/cardgroup.hpp"
#include "ui/spacer.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"

ui::Root::Root(gfx::Engine& engine, gfx::ResourceProvider& provider, int wi, int he, int bpp, gfx::Engine::WindowFlags_t flags)
    : Widget(),
      m_engine(engine),
      m_engineWindowFlags(flags),
      m_engineWindowSize(wi, he),
      m_engineWindowBPP(bpp),
      m_window(),
      m_filter(),
      m_colorScheme(),
      m_provider(provider),
      m_mouseEventKnown(false),
      m_mouseEventRequested(false),
      m_mousePosition(),
      m_mouseButtons()
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
ui::Root::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    // I do not change my position.
}

void
ui::Root::handleChildPositionChange(Widget& child, gfx::Rectangle& oldPosition)
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
    switch (key ^ (util::KeyMod_Ctrl + util::KeyMod_Shift)) {
     case 's':
        // FIXME: doScreenshot();
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
    return defaultHandleMouse(pt, pressedButtons);
}

void
ui::Root::handleEvent()
{
    // Perform deferred redraws
    performDeferredRedraws();

    // Process an event
    if (m_mouseEventRequested) {
        m_mouseEventRequested = false;
        handleMouse(m_mousePosition, m_mouseButtons);
    } else {
        m_engine.handleEvent(*this, false);
    }
}

void
ui::Root::postMouseEvent()
{
    if (m_mouseEventKnown) {
        m_mouseEventRequested = true;
    }
}

ui::ColorScheme&
ui::Root::colorScheme()
{
    return m_colorScheme;
}

gfx::ResourceProvider&
ui::Root::provider()
{
    return m_provider;
}

gfx::Engine&
ui::Root::engine()
{
    return m_engine;
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
ui::Root::moveWidgetToEdge(Widget& widget, int xPos, int yPos, int offset)
{
    // ex UIRoot::moveWidgetToEdge
    // FIXME: could be member of Widget
    gfx::Rectangle widgetPos = widget.getExtent();
    widgetPos.moveToEdge(getExtent(), xPos, yPos, offset);
    widget.setExtent(widgetPos);
}

void
ui::Root::initWindow()
{
    // Set up window
    m_window = m_engine.createWindow(m_engineWindowSize.getX(), m_engineWindowSize.getY(), m_engineWindowBPP, m_engineWindowFlags);
    setExtent(gfx::Rectangle(gfx::Point(0, 0), m_engineWindowSize));

    // Set up drawing filter
    m_filter.reset(new gfx::MultiClipFilter(*m_window));

    // Palette
    m_colorScheme.init(*m_window);
    setColorScheme(m_colorScheme);
}

void
ui::Root::performDeferredRedraws()
{
    m_filter->clipRegionAtRectangle(getExtent());
    if (!m_filter->empty()) {
        // FIXME: optimisation opportunity: draw on a custom filter
        draw(*m_filter);
        m_filter->clear();
    }
}

/** Draw frames around current window's widgets.
    This is used to visualize layout management.
    \param can Screen canvas
    \param widget Widget to draw

    Widgets are color-coded as follows:
    - red: UIGroup (all descendants shown)
    - white: UICardGroup (only visible descendant shown)
    - green, slashed: UISpacer
    - yellow: regular widgets */
void
ui::Root::drawFrames(gfx::Canvas& can, Widget& widget)
{
    gfx::Context ctx(can);
    ctx.useColorScheme(m_colorScheme);

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
