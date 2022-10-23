/**
  *  \file client/widgets/messageactionpanel.cpp
  *  \brief Class client::widgets::MessageActionPanel
  */

#include "client/widgets/messageactionpanel.hpp"
#include "afl/base/staticassert.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "util/unicodechars.hpp"

namespace {
    // FIXME: metrics should be derived from font sizes
    const int PAD = 5;
    const int GRID = 20;
    const int STEP = PAD+GRID;
    const size_t SPLIT = client::widgets::MessageActionPanel::Edit;
}

client::widgets::MessageActionPanel::LabeledButton::LabeledButton(ui::Root& root, util::Key_t key, String_t buttonLabel, String_t label)
    : button(buttonLabel, key, root),
      label(label),
      note()
{ }

client::widgets::MessageActionPanel::MessageActionPanel(ui::Root& root, afl::string::Translator& tx)
    : m_root(root),
      m_prevButton(UTF_UP_ARROW, util::Key_Up, root),
      m_nextButton(UTF_DOWN_ARROW, util::Key_Down, root),
      m_positionLabel(),
      m_positionDimmed(false),
      m_actions(),
      conn_imageChange(root.provider().sig_imageChange.add(this, (void (MessageActionPanel::*)()) &MessageActionPanel::requestRedraw))
{
    // WMessageActionPanel::WMessageActionPanel
    init(root, tx);
}

client::widgets::MessageActionPanel::~MessageActionPanel()
{ }

void
client::widgets::MessageActionPanel::enableAction(Action a, const String_t& note)
{
    // ex WMessageActionPanel::showWidget
    if (size_t(a) < m_actions.size()) {
        LabeledButton& lb = *m_actions[a];
        if (lb.button.getParent() == 0) {
            addChild(lb.button, 0);
            updatePositions();
        }
        if (lb.note != note) {
            lb.note = note;
            requestRedraw();
        }
    }
}

void
client::widgets::MessageActionPanel::disableAction(Action a)
{
    // ex WMessageActionPanel::hideWidget
    if (size_t(a) < m_actions.size()) {
        LabeledButton& lb = *m_actions[a];
        if (lb.button.getParent() != 0) {
            removeChild(lb.button);
            updatePositions();
        }
    }
}

void
client::widgets::MessageActionPanel::setPosition(String_t label, bool dim)
{
    if (m_positionLabel != label || m_positionDimmed != dim) {
        m_positionLabel = label;
        m_positionDimmed = dim;
        requestRedraw();
    }
}


bool
client::widgets::MessageActionPanel::handleKey(util::Key_t key, int prefix)
{
    // ex WMessageActionPanel::handleEvent
    return handleBuiltinKey(key, prefix) || defaultHandleKey(key, prefix);
}

bool
client::widgets::MessageActionPanel::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::widgets::MessageActionPanel::draw(gfx::Canvas& can)
{
    // ex WMessageActionPanel::drawContent
    // Panel
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    afl::base::Ptr<gfx::Canvas> tile = m_root.provider().getImage("bluetile");
    ui::drawTiledArea(ctx, getExtent(), tile, ui::Color_Shield+3, 0);
    ui::drawFrameUp(ctx, getExtent());
    if (tile.get() != 0) {
        conn_imageChange.disconnect();
    }

    // Position
    int posX1 = m_prevButton.getExtent().getRightX();
    int posX2 = m_nextButton.getExtent().getLeftX();
    gfx::Rectangle pos(posX1,         m_nextButton.getExtent().getTopY(),
                       posX2 - posX1, m_nextButton.getExtent().getHeight());
    ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
    if (m_positionDimmed) {
        ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
        ctx.setColor(ui::Color_Gray);
    } else {
        ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addWeight(1)));
        ctx.setColor(ui::Color_White);
    }
    outTextF(ctx, pos, m_positionLabel);

    // Button labels
    for (size_t i = 0, n = m_actions.size(); i < n; ++i) {
        LabeledButton& b = *m_actions[i];
        if (b.button.getParent() != 0) {
            int labelX1 = b.button.getExtent().getRightX() + PAD;
            int labelX2 = getExtent().getRightX() - PAD;
            gfx::Rectangle label(labelX1, b.button.getExtent().getTopY(),
                                 labelX2 - labelX1, b.button.getExtent().getHeight());
            ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addSize(-1)));
            ctx.setColor(ui::Color_Gray);
            ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
            if (b.note.empty()) {
                outTextF(ctx, label, b.label);
            } else {
                outTextF(ctx, label.splitY(label.getHeight()/2), b.label);
                ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addSize(-1).addWeight(+1)));
                ctx.setColor(ui::Color_White);
                outTextF(ctx, label, b.note);
            }
        }
    }

    // Buttons
    defaultDrawChildren(can);
}

void
client::widgets::MessageActionPanel::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::MessageActionPanel::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::widgets::MessageActionPanel::handleChildAdded(Widget& /*child*/)
{
    requestRedraw();
}

void
client::widgets::MessageActionPanel::handleChildRemove(Widget& /*child*/)
{
    requestRedraw();
}

void
client::widgets::MessageActionPanel::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    updatePositions();
    requestRedraw();
}

void
client::widgets::MessageActionPanel::handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::widgets::MessageActionPanel::getLayoutInfo() const
{
    // ex WMessageActionPanel::getLayoutInfo
    // 1 em FONT_TITLE     = 18 px, so we need 2.7 em for the buttons.
    // 1 digit FONT_NORMAL = 8 px, so we need 2.7 em for 6 digits.
    gfx::Point size = m_root.provider().getFont(gfx::FontRequest().addSize(1))->getCellSize().scaledBy(7, 10);
    return ui::layout::Info(size, size, ui::layout::Info::GrowBoth);
}

void
client::widgets::MessageActionPanel::init(ui::Root& root, afl::string::Translator& tx)
{
    // Create all buttons
    static_assert(GoTo1 == 0, "GoTo1");
    m_actions.pushBackNew(new LabeledButton(root, 'g', "G", tx("Go to")));

    static_assert(GoTo2 == 1, "GoTo2");
    m_actions.pushBackNew(new LabeledButton(root, 'x', "X", tx("Go to")));

    static_assert(Reply == 2, "Reply");
    m_actions.pushBackNew(new LabeledButton(root, 'r', "R", tx("Reply to")));

    static_assert(Confirm == 3, "Confirm");
    m_actions.pushBackNew(new LabeledButton(root, 'c', "C", tx("Confirm")));

    static_assert(Accept == 4, "Accept");
    m_actions.pushBackNew(new LabeledButton(root, 'a', "A", tx("Accept")));

    static_assert(Edit == 5, "Edit");
    m_actions.pushBackNew(new LabeledButton(root, 'e', "E", tx("Edit...")));

    static_assert(Redirect == 6, "Redirect");
    m_actions.pushBackNew(new LabeledButton(root, 't', "T", tx("To...")));

    static_assert(Delete == 7, "Delete");
    m_actions.pushBackNew(new LabeledButton(root, util::Key_Delete, "Del", tx("Delete")));

    static_assert(Forward == 8, "Forward");
    m_actions.pushBackNew(new LabeledButton(root, 'f', "F", tx("Forward...")));

    static_assert(Search == 9, "Search");
    m_actions.pushBackNew(new LabeledButton(root, 's', "S", tx("Search...")));

    static_assert(Write == 10, "Write");
    m_actions.pushBackNew(new LabeledButton(root, 'w', "W", tx("Write to file...")));

    // Add pager buttons
    addChild(m_prevButton, 0);
    addChild(m_nextButton, 0);

    // Enable default buttons
    enableAction(Forward, String_t());
    enableAction(Search, String_t());
    enableAction(Write, String_t());

    // Observe everything
    m_prevButton.sig_fireKey.add(this, &MessageActionPanel::onKey);
    m_nextButton.sig_fireKey.add(this, &MessageActionPanel::onKey);
    for (size_t i = 0, n = m_actions.size(); i < n; ++i) {
        m_actions[i]->button.sig_fireKey.add(this, &MessageActionPanel::onKey);
    }
}

void
client::widgets::MessageActionPanel::updatePositions()
{
    // ex WMessageActionPanel::updateWidgets
    int x = getExtent().getLeftX() + PAD;
    int y = getExtent().getTopY() + PAD;

    // Arrows
    m_prevButton.setExtent(gfx::Rectangle(x, y, GRID, GRID));
    m_nextButton.setExtent(gfx::Rectangle(x + getExtent().getWidth() - 2*PAD - GRID, y, GRID, GRID));
    y += STEP;

    // Top buttons
    for (size_t i = 0; i < SPLIT && i < m_actions.size(); ++i) {
        ui::Widget& w = m_actions[i]->button;
        if (w.getParent() != 0) {
            w.setExtent(gfx::Rectangle(x, y, GRID, GRID));
            y += STEP;
        }
    }

    // Bottom buttons
    y = getExtent().getBottomY();
    for (size_t i = m_actions.size(); i > SPLIT; --i) {
        ui::Widget& w = m_actions[i-1]->button;
        if (w.getParent() != 0) {
            y -= STEP;
            w.setExtent(gfx::Rectangle(x, y, (i-1 == Delete ? 7*GRID/4 : GRID), GRID));
        }
    }
}

void
client::widgets::MessageActionPanel::onKey(int arg, util::Key_t key)
{
    // Note swapped arguments!
    handleBuiltinKey(key, arg);
}

bool
client::widgets::MessageActionPanel::handleBuiltinKey(util::Key_t key, int arg)
{
    // ex WMessageDisplay::handleEvent
    using util::KeyMod_Shift;
    using util::KeyMod_Ctrl;
    const util::Key_t rawKey = key & ~(KeyMod_Shift | KeyMod_Ctrl);
    const bool        ctrl   = (key & KeyMod_Ctrl) != 0;
    const bool        shift  = (key & KeyMod_Shift) != 0;
    switch (rawKey) {
     case util::Key_PgUp:
     case util::Key_Up:
     case util::Key_WheelUp:
     case '-':
        doAction(ctrl
                 ? (shift ? BrowseFirstAll    : BrowseFirst)
                 : (shift ? BrowsePreviousAll : BrowsePrevious),
                 arg);
        return true;

     case util::Key_PgDn:
     case util::Key_Down:
     case util::Key_WheelDown:
     case '+':
        doAction(ctrl
                 ? (shift ? BrowseLastAll : BrowseLast)
                 : (shift ? BrowseNextAll : BrowseNext),
                 arg);
        return true;

     case util::Key_Home:
     case '<':
     case '=':
        doAction(arg != 0
                 ? BrowseNth
                 : (shift ? BrowseFirstAll : BrowseFirst),
                 arg);
        return true;

     case util::Key_End:
     case '>':
        doAction(arg != 0
                 ? BrowseNth
                 : (shift ? BrowseLastAll : BrowseLast),
                 arg);
        return true;

     case 's':
     case '/':
        doAction(Search, arg);
        return true;

     case 'n':
        doAction(SearchNext, arg);
        return true;

     case util::Key_F7:
        if (!ctrl) {
            doAction(shift ? SearchNext : Search, arg);
            return true;
        } else {
            return false;
        }

     case 'w':
        doAction(ctrl ? WriteAll : Write, arg);
        return true;

     case 'r':
        doAction(ctrl ? ReplyAll : Reply, arg);
        return true;

        // FIXME: port this: // case SDLK_RETURN:
        //    /* Do what I mean: Reply is default unless it would go to
        //       the host and we have other options. This is because
        //       PHost sends wormhole messages as (-h) by default. */
        //    /* FIXME: what do we do with object transfers? Probably
        //       the user doesn't want to reply to these. */
        //    if (haveReply() && !((haveObject() || haveXY()) && getReply() == 12)) {
        //        doReply();
        //    } else if (haveObject()) {
        //        doGoToObject();
        //    } else {
        //        doGoToXY();
        //    }
        //    return true;

     case util::Key_Tab:
        doAction(BrowseSubjects, arg);
        return true;

     default:
        if (!ctrl) {
            for (size_t i = 0, n = m_actions.size(); i < n; ++i) {
                LabeledButton& b = *m_actions[i];
                if (b.button.getParent() != 0 && b.button.getKey() == rawKey) {
                    doAction(Action(i), arg);
                    return true;
                }
            }
        }
        return false;
    }
}

void
client::widgets::MessageActionPanel::doAction(Action a, int arg)
{
    requestActive();
    sig_action.raise(a, arg);
}
