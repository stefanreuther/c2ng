/**
  *  \file client/widgets/alliancelevelgrid.cpp
  */

#include <algorithm>
#include "client/widgets/alliancelevelgrid.hpp"
#include "afl/base/countof.hpp"
#include "afl/functional/stringtable.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "util/translation.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using gfx::Canvas;
using gfx::Font;
using gfx::Point;
using gfx::Rectangle;

namespace {
    /*
     * Layout:
     *    ---- Our offer
     *   |  +- Their offer
     *  [x][x] Level
     *  [x][x] Level
     *  [x][x] Level
     *  [x][x] Level
     *
     *  Checkboxes are 16x16, plus a border makes 20x20; UICheckbox reserves 24x24.
     *  Labels are in FONT_SMALL, so add 2 lines of that.
     */

    static const char*const LABEL_TEXT[] = {
        N_("Our offer"),
        N_("Their offer"),
    };
    static const int NUM_LABELS = countof(LABEL_TEXT);

    struct Metric {
        int gridSize;
        int labelHeight;
    };

    Metric getMetric(ui::Root& root)
    {
        Metric result;
        result.gridSize = 24;        // FIXME: hardcoded for now; this is what Checkbox uses
        result.labelHeight = root.provider().getFont(gfx::FontRequest().addSize(-1))->getCellSize().getY();
        return result;
    }
}


client::widgets::AllianceLevelGrid::AllianceLevelGrid(ui::Root& root, afl::string::Translator& tx)
    : SimpleWidget(),
      sig_toggleOffer(),
      m_root(root),
      m_translator(tx),
      m_items(),
      m_position(0),
      m_mouseDown(false)
{
    // ex WAllyLevelGrid::WAllyLevelGrid
}

void
client::widgets::AllianceLevelGrid::add(size_t ref, String_t name)
{
    m_items.push_back(Item(ref, name));
}

void
client::widgets::AllianceLevelGrid::setOffer(size_t ref, OfferType_t theirOffer, OfferType_t ourOffer)
{
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        Item& it = m_items[i];
        if (it.ref == ref) {
            if (it.theirOffer != theirOffer || it.ourOffer != ourOffer) {
                it.theirOffer = theirOffer;
                it.ourOffer = ourOffer;
                requestRedraw();
            }
        }
    }
}

void
client::widgets::AllianceLevelGrid::setPosition(size_t index)
{
    // ex WAllyLevelGrid::setPosition
    if (index >= m_items.size()) {
        index = m_items.size()-1;
    }
    if (index != m_position) {
        m_position = index;
        requestRedraw();
    }
}

void
client::widgets::AllianceLevelGrid::draw(gfx::Canvas& can)
{
    // ex WAllyLevelGrid::drawContent
    const Metric metric = getMetric(m_root);

    // Make a context
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());

    // Header
    const int x = getExtent().getLeftX();
    const int y = getExtent().getTopY();
    const Ref<Font> labelFont = m_root.provider().getFont(gfx::FontRequest().addSize(-1));
    ctx.useFont(*labelFont);
    for (int i = 0; i < NUM_LABELS; ++i) {
        ctx.setColor(util::SkinColor::Faded);
        drawHLine(ctx,
                  x + metric.gridSize/2 + i*metric.gridSize,
                  y + metric.labelHeight/2 + i*metric.labelHeight,
                  x + metric.gridSize*NUM_LABELS - 2);
        drawVLine(ctx,
                  x + metric.gridSize/2 + i*metric.gridSize,
                  y + metric.labelHeight/2 + i*metric.labelHeight,
                  y + metric.labelHeight*NUM_LABELS - 2);
        ctx.setColor(util::SkinColor::Static);
        outText(ctx, Point(x + metric.gridSize*NUM_LABELS, y + metric.labelHeight*i), m_translator(LABEL_TEXT[i]));
    }

    // Checkboxes and Labels
    const Ref<Font> itemFont = m_root.provider().getFont(gfx::FontRequest().addSize(+1));
    ctx.useFont(*itemFont);
    ctx.setTextAlign(0, 1);
    // const GAllianceLevels& levels = editAllies.getLevels();
    bool focus = getFocusState() != NoFocus;
    for (int i = 0, n = int(m_items.size()); i < n; ++i) {
        const int thisY = y + metric.labelHeight*NUM_LABELS + i*metric.gridSize;

        drawCheckbox(ctx, x,                   thisY, m_items[i].ourOffer,   metric.gridSize, focus && int(m_position) == i);
        drawCheckbox(ctx, x + metric.gridSize, thisY, m_items[i].theirOffer, metric.gridSize, false);

        ctx.setColor(util::SkinColor::Static);
        outText(ctx, Point(x + metric.gridSize*NUM_LABELS, thisY + metric.gridSize/2), m_items[i].name);
    }
}

void
client::widgets::AllianceLevelGrid::handleStateChange(State st, bool enable)
{
    // ex WAllyLevelGrid::onStateChange
    if (st == ActiveState && !enable) {
        // Forget mouse is down if anyone else claims it
        m_mouseDown = false;
    }
    if (st == FocusedState) {
        // Must redraw if focus changes
        requestRedraw();
    }
}

void
client::widgets::AllianceLevelGrid::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::widgets::AllianceLevelGrid::getLayoutInfo() const
{
    // ex WAllyLevelGrid::getLayoutInfo
    const Metric metric = getMetric(m_root);

    const Ref<Font> labelFont = m_root.provider().getFont(gfx::FontRequest().addSize(-1));
    int labelWidth = labelFont->getMaxTextWidth(afl::functional::createStringTable(LABEL_TEXT).map(m_translator));

    const Ref<Font> itemFont = m_root.provider().getFont(gfx::FontRequest().addSize(+1));
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        labelWidth = std::max(labelWidth, itemFont->getTextWidth(m_items[i].name));
    }

    return ui::layout::Info(Point(labelWidth + 2 * metric.gridSize,
                                  NUM_LABELS * metric.labelHeight + int(m_items.size()) * metric.gridSize));
}

bool
client::widgets::AllianceLevelGrid::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex WAllyLevelGrid::handleEvent
    if (hasState(FocusedState)) {
        switch (key) {
         case util::Key_Up:
            requestActive();
            if (m_position > 0) {
                setPosition(m_position - 1);
            }
            return true;

         case util::Key_Down:
            requestActive();
            setPosition(m_position + 1);
            return true;

         case util::Key_Home:
         case util::Key_PgUp:
            requestActive();
            setPosition(0);
            return true;

         case util::Key_End:
         case util::Key_PgDn:
            requestActive();
            setPosition(m_items.size());
            return true;

         case ' ':
            requestActive();
            toggleCurrent();
            return true;
        }
    }
    return false;
}

bool
client::widgets::AllianceLevelGrid::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // ex WAllyLevelGrid::handleEvent
    const Metric metric = getMetric(m_root);

    const int x = getExtent().getLeftX();
    const int y = getExtent().getTopY() + NUM_LABELS * metric.labelHeight;
    const int numLines = int(m_items.size());
    if (Rectangle(x, y, metric.gridSize, metric.gridSize * numLines).contains(pt)) {
        // Mouse is inside left column of checkboxes
        requestActive();

        size_t line = (pt.getY() - y) / metric.gridSize;
        if (!pressedButtons.empty()) {
            requestFocus();
            m_mouseDown = true;
            setPosition(line);
        } else {
            if (m_mouseDown) {
                toggleCurrent();
            }
            m_mouseDown = false;
        }
        return true;
    } else {
        // Do not eat this event.
        m_mouseDown = false;
        return false;
    }
}

void
client::widgets::AllianceLevelGrid::drawCheckbox(gfx::Context<util::SkinColor::Color>& ctx, int x, int y, OfferType_t offer, int gridSize, bool focused)
{
    using game::alliance::Offer;
    Ptr<Canvas> pix;
    switch (offer) {
     case Offer::Unknown:
     case Offer::No:
        pix = m_root.provider().getImage("ui.cb0");
        break;

     case Offer::Yes:
        pix = m_root.provider().getImage("ui.cb1");
        break;

     case Offer::Conditional:
        pix = m_root.provider().getImage("ui.cbc");
        break;
    }

    if (pix.get() != 0) {
        // FIXME: '16' is actually the size...
        blitPixmap(ctx, Point(x + (gridSize-16)/2, y + (gridSize-16)/2), *pix);
    }

    ctx.setColor(focused ? util::SkinColor::Static : util::SkinColor::Background);
    drawRectangle(ctx, Rectangle(x, y, gridSize, gridSize));
}

void
client::widgets::AllianceLevelGrid::toggleCurrent()
{
    sig_toggleOffer.raise(m_position);
}
