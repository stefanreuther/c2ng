/**
  *  \file client/widgets/controlscreenheader.cpp
  */

#include "client/widgets/controlscreenheader.hpp"
#include "afl/base/staticassert.hpp"
#include "util/unicodechars.hpp"
#include "afl/base/countof.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/widgets/button.hpp"


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

client::widgets::ControlScreenHeader::ControlScreenHeader(ui::Root& root, KeymapWidget& kmw)
    : m_deleter(),
      m_visibleButtons()
{
    // ex WControlScreenHeaderTile::WControlScreenHeaderTile
    createChildWidgets(root, kmw);
    setChildWidgetPositions();
}

client::widgets::ControlScreenHeader::~ControlScreenHeader()
{ }

void
client::widgets::ControlScreenHeader::enableButton(Button btn, ui::widgets::FrameGroup::Type type)
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
    if (m_texts[which]) {
        m_texts[which]->setText(value);
    }
}

void
client::widgets::ControlScreenHeader::setImage(String_t name)
{
    if (m_image) {
        m_image->setImage(name);
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
{
}

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
    // FIXME: magic numbers
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
client::widgets::ControlScreenHeader::createChildWidgets(ui::Root& root, KeymapWidget& kmw)
{
    // Create buttons
    static_assert(countof(BUTTONS) == NUM_BUTTONS, "countof BUTTONS");
    for (size_t i = 0; i < NUM_BUTTONS; ++i) {
        // Button
        ui::Widget* w;
        if (i == btnImage) {
            ui::widgets::ImageButton& btn = m_deleter.addNew(new ui::widgets::ImageButton(String_t(), BUTTONS[i].key, root, gfx::Point(105, 95)));
            ui::widgets::FrameGroup& innerFrame = m_deleter.addNew(new ui::widgets::FrameGroup(ui::layout::HBox::instance0, root.colorScheme(), ui::widgets::FrameGroup::LoweredFrame));
            innerFrame.add(btn);
            kmw.addButton(btn);
            w = &innerFrame;
            m_image = &btn;
        } else {
            ui::widgets::Button& btn = m_deleter.addNew(new ui::widgets::Button(BUTTONS[i].text, BUTTONS[i].key, root));
            kmw.addButton(btn);
            w = &btn;
        }

        // FrameGroup
        m_frames[i] = &m_deleter.addNew(new ui::widgets::FrameGroup(ui::layout::HBox::instance0, root.colorScheme(), ui::widgets::FrameGroup::NoFrame));
        m_frames[i]->setFrameWidth(2);
        m_frames[i]->add(*w);

        if (BUTTONS[i].defaultEnabled) {
            addChild(*m_frames[i], 0);
            m_visibleButtons += Button(i);
        }
    }

    // Create text
    static_assert(NUM_TEXTS == 2, "NUM_TEXTS");
    m_texts[0] = &m_deleter.addNew(new ui::widgets::StaticText("", util::SkinColor::Heading, gfx::FontRequest().addSize(1), root.provider(), 0));
    m_texts[1] = &m_deleter.addNew(new ui::widgets::StaticText("", util::SkinColor::Yellow,  gfx::FontRequest(),            root.provider(), 0));
    addChild(*m_texts[0], 0);
    addChild(*m_texts[1], 0);
}

void
client::widgets::ControlScreenHeader::setChildWidgetPositions()
{
    // ex WControlScreenHeaderTile::init, sort-of
    for (size_t i = 0; i < NUM_BUTTONS; ++i) {
        setChildPosition(m_frames[i], BUTTONS[i].x-2, BUTTONS[i].y-2, BUTTONS[i].w+4, BUTTONS[i].h+4);
    }

    static_assert(NUM_TEXTS == 2, "NUM_TEXTS");
    setChildPosition(m_texts[0], 0,  0, 293, 24);
    setChildPosition(m_texts[1], 0, 25, 293, 16);
}

void
client::widgets::ControlScreenHeader::setChildPosition(ui::Widget* widget, int x, int y, int w, int h)
{
    if (widget != 0) {
        gfx::Point origin = getExtent().getTopLeft();
        widget->setExtent(gfx::Rectangle(x + origin.getX(), y + origin.getY(), w, h));
    }
}
