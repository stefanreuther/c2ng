/**
  *  \file client/vcr/cameracontrolwidget.cpp
  *  \brief Class client::vcr::CameraControlWidget
  */

#include "client/vcr/cameracontrolwidget.hpp"
#include "gfx/context.hpp"

namespace {
    /*
     *  Layout Parameters
     *
     *  For simplicity, we operate on a grid, where each grid cell contains a button, plus padding.
     */

    enum {
        bZoomIn,
        bZoomOut,
        bToggleCamera,
        bFollowFleet,
        bToggleGrid,
        bToggle3D
    };

    // Padding around buttons, each direction
    const int BUTTON_PAD = 1;

    // Additional distance from button's grid cell to text
    const int TEXT_PAD = 4;

    // Number of lines
    const int NUM_LINES = 5;

    // Width in grid cells
    const int WIDTH = 7;

    void drawLine(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle& area, gfx::Point grid, String_t label, String_t value, int numButtons)
    {
        label += ": ";
        gfx::Rectangle line = area.splitY(grid.getY());
        line.consumeX(numButtons*grid.getX() + TEXT_PAD);

        ctx.setColor(util::SkinColor::Static);
        outTextF(ctx, line.splitX(ctx.getFont()->getTextWidth(label)), label);

        ctx.setColor(util::SkinColor::Green);
        outTextF(ctx, line, value);
    }
}

client::vcr::CameraControlWidget::CameraControlWidget(ui::Root& root, afl::string::Translator& tx)
    : m_root(root),
      m_translator(tx),
      m_buttons(),
      m_autoCamera(false),
      m_grid(false)
{
    addButton("A", 'a');
    addButton("Y", 'y');
    addButton("C", 'c');
    addButton("F", 'f');
    addButton("G", 'g');
    addButton("3", '3');
}

client::vcr::CameraControlWidget::~CameraControlWidget()
{ }

void
client::vcr::CameraControlWidget::setAutoCamera(bool enable)
{
    if (enable != m_autoCamera) {
        m_autoCamera = enable;
        requestRedraw();
    }
}

void
client::vcr::CameraControlWidget::setGrid(bool enable)
{
    if (enable != m_grid) {
        m_grid = enable;
        requestRedraw();
    }
}

void
client::vcr::CameraControlWidget::setModeName(String_t name)
{
    if (m_modeName != name) {
        m_modeName = name;
        requestRedraw();
    }
}

void
client::vcr::CameraControlWidget::dispatchKeysTo(gfx::KeyEventConsumer& w)
{
    for (size_t i = 0, n = m_buttons.size(); i < n; ++i) {
        m_buttons[i]->dispatchKeyTo(w);
    }
}

void
client::vcr::CameraControlWidget::draw(gfx::Canvas& can)
{
    // Content
    gfx::Point grid = getGridSize();
    gfx::Rectangle area = getExtent();
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());
    ctx.useFont(*font);
    ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
    ctx.setSolidBackground();

    // First line
    ctx.setColor(util::SkinColor::Static);
    {
        gfx::Rectangle line = area.splitY(grid.getY());
        line.consumeX(grid.getX() + TEXT_PAD);
        line.consumeRightX(grid.getX());
        outTextF(ctx, line, m_translator("Zoom"));
    }

    // Subsequent lines
    drawLine(ctx, area, grid, m_translator("Camera"), m_autoCamera ? m_translator("auto") : m_translator("manual"), 1);

    ctx.setColor(util::SkinColor::Static);
    {
        gfx::Rectangle line = area.splitY(grid.getY());
        line.consumeX(grid.getX() + TEXT_PAD);
        outTextF(ctx, line, m_translator("Follow fleet"));
    }

    drawLine(ctx, area, grid, m_translator("Grid"), m_grid ? m_translator("yes") : m_translator("no"), 1);
    drawLine(ctx, area, grid, m_translator("Mode"), m_modeName, 1);

    // Buttons
    defaultDrawChildren(can);
}

void
client::vcr::CameraControlWidget::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::vcr::CameraControlWidget::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::vcr::CameraControlWidget::handleChildAdded(Widget& /*child*/)
{ }

void
client::vcr::CameraControlWidget::handleChildRemove(Widget& /*child*/)
{ }

void
client::vcr::CameraControlWidget::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    gfx::Point grid = getGridSize();
    gfx::Rectangle area = getExtent();

    // First line
    {
        gfx::Rectangle line = area.splitY(grid.getY());
        placeButton(bZoomIn, line.splitX(grid.getX()));
        placeButton(bZoomOut, line.splitRightX(grid.getX()));
    }

    // Subsequent lines
    placeButton(bToggleCamera, area.splitY(grid.getY()).splitX(grid.getX()));
    placeButton(bFollowFleet,  area.splitY(grid.getY()).splitX(grid.getX()));
    placeButton(bToggleGrid,   area.splitY(grid.getY()).splitX(grid.getX()));
    placeButton(bToggle3D,     area.splitY(grid.getY()).splitX(grid.getX()));
}

void
client::vcr::CameraControlWidget::handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
{
}

ui::layout::Info
client::vcr::CameraControlWidget::getLayoutInfo() const
{
    return getGridSize().scaledBy(WIDTH, NUM_LINES);
}

bool
client::vcr::CameraControlWidget::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::vcr::CameraControlWidget::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

/* Get size of a grid cell */
gfx::Point
client::vcr::CameraControlWidget::getGridSize() const
{
    // For now, buttons (and therefore grid cells) are square
    int he = m_root.provider().getFont("+")->getLineHeight() * 9/8;
    he += 2*BUTTON_PAD;
    return gfx::Point(he, he);
}

/* Macro to add a button */
void
client::vcr::CameraControlWidget::addButton(const char* label, util::Key_t key)
{
    ui::widgets::Button& btn = *m_buttons.pushBackNew(new ui::widgets::Button(label, key, m_root));
    addChild(btn, 0);
}

/* Macro to place a button */
void
client::vcr::CameraControlWidget::placeButton(size_t which, gfx::Rectangle r)
{
    if (which < m_buttons.size()) {
        r.grow(-BUTTON_PAD, -BUTTON_PAD);
        m_buttons[which]->setExtent(r);
    }
}
