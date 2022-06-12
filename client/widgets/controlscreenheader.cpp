/**
  *  \file client/widgets/controlscreenheader.cpp
  *
  *  FIXME: this uses too many fixed dimensions
  */

#include "client/widgets/controlscreenheader.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/staticassert.hpp"
#include "client/marker.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/widgets/button.hpp"
#include "util/unicodechars.hpp"
#include "util/updater.hpp"
#include "gfx/complex.hpp"

using util::Updater;

namespace {
    struct ButtonSpec {
        char text[7];           // Button text (inline is shorter than pointer :)
        bool defaultEnabled;    // true if button is enabled by default. Set for all buttons that appear on every screen.
        util::Key_t key;        // Key.
        int x, y, w, h;         // Relative coordinates within widget.
    };

    const ButtonSpec BUTTONS[] = {
        { "F1",           true,  util::Key_F1,       5,  45,  30,  25 },
        { "F2",           true,  util::Key_F2,      40,  45,  30,  25 },
        { "F3",           true,  util::Key_F3,       5,  75,  30,  25 },
        { "F4",           true,  util::Key_F4,      40,  75,  30,  25 },
        { "F6",           true,  util::Key_F6,       5, 105,  30,  25 },
        { "F7",           true,  util::Key_F7,      40, 105,  30,  25 },
        { UTF_UP_ARROW,   true,  '-',               75,  45,  20,  25 },
        { UTF_DOWN_ARROW, true,  '+',               75,  75,  20,  25 },
        { "I",            false, 'i',               75, 105,  20,  25 },
        { "Auto",         false, util::Key_Return, 230,  45,  50,  25 },
        { "CScr",         false, util::Key_Return, 230,  45,  50,  25 },
        { "X",            true,  'x',              285,  45,  25,  25 },
        { "Add",          false, util::Key_Insert, 230,  75,  40,  25 },
        { UTF_TAB_ARROW,  false, util::Key_Tab,    275,  75,  35,  25 },
        { "J",            false, 'j',              285,  75,  25,  25 },
        { "H",            true,  'h',              230, 114,  25,  25 },
        { "ESC",          true,  util::Key_Escape, 265, 114,  45,  25 },
        { "N",            false, 'n',              295,   0,  20,  20 },
        { "<img>",        true,  '.',              108,  45, 107,  95 },
    };
}

class client::widgets::ControlScreenHeader::TitleWidget : public ui::SimpleWidget {
 public:
    TitleWidget(ui::Root& root);

    void setText(Text which, String_t value);
    void setHasMessages(bool flag);

    // SimpleWidget:
    virtual void draw(gfx::Canvas& can);
    virtual void handleStateChange(State st, bool enable);
    virtual void handlePositionChange(gfx::Rectangle& oldPosition);
    virtual ui::layout::Info getLayoutInfo() const;
    virtual bool handleKey(util::Key_t key, int prefix);
    virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

 private:
    ui::Root& m_root;
    String_t m_text[NUM_TEXTS];
    bool m_hasMessages;
};

client::widgets::ControlScreenHeader::TitleWidget::TitleWidget(ui::Root& root)
    : m_root(root),
      m_hasMessages(false)
{ }

void
client::widgets::ControlScreenHeader::TitleWidget::setText(Text which, String_t value)
{
    if (Updater().set(m_text[which], value)) {
        requestRedraw();
    }
}

void
client::widgets::ControlScreenHeader::TitleWidget::setHasMessages(bool flag)
{
    if (Updater().set(m_hasMessages, flag)) {
        requestRedraw();
    }
}

// SimpleWidget:
void
client::widgets::ControlScreenHeader::TitleWidget::draw(gfx::Canvas& can)
{
    gfx::Rectangle area(getExtent());
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
    drawBackground(ctx, area);

    // Title
    gfx::Rectangle titleArea = area.splitY(25);
    afl::base::Ref<gfx::Font> titleFont = m_root.provider().getFont("+");
    ctx.useFont(*titleFont);
    ctx.setColor(util::SkinColor::Heading);
    int w = titleFont->getTextWidth(m_text[txtHeading]);
    outTextF(ctx, titleArea.splitX(w), m_text[txtHeading]);

    // Symbol
    const int SYM_WIDTH = 10;
    if (m_hasMessages && titleArea.getWidth() >= SYM_WIDTH) {
        int lineHeight = titleFont->getLineHeight();
        ctx.setColor(util::SkinColor::Blue);
        drawMessageMarker(ctx,
                          gfx::Point(titleArea.getLeftX() + 5,
                                     titleArea.getTopY() + lineHeight*8/10),
                          5*lineHeight,
                          3*lineHeight);
    }

    // Subtitle
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
    ctx.setColor(util::SkinColor::Yellow);
    outTextF(ctx, area, m_text[txtSubtitle]);    
}

void
client::widgets::ControlScreenHeader::TitleWidget::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::ControlScreenHeader::TitleWidget::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
client::widgets::ControlScreenHeader::TitleWidget::getLayoutInfo() const
{
    return gfx::Point(293, 25+16);
}

bool
client::widgets::ControlScreenHeader::TitleWidget::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::widgets::ControlScreenHeader::TitleWidget::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}





/*
 *  ControlScreenHeader
 */


client::widgets::ControlScreenHeader::ControlScreenHeader(ui::Root& root, gfx::KeyEventConsumer& kmw)
    : m_deleter(),
      m_visibleButtons()
{
    // ex WControlScreenHeaderTile::WControlScreenHeaderTile
    createChildWidgets(root, kmw);
    setChildWidgetPositions();

    // Disable so it doesn't get focus (and the TaskEditorTile gets it instead)
    // FIXME: should we have an opt-in FocusableState instead?
    setState(DisabledState, true);
}

client::widgets::ControlScreenHeader::~ControlScreenHeader()
{ }

void
client::widgets::ControlScreenHeader::enableButton(Button btn, ui::FrameType type)
{
    if (m_frames[btn] != 0) {
        m_frames[btn]->setType(type);
        if (!m_visibleButtons.contains(btn)) {
            addChild(*m_frames[btn], 0);
            m_visibleButtons += btn;
        }
    }
}

void
client::widgets::ControlScreenHeader::disableButton(Button btn)
{
    if (m_frames[btn] != 0) {
        if (m_visibleButtons.contains(btn)) {
            removeChild(*m_frames[btn]);
            m_visibleButtons -= btn;
        }
    }
}

void
client::widgets::ControlScreenHeader::setText(Text which, String_t value)
{
    if (m_title) {
        m_title->setText(which, value);
    }
}

void
client::widgets::ControlScreenHeader::setImage(String_t name)
{
    if (m_image) {
        m_image->setImage(name);
    }
}

void
client::widgets::ControlScreenHeader::setHasMessages(bool flag)
{
    if (m_title) {
        m_title->setHasMessages(flag);
    }
}


// Widget:
void
client::widgets::ControlScreenHeader::draw(gfx::Canvas& can)
{
    defaultDrawChildren(can);
}

void
client::widgets::ControlScreenHeader::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::ControlScreenHeader::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::widgets::ControlScreenHeader::handleChildAdded(Widget& /*child*/)
{ }

void
client::widgets::ControlScreenHeader::handleChildRemove(Widget& /*child*/)
{ }

void
client::widgets::ControlScreenHeader::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    setChildWidgetPositions();
}

void
client::widgets::ControlScreenHeader::handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
client::widgets::ControlScreenHeader::getLayoutInfo() const
{
    // ex WControlScreenHeaderTile::WControlScreenHeaderTile (part)
    return ui::layout::Info(gfx::Point(315, 145));
}

bool
client::widgets::ControlScreenHeader::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::widgets::ControlScreenHeader::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::widgets::ControlScreenHeader::createChildWidgets(ui::Root& root, gfx::KeyEventConsumer& kmw)
{
    // Create buttons
    static_assert(countof(BUTTONS) == NUM_BUTTONS, "countof BUTTONS");
    for (size_t i = 0; i < NUM_BUTTONS; ++i) {
        // Button
        ui::Widget* w;
        if (i == btnImage) {
            ui::widgets::ImageButton& btn = m_deleter.addNew(new ui::widgets::ImageButton(String_t(), BUTTONS[i].key, root, gfx::Point(105, 95)));
            ui::widgets::FrameGroup& innerFrame = m_deleter.addNew(new ui::widgets::FrameGroup(ui::layout::HBox::instance0, root.colorScheme(), ui::LoweredFrame));
            innerFrame.add(btn);
            btn.dispatchKeyTo(kmw);
            w = &innerFrame;
            m_image = &btn;
        } else {
            ui::widgets::Button& btn = m_deleter.addNew(new ui::widgets::Button(BUTTONS[i].text, BUTTONS[i].key, root));
            btn.dispatchKeyTo(kmw);
            w = &btn;
        }

        // FrameGroup
        m_frames[i] = &m_deleter.addNew(new ui::widgets::FrameGroup(ui::layout::HBox::instance0, root.colorScheme(), ui::NoFrame));
        m_frames[i]->setFrameWidth(2);
        m_frames[i]->add(*w);

        if (BUTTONS[i].defaultEnabled) {
            addChild(*m_frames[i], 0);
            m_visibleButtons += Button(i);
        }
    }

    // Create title
    m_title = &m_deleter.addNew(new TitleWidget(root));
    addChild(*m_title, 0);
}

void
client::widgets::ControlScreenHeader::setChildWidgetPositions()
{
    // ex WControlScreenHeaderTile::init, sort-of
    for (size_t i = 0; i < NUM_BUTTONS; ++i) {
        setChildPosition(m_frames[i], BUTTONS[i].x-2, BUTTONS[i].y-2, BUTTONS[i].w+4, BUTTONS[i].h+4);
    }
    setChildPosition(m_title, 0, 0, 293, 25+16);
}

void
client::widgets::ControlScreenHeader::setChildPosition(ui::Widget* widget, int x, int y, int w, int h)
{
    if (widget != 0) {
        gfx::Point origin = getExtent().getTopLeft();
        widget->setExtent(gfx::Rectangle(x + origin.getX(), y + origin.getY(), w, h));
    }
}

ui::FrameType
client::widgets::getFrameTypeFromTaskStatus(game::Session::TaskStatus st)
{
    // FIXME: where to place this?
    using game::Session;
    switch (st) {
     case Session::NoTask:      return ui::NoFrame;
     case Session::ActiveTask:  return ui::GreenFrame;
     case Session::WaitingTask: return ui::RedFrame;
     case Session::OtherTask:   return ui::YellowFrame;
    }
    return ui::NoFrame;
}
