/**
  *  \file ui/rich/documentview.cpp
  */

#include "ui/rich/documentview.hpp"
#include "gfx/clipfilter.hpp"
#include "gfx/complex.hpp"
#include "util/unicodechars.hpp"

ui::rich::DocumentView::DocumentView(gfx::Point pref_size, uint16_t key_flags, gfx::ResourceProvider& provider)
    : ScrollableWidget(),
      m_provider(provider),
      doc(provider),
      pref_size(pref_size),
      m_minSize(pref_size),
      key_flags(key_flags),
      mdown(false),
      selected_link(Document::nil),
      hover_link(Document::nil),
      m_topY(0)
{
    // ex UIRichDocument::UIRichDocument
}

ui::rich::DocumentView::~DocumentView()
{ }

ui::rich::Document&
ui::rich::DocumentView::getDocument()
{
    // ex UIRichDocument::getDocument
    return doc;
}

const ui::rich::Document&
ui::rich::DocumentView::getDocument() const
{
    return doc;
}


// /** Handle document update. Must be called when the document was updated
//     while the widget is displayed. */
void
ui::rich::DocumentView::handleDocumentUpdate()
{
    // ex UIRichDocument::handleDocumentUpdate (sort-of)
    m_topY = 0;
    sig_change.raise();

    // Configure rest
    selected_link = hover_link = Document::nil;
    requestRedraw();
    // FIXME: postMouseEvent();
}

void
ui::rich::DocumentView::adjustToDocumentSize()
{
    // ex UIRichDocument::adjustToDocumentSize
    pref_size.setY(doc.getDocumentHeight());
}

void
ui::rich::DocumentView::setPreferredSize(gfx::Point prefSize)
{
    pref_size = prefSize;
}

ui::rich::Document::LinkId_t
ui::rich::DocumentView::getSelectedLink() const
{
    // ex UIRichDocument::getSelectedLink
    return selected_link;
}

void
ui::rich::DocumentView::setSelectedLink(Document::LinkId_t link)
{
    // ex UIRichDocument::setSelectedLink
    if (link != selected_link) {
        Document::LinkId_t old = selected_link;
        selected_link = link;
        setLink(old);
        setLink(selected_link);
        requestRedraw();
    }
}

void
ui::rich::DocumentView::setTopY(int topY)
{
    // ex UIRichDocument::scrollTo
    // FIXME: replace by setPageTop()
    int maxY = doc.getDocumentHeight() - getExtent().getHeight();
    if (topY > maxY) {
        topY = maxY;
    }
    if (topY < 0) {
        topY = 0;
    }

    if (topY != m_topY) {
        m_topY = topY;
        if (selected_link != Document::nil && !doc.isLinkVisible(selected_link, gfx::Rectangle(0, topY, getExtent().getWidth(), getExtent().getHeight()))) {
            setSelectedLink(Document::nil);
        }
        if (hover_link != Document::nil && !doc.isLinkVisible(hover_link, gfx::Rectangle(0, topY, getExtent().getWidth(), getExtent().getHeight()))) {
            setHoverLink(Document::nil);
        }
        sig_change.raise();
        requestRedraw();
    }
}

void
ui::rich::DocumentView::addTopY(int deltaY)
{
    setTopY(m_topY + deltaY);
}

// ScrollableWidget:
int
ui::rich::DocumentView::getPageTop() const
{
    // ex UIRichDocument::getTopY, sort-of
    return m_topY;
}

int
ui::rich::DocumentView::getPageSize() const
{
    return getExtent().getHeight();
}

int
ui::rich::DocumentView::getCursorTop() const
{
    return getPageTop();
}

int
ui::rich::DocumentView::getCursorSize() const
{
    return getPageSize();
}

int
ui::rich::DocumentView::getTotalSize() const
{
    return doc.getDocumentHeight();
}

void
ui::rich::DocumentView::setPageTop(int top)
{
    setTopY(top);
}

void
ui::rich::DocumentView::scroll(Operation op)
{
    switch (op) {
     case LineUp:
        addTopY(-m_provider.getFont(gfx::FontRequest())->getLineHeight());
        break;
     case LineDown:
        addTopY(+m_provider.getFont(gfx::FontRequest())->getLineHeight());
        break;
     case PageUp:
        addTopY(-getExtent().getHeight());
        break;
     case PageDown:
        addTopY(+getExtent().getHeight());
        break;
    }
}

void
ui::rich::DocumentView::handlePositionChange()
{
    // ex UIRichDocument::onResize
    doc.setPageWidth(getExtent().getWidth());

    // FIXME: original code would also reset m_topY to 0. Do we need that?
    sig_change.raise();

    requestRedraw();
    // FIXME: was ScrollableWidget::handlePositionChange(oldPosition);
}


// SimpleWidget:
void
ui::rich::DocumentView::draw(gfx::Canvas& can)
{
    // ex UIRichDocument::drawContent
    gfx::ClipFilter filter(can, getExtent());
    gfx::Context<util::SkinColor::Color> ctx(filter, getColorScheme());
    drawBackground(ctx, getExtent());
    doc.draw(ctx, getExtent(), getPageTop());

    if ((key_flags & fl_ScrollMark) != 0) {
        ctx.setColor(util::SkinColor::White);
        ctx.useFont(*m_provider.getFont(gfx::FontRequest()));
        if (getPageTop() > 0) {
            ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
            outText(ctx, getExtent().getTopRight(), UTF_UP_ARROW);
        }
        if (getPageTop() < getTotalSize() - getPageSize()) {
            ctx.setTextAlign(gfx::RightAlign, gfx::BottomAlign);
            outText(ctx, getExtent().getBottomRight(), UTF_DOWN_ARROW);
        }
    }
}

void
ui::rich::DocumentView::handleStateChange(State st, bool enable)
{
    // ex UIRichDocument::onStateChange
    if (st == ActiveState && !enable) {
        setHoverLink(Document::nil);
        mdown = false;
    }
}

ui::layout::Info
ui::rich::DocumentView::getLayoutInfo() const
{
    // ex UIRichDocument::getLayoutInfo (sort-of)
    return ui::layout::Info(m_minSize, pref_size, ui::layout::Info::GrowBoth);
}

bool
ui::rich::DocumentView::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex UIRichDocument::handleEvent (part)
    if ((key == ' ' && (key_flags & fl_Space) != 0) || (key == util::Key_Return && (key_flags & fl_Return) != 0)) {
        /* Return pressed */
        requestActive();
        mdown = false;
        if (selected_link != Document::nil) {
            sig_linkClick.raise(doc.getLinkTarget(selected_link));
        }
        return true;
    }
    if (key == util::Key_Tab && (key_flags & fl_Tab) != 0) {
        /* Next link */
        requestActive();
        mdown = false;
        gfx::Rectangle area(0, getPageTop(), getExtent().getWidth(), getExtent().getHeight());
        Document::LinkId_t link = doc.getNextLink(selected_link, area);
        if (link == Document::nil && selected_link != Document::nil) {
            link = doc.getNextLink(Document::nil, area);
        }
        setSelectedLink(link);
        return true;
    }
    if (key == util::KeyMod_Shift + util::Key_Tab && (key_flags & fl_Tab) != 0) {
        /* Previous link */
        requestActive();
        mdown = false;
        gfx::Rectangle area(0, getPageTop(), getExtent().getWidth(), getExtent().getHeight());
        Document::LinkId_t link = doc.getPreviousLink(selected_link, area);
        if (link == Document::nil && selected_link != Document::nil) {
            link = doc.getPreviousLink(Document::nil, area);
        }
        setSelectedLink(link);
        return true;
    }

    if ((key_flags & fl_Scroll) != 0) {
        int oldTopY = m_topY;
        switch (key) {
         case util::Key_Up:
         case util::Key_WheelUp:
            scroll(LineUp);
            break;
         case util::Key_Down:
         case util::Key_WheelDown:
            scroll(LineDown);
            break;
         case util::Key_PgUp:
            scroll(PageUp);
            break;
         case util::Key_PgDn:
         case ' ':
            scroll(PageDown);
            break;
         case util::Key_PgUp + util::KeyMod_Ctrl:
         case util::Key_Home + util::KeyMod_Ctrl:
         case '<':
            setTopY(0);
            break;
         case util::Key_PgDn + util::KeyMod_Ctrl:
         case util::Key_End + util::KeyMod_Ctrl:
         case '>':
            setTopY(doc.getDocumentHeight());
            break;
        }
        if (oldTopY != m_topY) {
            return true;
        }
    }

    return false;
}

bool
ui::rich::DocumentView::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // ex UIRichDocument::handleEvent (part)
    if (!hasState(DisabledState) && getExtent().contains(pt)) {
        requestActive();
        Document::LinkId_t link =
            doc.getLinkFromPos(gfx::Point(pt.getX() - getExtent().getLeftX(),
                                          pt.getY() - getExtent().getTopY() + m_topY));
        if (!pressedButtons.empty()) {
            if (link != Document::nil) {
                setSelectedLink(link);
            }
            setHoverLink(Document::nil);
            mdown = true;
        } else {
            if (mdown && selected_link != Document::nil && selected_link == link) {
                sig_linkClick.raise(doc.getLinkTarget(selected_link));
            }
            setHoverLink(link);
            mdown = false;
        }
        return true;
    }

    if (mdown && pressedButtons.empty()) {
        /* Mouse released outside the widget: just reset state */
        mdown = false;
    }
    return false;
}


void
ui::rich::DocumentView::setHoverLink(Document::LinkId_t link)
{
    // ex UIRichDocument::setHoverLink
    if (link != hover_link) {
        Document::LinkId_t old = hover_link;
        hover_link = link;
        setLink(old);
        setLink(hover_link);
        requestRedraw();
    }
}

void
ui::rich::DocumentView::setLink(Document::LinkId_t link)
{
    // ex UIRichDocument::setLink
    if (link != Document::nil) {
        doc.setLinkKind(link, (link == selected_link ? Document::LinkFocus
                               : link == hover_link ? Document::LinkHover
                               : Document::Link));
    }
}

int
ui::rich::DocumentView::getScrollStep() const
{
    // ex UIRichDocument::getScrollStep
    // FIXME: retire?
    // int n = font_heights[FONT_NORMAL];
    // if (n <= 0) {
    //     n = 1;
    // }
    // return n;
    return 1;
}
