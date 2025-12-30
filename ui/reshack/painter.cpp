/**
  *  \file ui/reshack/painter.cpp
  *  \brief Class ui::reshack::Painter
  */

#include "ui/reshack/painter.hpp"

#include "gfx/basecontext.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/reshack/tool.hpp"


ui::reshack::Painter::Painter(afl::base::Ptr<gfx::PalettizedPixmap> pixmap, Palette::ColorMode colorMode, Root& root)
    : m_root(root),
      m_pixmap(pixmap),
      m_canvas(m_pixmap->makeCanvas().asPtr()),
      m_backup(),                 // initialized on demand
      m_tool(0),
      m_context(*m_canvas),
      m_lastPosition(0, 0),
      m_mouseIsDown(false),
      m_mouseColorSlot(0),
      m_scale(0),
      m_colorMode(colorMode)
{
    // RHPainter::RHPainter(Ptr<GfxPixmap> pixmap, ColorMode color_mode)
    m_colors[0] = Palette::FC_White;
    m_colors[1] = Palette::FC_Black;
    m_colors[2] = Palette::FC_Half;
    m_context.setRawColor(m_colors[0]);
    m_root.addPaletteHandler(*this);
}

ui::reshack::Painter::~Painter()
{
    // RHPainter::~RHPainter()
    m_root.removePaletteHandler(*this);
}

void
ui::reshack::Painter::draw(gfx::Canvas& can)
{
    // ex RHPainter::drawContent(GfxCanvas& can)
    int adj = 0;
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    if (m_scale == 0) {
        can.blit(getExtent().getTopLeft(), *m_canvas, gfx::Rectangle(0, 0, getExtent().getWidth(), getExtent().getHeight()));
    } else {
        /* Figure out size of pixels */
        const gfx::Point size = m_pixmap->getSize();
        int pixel = 1<<m_scale;
        int dx = getExtent().getLeftX();
        int dy = getExtent().getTopY();
        if (m_scale > 1) {
            /* At 4x and higher, draw grid */
            --pixel;
            for (int y = 0; y <= size.getY(); ++y) {
                can.drawHLine(gfx::Point(dx, dy + (y<<m_scale)), (size.getX()<<m_scale), m_root.colorScheme().getColor(Color_Dark), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
            }
            for (int x = 0; x <= size.getX(); ++x) {
                can.drawVLine(gfx::Point(dx + (x<<m_scale), dy), (size.getY()<<m_scale), m_root.colorScheme().getColor(Color_Dark), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
            }
            ++dx;
            ++dy;
            ++adj;
        }

        /* Draw */
        afl::base::Memory<const uint8_t> pixels = m_pixmap->pixels();
        for (int y = 0; y < size.getY(); ++y) {
            for (int x = 0; x < size.getX(); ++x) {
                uint8_t pixelColor = *pixels.eat();
                drawSolidBar(ctx, gfx::Rectangle(dx + (x<<m_scale), dy + (y<<m_scale), pixel, pixel), pixelColor);
            }
        }
    }

    /* Clear area outside image */
    const int r = (m_pixmap->getWidth() << m_scale) + adj;
    const int b = (m_pixmap->getHeight() << m_scale) + adj;
    gfx::Rectangle toClear = getExtent();               // Entire widget
    gfx::Rectangle toClearTop = toClear.splitY(b);      // Picture, including area to the right; leaving toClear as area below/right
    toClearTop.consumeX(r);                             // Leaving toClearTop as area to the right of image
    drawSolidBar(ctx, toClearTop, Color_Black);
    drawSolidBar(ctx, toClear, Color_Black);

    /* Aux lines */
    ctx.setColor(Color_GreenBlack);
    if (r < getExtent().getWidth()) {
        int len = getExtent().getWidth() - r;
        int left = m_scale != 0 ? 0 : r;
        if (len > 20) {
            len = 20;
        }
        for (std::vector<int>::iterator i = m_auxLines[false].begin(); i != m_auxLines[false].end(); ++i) {
            drawHLine(ctx, getExtent().getLeftX() + left, getExtent().getTopY() + (*i << m_scale), getExtent().getLeftX() + r + len - 1);
        }
    }
    if (b < getExtent().getHeight()) {
        int len = getExtent().getHeight() - b;
        int top = m_scale ? 0 : b;
        if (len > 20) {
            len = 20;
        }
        for (std::vector<int>::iterator i = m_auxLines[true].begin(); i != m_auxLines[true].end(); ++i) {
            drawVLine(ctx, getExtent().getLeftX() + (*i << m_scale), getExtent().getTopY() + top, getExtent().getTopY() + b + len - 1);
        }
    }
}

void
ui::reshack::Painter::handleStateChange(State st, bool enable)
{
    // ex RHPainter::onStateChange(int flags, bool enable)
    if (st == ActiveState && !enable && m_mouseIsDown) {
        handleMouse(m_lastPosition, MouseButtons_t());
    }
}

void
ui::reshack::Painter::handlePositionChange()
{
    requestRedraw();
}

ui::layout::Info
ui::reshack::Painter::getLayoutInfo() const
{
    // RHPainter::getLayoutInfo(LayoutInfo& info)
    return ui::layout::Info(gfx::Point(150, 150), ui::layout::Info::GrowBoth);
}

bool
ui::reshack::Painter::handleKey(util::Key_t key, int prefix)
{
    // ex RHPainter::handleEvent(const UIEvent& e, bool second_pass)
    if (key == util::Key_Escape && m_mouseIsDown) {
        requestActive();
        if (!m_backup.empty()) {
            m_pixmap->pixels().copyFrom(m_backup);
            requestRedraw();
        }
        m_mouseIsDown = false;
        return true;
    } else if (key == util::Key_Right + util::KeyMod_Shift) {
        requestActive();
        moveOrigin(1, 0);
        requestRedraw();
        return true;
    } else if (key == util::Key_Left + util::KeyMod_Shift) {
        requestActive();
        moveOrigin(m_pixmap->getWidth()-1, 0);
        requestRedraw();
        return true;
    } else if (key == util::Key_Up + util::KeyMod_Shift) {
        requestActive();
        moveOrigin(0, m_pixmap->getHeight()-1);
        requestRedraw();
        return true;
    } else if (key == util::Key_Down + util::KeyMod_Shift) {
        requestActive();
        moveOrigin(0, 1);
        requestRedraw();
        return true;
    } else if (key == 'h') {
        requestActive();
        m_pixmap->flipHorizontal();
        requestRedraw();
        return true;
    } else if (key == 'v') {
        requestActive();
        m_pixmap->flipVertical();
        requestRedraw();
        return true;
    } else {
        return defaultHandleKey(key, prefix);
    }
}

bool
ui::reshack::Painter::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // ex RHPainter::handleMouse(GfxPoint pt, int btn)
    /* If mouse is inside, process it. Also consume all mouse events while mouse is down,
       to track movement outside the widget. */
    if (getExtent().contains(pt) || m_mouseIsDown) {
        requestActive();

        gfx::Point effPos = screenToWorld(pt);

        if (!m_tool) {
            /* we don't have a tool, so ignore this event */
        } else if (!pressedButtons.empty()) {
            if (!m_mouseIsDown) {
                /* mouse pressed. */
                m_backup.resize(m_pixmap->pixels().size());
                m_backup.copyFrom(m_pixmap->pixels());

                /* colors */
                m_mouseColorSlot = pressedButtons.contains(MiddleButton)
                    ? 2
                    : pressedButtons.contains(RightButton)
                    ? 1
                    : 0;
                m_context.useCanvas(*m_canvas);
                m_context.setRawColor(m_colors[m_mouseColorSlot]);
                m_tool->click(m_context, effPos, m_colors[!m_mouseColorSlot]);
                requestRedraw();
            } else {
                /* mouse still pressed. */
                if (m_tool->needsPreview()) {
                    m_pixmap->pixels().copyFrom(m_backup);
                }
                m_tool->drag(m_context, effPos);
                requestRedraw();
            }
        } else {
            if (m_mouseIsDown) {
                /* mouse released. */
                if (m_tool->needsPreview()) {
                    m_pixmap->pixels().copyFrom(m_backup);
                }
                m_tool->release(m_context, effPos);
                requestRedraw();
            }
        }
        m_mouseIsDown = !pressedButtons.empty();
        m_lastPosition = pt;
        return true;
    } else {
        return false;
    }
}

void
ui::reshack::Painter::loadPalette(Root::PaletteLoader& ldr)
{
    // Update palette
    gfx::ColorQuad_t palette[256 - Color_Avail];
    m_pixmap->getPalette(Color_Avail, palette);
    ldr.setPalette(Color_Avail, palette);

    // Redraw
    requestRedraw();

    // Inform listeners; in particular, color selector and palette editor
    sig_colorChange.raise();
}

void
ui::reshack::Painter::unloadPalette()
{
    requestRedraw();
}

void
ui::reshack::Painter::addAuxLine(int n, bool onXAxis)
{
    // ex RHPainter::addAuxLine(int n, bool onXAxis)
    for (std::vector<int>::iterator i = m_auxLines[onXAxis].begin(); i != m_auxLines[onXAxis].end(); ++i) {
        if (*i == n) {
            return;
        }
    }
    m_auxLines[onXAxis].push_back(n);
    requestRedraw();
}

void
ui::reshack::Painter::removeAuxLine(int n, bool onXAxis)
{
    // ex RHPainter::removeAuxLine(int n, bool onXAxis)
    for (std::vector<int>::size_type i = 0; i != m_auxLines[onXAxis].size(); ++i) {
        if (m_auxLines[onXAxis][i] == n) {
            m_auxLines[onXAxis][i] = m_auxLines[onXAxis].back();
            m_auxLines[onXAxis].pop_back();
            requestRedraw();
            break;
        }
    }
}

void
ui::reshack::Painter::setTool(Tool* tool)
{
    // ex RHPainter::setTool(RHTool* tool)
    if (m_mouseIsDown) {
        requestActive();
        handleMouse(m_lastPosition, MouseButtons_t());
    }
    m_tool = tool;
}

ui::reshack::Tool*
ui::reshack::Painter::getTool() const
{
    // ex RHPainter::getTool() const
    return m_tool;
}

void
ui::reshack::Painter::setZoom(int scale)
{
    // ex RHPainter::setZoom(int scale)
    if (m_mouseIsDown) {
        requestActive();
        handleMouse(m_lastPosition, MouseButtons_t());
    }
    m_scale = scale;
    requestRedraw();
}

int
ui::reshack::Painter::getZoom() const
{
    // ex RHPainter::getZoom() const
    return m_scale;
}

void
ui::reshack::Painter::setColor(bool bg, uint8_t color)
{
    // ex RHPainter::setColor(bool bg, uint32_t color)
    if (m_colors[bg] != color) {
        m_colors[bg] = color;
        sig_colorChange.raise();
    }
}

uint8_t
ui::reshack::Painter::getColor(bool bg) const
{
    // ex RHPainter::getColor(bool bg) const
    return m_colors[bg];
}

ui::reshack::Palette::ColorMode
ui::reshack::Painter::getColorMode() const
{
    // ex RHPainter::getColorMode() const
    return m_colorMode;
}

afl::base::Ptr<gfx::PalettizedPixmap>
ui::reshack::Painter::getPixmap() const
{
    // ex RHPainter::getPixmap() const
    return m_pixmap;
}

void
ui::reshack::Painter::setPixmap(afl::base::Ptr<gfx::PalettizedPixmap> pix)
{
    // ex RHPainter::setPixmap(Ptr<GfxPixmap> pix)
    m_pixmap = pix;
    m_canvas = pix->makeCanvas().asPtr();
    requestRedraw();
}

gfx::Point
ui::reshack::Painter::getSize() const
{
    // ex RHPainter::getSize() const
    return m_pixmap->getSize();
}

void
ui::reshack::Painter::setSize(gfx::Point pt)
{
    // ex RHPainter::setSize(GfxPoint pt)
    if (pt != m_pixmap->getSize()) {
        afl::base::Ref<gfx::PalettizedPixmap> n = gfx::PalettizedPixmap::create(pt.getX(), pt.getY());
        n->copyPalette(*m_pixmap);
        n->copyPixels(gfx::Point(0, 0), *m_pixmap);
        setPixmap(n.asPtr());
    }
}

void
ui::reshack::Painter::moveOrigin(int dx, int dy)
{
    // ex RHPainter::moveOrigin(int dx, int dy)
    const gfx::Point size = m_pixmap->getSize();
    afl::base::Ref<gfx::PalettizedPixmap> c = gfx::PalettizedPixmap::create(size.getX(), size.getY());
    c->pixels().copyFrom(m_pixmap->pixels());

    m_pixmap->copyPixels(gfx::Point(dx,               dy),               *c);
    m_pixmap->copyPixels(gfx::Point(dx - size.getX(), dy),               *c);
    m_pixmap->copyPixels(gfx::Point(dx,               dy - size.getY()), *c);
    m_pixmap->copyPixels(gfx::Point(dx - size.getX(), dy - size.getY()), *c);
}

gfx::Point
ui::reshack::Painter::worldToScreen(gfx::Point pt)
{
    // ex RHPainter::worldToScreen(GfxPoint pt)
    return gfx::Point((pt.getX() + getExtent().getLeftX()) << m_scale, (pt.getY() + getExtent().getTopY()) << m_scale);
}

gfx::Point
ui::reshack::Painter::screenToWorld(gfx::Point pt)
{
    // ex RHPainter::screenToWorld(GfxPoint pt)
    return gfx::Point((pt.getX() - getExtent().getLeftX()) >> m_scale, (pt.getY() - getExtent().getTopY()) >> m_scale);
}
