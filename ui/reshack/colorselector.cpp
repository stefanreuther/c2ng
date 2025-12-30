/**
  *  \file ui/reshack/colorselector.cpp
  *  \brief Class ui::reshack::ColorSelector
  */

#include "ui/reshack/colorselector.hpp"

#include <algorithm>
#include "afl/base/deleter.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/string/format.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/reshack/painter.hpp"
#include "ui/reshack/session.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/numberselector.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

using ui::widgets::DecimalSelector;
using ui::widgets::FocusIterator;
using ui::widgets::StaticText;

/*
 *  PalettePreview - show a single color; one-trick widget for palette preview
 */

class ui::reshack::ColorSelector::PalettePreview : public ui::SimpleWidget {
 public:
    PalettePreview(Root& root, Painter& painter, uint8_t color)
        : m_root(root), m_painter(painter), m_color(color),
          conn_colorChange(m_painter.sig_colorChange.add(this, (void (Widget::*)()) &Widget::requestRedraw))
        { }
    ~PalettePreview()
        { }
    virtual void draw(gfx::Canvas& can)
        {
            gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
            drawSolidBar(ctx, getExtent(), m_color);
        }
    virtual void handleStateChange(State /*st*/, bool /*enable*/)
        { }
    virtual void handlePositionChange()
        { }
    virtual ui::layout::Info getLayoutInfo() const
        { return gfx::Point(100, 30); }
    virtual bool handleKey(util::Key_t /*key*/, int /*prefix*/)
        { return false; }
    virtual bool handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
        { return false; }

 private:
    Root& m_root;
    Painter& m_painter;
    uint8_t m_color;
    afl::base::SignalConnection conn_colorChange;
};


/*
 *  PaletteEditorDialog - dialog to edit an RGB triple
 */

class ui::reshack::ColorSelector::PaletteEditorDialog {
 public:
    PaletteEditorDialog(Session& session, Painter& painter, uint8_t index)
        : m_session(session),
          m_painter(painter),
          m_index(index),
          m_oldValue(),
          m_red(),
          m_green(),
          m_blue()
        {
            gfx::ColorQuad_t out[1] = {0};
            m_painter.getPixmap()->getPalette(index, out);
            m_red.set(RED_FROM_COLORQUAD(out[0]));
            m_green.set(GREEN_FROM_COLORQUAD(out[0]));
            m_blue.set(BLUE_FROM_COLORQUAD(out[0]));
            m_oldValue = out[0];

            m_red.sig_change.add(this, &PaletteEditorDialog::onChange);
            m_green.sig_change.add(this, &PaletteEditorDialog::onChange);
            m_blue.sig_change.add(this, &PaletteEditorDialog::onChange);
        }

    void run()
        {
            // ex RHPaletteEditorWindow::init() - sort-of
            afl::string::Translator& tx = m_session.translator();
            Root& root = m_session.root();
            afl::base::Deleter del;

            Window& win = del.addNew(new Window(tx("Palette"), root.provider(), root.colorScheme(), BLUE_WINDOW, ui::layout::VBox::instance5));

            // Build window
            //   VBox
            //     Grid
            //       "Red"   DecimalSelector
            //       "Green" DecimalSelector
            //       "Blue"  DecimalSelector
            //     Preview
            //     HBox [Button "OK", Button "Cancel", Spacer]

            // Component editors
            DecimalSelector& selRed   = del.addNew(new DecimalSelector(root, tx, m_red,   0, 255, 10));
            DecimalSelector& selGreen = del.addNew(new DecimalSelector(root, tx, m_green, 0, 255, 10));
            DecimalSelector& selBlue  = del.addNew(new DecimalSelector(root, tx, m_blue,  0, 255, 10));

            // Components
            Group& grid = del.addNew(new Group(del.addNew(new ui::layout::Grid(2))));
            grid.add(del.addNew(new StaticText(tx("Red"), SkinColor::Static, "+", root.provider())));
            grid.add(selRed.addButtons(del, root));
            grid.add(del.addNew(new StaticText(tx("Green"), SkinColor::Static, "+", root.provider())));
            grid.add(selGreen.addButtons(del, root));
            grid.add(del.addNew(new StaticText(tx("Blue"), SkinColor::Static, "+", root.provider())));
            grid.add(selBlue.addButtons(del, root));
            win.add(grid);

            // Preview
            win.add(del.addNew(new PalettePreview(root, m_painter, m_index)));

            // Buttons
            EventLoop loop(root);
            ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(root, tx));
            btn.addStop(loop);
            win.add(btn);

            FocusIterator& it = del.addNew(new FocusIterator(FocusIterator::Vertical | FocusIterator::Tab));
            it.add(selRed);
            it.add(selGreen);
            it.add(selBlue);
            win.add(it);

            win.add(del.addNew(new ui::widgets::Quit(root, loop)));

            win.pack();
            root.centerWidget(win);
            root.add(win);
            if (!loop.run()) {
                setPalette(m_oldValue);
            }
        }

 private:
    void onChange()
        {
            // ex RHPaletteEditorWindow::onChange()
            setPalette(COLORQUAD_FROM_RGB(m_red.get(), m_green.get(), m_blue.get()));
        }

    void setPalette(gfx::ColorQuad_t c)
        {
            m_painter.getPixmap()->setPalette(m_index, c);
            m_session.root().updatePalette(m_painter);
        }

    Session& m_session;
    Painter& m_painter;
    uint8_t m_index;
    gfx::ColorQuad_t m_oldValue;
    afl::base::Observable<int32_t> m_red;
    afl::base::Observable<int32_t> m_green;
    afl::base::Observable<int32_t> m_blue;
};


ui::reshack::ColorSelector::ColorSelector(Session& session, Painter& painter)
    : m_session(session),
      m_painter(painter),
      m_page(0),
      m_mouseDown(false),
      conn_colorChange(m_painter.sig_colorChange.add(this, (void (Widget::*)()) &Widget::requestRedraw))
{ }

ui::reshack::ColorSelector::~ColorSelector()
{ }

void
ui::reshack::ColorSelector::draw(gfx::Canvas& can)
{
    // ex RHColorSelector::drawContent(GfxCanvas& can)
    // 16 lines:
    //   nnn M[cc]N Pal
    //    |  |  | |  '- "can edit palette" indicator
    //    |  |  | `- "background color" marker
    //    |  |  `- color box
    //    |  `- "foreground color" marker
    //    `- color number
    // 17th line:
    //   [<] 0xNN [>]
    //    |   |    `- "next page"
    //    |   `- base color
    //    `- "previous page"
    // => 17x12 characters

    gfx::Context<uint8_t> ctx(can, m_session.root().colorScheme());
    ctx.useFont(*m_session.root().provider().getFont("f"));

    // Draw textual/skinnable items
    int lineHeight = ctx.getFont()->getLineHeight();
    int emWidth = ctx.getFont()->getEmWidth();
    const gfx::Rectangle& extent = getExtent();

    drawBackground(ctx, extent);
    ctx.setColor(Color_White);
    for (int i = 0; i < 16; ++i) {
        uint8_t color = static_cast<uint8_t>(i + 16*m_page);
        int y = extent.getTopY() + lineHeight*i;
        outText(ctx, gfx::Point(extent.getLeftX(), y), afl::string::Format("%3d", color));
        if (Palette::isEditableColor(m_painter.getColorMode(), color)) {
            outText(ctx, gfx::Point(extent.getLeftX() + 9*emWidth, y), "Pal");
        }
        if (color == m_painter.getColor(false)) {
            outText(ctx, gfx::Point(extent.getLeftX() + 4*emWidth, y), ">");
        }
        if (color == m_painter.getColor(true)) {
            outText(ctx, gfx::Point(extent.getLeftX() + 7*emWidth, y), "<");
        }
    }
    outText(ctx, gfx::Point(extent.getLeftX(), extent.getTopY() + 16*lineHeight), afl::string::Format("[<] 0x%X0 [>]", m_page));

    // Draw color boxes
    for (int i = 0; i < 16; ++i) {
        uint8_t color = static_cast<uint8_t>(i + 16*m_page);
        drawSolidBar(ctx, gfx::Rectangle(extent.getLeftX() + 5*emWidth, extent.getTopY() + lineHeight*i, 2*emWidth, lineHeight), color);
    }
}

void
ui::reshack::ColorSelector::handleStateChange(State st, bool enable)
{
    if (st == ActiveState && !enable) {
        m_mouseDown = false;
    }
}

void
ui::reshack::ColorSelector::handlePositionChange()
{ }

ui::layout::Info
ui::reshack::ColorSelector::getLayoutInfo() const
{
    // ex RHColorSelector::getLayoutInfo(LayoutInfo& info)
    return ui::layout::Info(m_session.root().provider().getFont("f")->getCellSize().scaledBy(12, 17));
}

bool
ui::reshack::ColorSelector::handleKey(util::Key_t key, int /*prefix*/)
{
    switch (key) {
     case util::Key_Right + util::KeyMod_Ctrl:
        requestActive();
        onNextPage();
        return true;

     case util::Key_Left + util::KeyMod_Ctrl:
        requestActive();
        onPreviousPage();
        return true;

     default:
        return false;
    }
}

bool
ui::reshack::ColorSelector::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    if (getExtent().contains(pt)) {
        // Mouse is inside this widget, so consume this event
        afl::base::Ref<gfx::Font> font = m_session.root().provider().getFont("f");
        int lineHeight = std::max(1, font->getLineHeight());
        int emWidth    = std::max(1, font->getEmWidth());

        int x = (pt.getX() - getExtent().getLeftX()) / emWidth;
        int y = (pt.getY() - getExtent().getTopY()) / lineHeight;

        uint8_t color = static_cast<uint8_t>(16*m_page + y);
        if (pressedButtons.contains(LeftButton)) {
            // Left mouse button pressed
            requestActive();
            m_mouseDown = true;
            if (x < 9 && y < 16) {
                m_painter.setColor(false, color);
            }
        } else if (pressedButtons.contains(RightButton)) {
            // Right mouse button pressed
            requestActive();
            m_mouseDown = true;
            if (x < 9 && y < 16) {
                m_painter.setColor(true, color);
            }
        } else if (m_mouseDown) {
            // Mouse button released. Check location.
            requestActive();
            m_mouseDown = false;
            if (x >= 9 && y < 16) {
                editColor(color);
            } else if (y == 16) {
                if (x < 3) {
                    onPreviousPage();
                } else if (x >= 9) {
                    onNextPage();
                } else {
                    // FIXME: 16x16 selector
                }
            } else {
                // ignore
            }
        }
        return true;
    } else {
        m_mouseDown = false;
        return false;
    }
}

void
ui::reshack::ColorSelector::onNextPage()
{
    // ex RHColorSelector::onNextPage()
    m_page = (m_page+1) & 15;
    requestRedraw();
}

void
ui::reshack::ColorSelector::onPreviousPage()
{
    // ex RHColorSelector::onPreviousPage()
    m_page = (m_page+15) & 15;
    requestRedraw();
}

void
ui::reshack::ColorSelector::editColor(uint8_t color)
{
    // ex RHColorSelector::editColor(uint8_t color)
    if (Palette::isEditableColor(m_painter.getColorMode(), color)) {
        PaletteEditorDialog(m_session, m_painter, color).run();
    }
}
