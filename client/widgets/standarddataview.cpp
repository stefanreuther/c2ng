/**
  *  \file client/widgets/standarddataview.cpp
  */

#include "client/widgets/standarddataview.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/layout/hbox.hpp"
#include "client/widgets/keymapwidget.hpp"

struct client::widgets::StandardDataView::Button {
    Button(ButtonAlignment alignment, int x, int y, std::auto_ptr<ui::widgets::AbstractButton> btn, ui::Root& root)
        : alignment(alignment),
          x(x),
          y(y),
          button(btn),
          frame(ui::layout::HBox::instance0, root.colorScheme(), ui::widgets::FrameGroup::NoFrame)
        {
            frame.setFrameWidth(2);
            frame.add(*button);
        }

    ButtonAlignment alignment;
    int x;
    int y;
    std::auto_ptr<ui::widgets::AbstractButton> button;
    ui::widgets::FrameGroup frame;
};


client::widgets::StandardDataView::StandardDataView(ui::Root& root, gfx::Point sizeInCells, KeymapWidget& widget)
    : CollapsibleDataView(root),
      m_sizeInCells(sizeInCells),
      m_docView(sizeInCells.scaledBy(root.provider().getFont(gfx::FontRequest())->getCellSize()), 0, root.provider()),
      m_keys(widget)
{
    addChild(m_docView, 0);

    // Must disable the DocumentView here because it overlaps with the buttons and would steal their events.
    // It would be pretty cool if we could allow hyperlinks on tiles.
    // To make that work, we'd have to
    // (a) make sure defaultDrawChildren() and defaultDrawChildren() in their processing order.
    //     Right now, the last child is drawn last (=atop), but sees mouse events last.
    //     This also means ui::Root needs a separate handleMouse implementation.
    // (b) make DocumentView less aggressive in stealing mouse events.
    //     Once the mouse pointer goes across a border not covered by a button, it claims mouse events (ActiveState).
    m_docView.setState(DisabledState, true);
}

client::widgets::StandardDataView::~StandardDataView()
{ }

void
client::widgets::StandardDataView::setChildPositions()
{
    gfx::Point origin = getAnchorPoint(LeftAligned + DataAligned);
    m_docView.setExtent(gfx::Rectangle(origin.getX(),
                                       origin.getY(),
                                       getExtent().getRightX() - origin.getX(),
                                       getExtent().getBottomY() - origin.getY()));

    // Grid size.
    // "*9/8" is Button's internal computation.
    // We would normally have to add +4 to compensate the frames.
    // However, these buttons traditionally are smaller.
    const int GRID = root().provider().getFont(gfx::FontRequest().addSize(1))->getTextHeight("Tp") * 9/8;
    const gfx::Rectangle& ex = getExtent();
    for (size_t i = 0, n = m_buttons.size(); i < n; ++i) {
        // FIXME: honor buttons' layout wishes: give them more or less X space if they want, give them less Y space if they want
        Button& b = *m_buttons[i];
        switch (b.alignment) {
         case Top:
            b.frame.setExtent(gfx::Rectangle(ex.getRightX() - GRID*(1+b.x), ex.getTopY() + GRID*b.y, GRID, GRID));
            break;

         case Bottom:
            b.frame.setExtent(gfx::Rectangle(ex.getRightX() - GRID*(1+b.x), ex.getBottomY() - GRID*(1+b.y), GRID, GRID));
            break;
        }
    }
    
    updateText();
}

gfx::Point
client::widgets::StandardDataView::getPreferredChildSize() const
{
    // FIXME: determine space taken by buttons.
    return m_sizeInCells.scaledBy(root().provider().getFont(gfx::FontRequest())->getCellSize());
}

void
client::widgets::StandardDataView::addNewButton(ButtonAlignment alignment, int x, int y, ui::widgets::AbstractButton* btn)
{
    std::auto_ptr<ui::widgets::AbstractButton> abtn(btn);
    Button* p = m_buttons.pushBackNew(new Button(alignment, x, y, abtn, root()));
    addChild(p->frame, getFirstChild());
    p->button->dispatchKeyTo(m_keys);
}

void
client::widgets::StandardDataView::setText(const util::rich::Text& text)
{
    m_text = text;
    updateText();
}

bool
client::widgets::StandardDataView::enableButton(util::Key_t key, ui::widgets::FrameGroup::Type type)
{
    if (Button* p = findButton(key)) {
        p->frame.setType(type);
        if (p->frame.getParent() == 0) {
            addChild(p->frame, getFirstChild());
        }
        return true;
    } else {
        return false;
    }
}

bool
client::widgets::StandardDataView::disableButton(util::Key_t key)
{
    if (Button* p = findButton(key)) {
        if (p->frame.getParent() != 0) {
            removeChild(p->frame);
        }
        return true;
    } else {
        return false;
    }
}

void
client::widgets::StandardDataView::updateText()
{
    m_docView.getDocument().clear();
    m_docView.getDocument().add(m_text);
    m_docView.getDocument().finish();
    m_docView.handleDocumentUpdate();
}

client::widgets::StandardDataView::Button*
client::widgets::StandardDataView::findButton(util::Key_t key)
{
    for (size_t i = 0, n = m_buttons.size(); i < n; ++i) {
        if (m_buttons[i]->button->getKey() == key) {
            return m_buttons[i];
        }
    }
    return 0;
}
