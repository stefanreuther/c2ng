/**
  *  \file ui/prefixargument.cpp
  *  \brief Class ui::PrefixArgument
  *
  *  A prefix argument is directly associated with keypresses (parameter to handleKey).
  *
  *  For mouse events, the prefix argument is temporarily stored in ui::Root.
  *  Mouse handlers must poll the it themselves because after the initiating event (press),
  *  there can be many more mouse events (move) until an action is triggered (release).
  */

#include "ui/prefixargument.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "gfx/font.hpp"
#include "ui/colorscheme.hpp"
#include "ui/draw.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"
#include "util/prefixargument.hpp"

namespace ui { namespace {
    class PrefixPopup : public ui::SimpleWidget {
     public:
        PrefixPopup(int initialValue, ui::Root& root, ui::EventLoop& loop);

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);
     private:
        void update();
        void onTick();

        ui::Root& m_root;
        ui::EventLoop& m_loop;
        util::PrefixArgument m_logic;
        afl::base::Ref<gfx::Timer> m_blinkTimer;
        bool m_blink;
    };
} }

/*************************** Class PrefixPopup ***************************/

ui::PrefixPopup::PrefixPopup(int initialValue, Root& root, EventLoop& loop)
    : SimpleWidget(),
      m_root(root),
      m_loop(loop),
      m_logic(initialValue),
      m_blinkTimer(root.engine().createTimer()),
      m_blink(false)
{
    update();
    m_blinkTimer->sig_fire.add(this, &PrefixPopup::onTick);
    m_blinkTimer->setInterval(CURSOR_BLINK_INTERVAL);
}

void
ui::PrefixPopup::draw(gfx::Canvas& can)
{
    // ex UIPrefixPopup::drawContent
    const gfx::Rectangle& r = getExtent();
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());

    drawSolidBar(ctx, r, Color_Tooltip);

    ctx.setColor(Color_Tooltip_Shade);
    drawHLine(ctx, r.getLeftX(), r.getBottomY()-1, r.getRightX()-1);
    drawVLine(ctx, r.getRightX()-1, r.getTopY(), r.getBottomY()-2);

    ctx.setColor(Color_Tooltip_Light);
    drawHLine(ctx, r.getLeftX()+1, r.getTopY(), r.getRightX()-1);
    drawVLine(ctx, r.getLeftX(), r.getTopY(), r.getBottomY()-2);

    const afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().setStyle(FixedFont));
    ctx.setColor(Color_Tooltip_Text);
    ctx.useFont(*font);
    outText(ctx, gfx::Point(r.getLeftX() + 3, r.getTopY() + 1), m_logic.getText(afl::string::Translator::getSystemInstance()));

    if (!m_blink) {
        drawSolidBar(ctx, gfx::Rectangle(r.getRightX() - 10, r.getBottomY() - 5, 7, 2), Color_Black);
    }
}

void
ui::PrefixPopup::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::PrefixPopup::handlePositionChange()
{ }

ui::layout::Info
ui::PrefixPopup::getLayoutInfo() const
{
    return ui::layout::Info();
}

bool
ui::PrefixPopup::handleKey(util::Key_t key, int /*prefix*/)
{
    switch (m_logic.handleKey(key)) {
     case util::PrefixArgument::Accepted:
        update();
        return true;
     case util::PrefixArgument::Canceled:
        m_loop.stop(0);
        return true;
     case util::PrefixArgument::NotHandled:
        if (key == util::Key_Quit) {
            // Quit. Don't bother with the prefix.
            m_root.ungetKeyEvent(key, 0);
            m_loop.stop(0);
        } else if (util::classifyKey(key & ~util::KeyMod_Mask) == util::NormalKey) {
            // Possibly-defined key: re-post with prefix arg, and exit.
            m_root.ungetKeyEvent(key, m_logic.getValue());
            m_loop.stop(0);
        } else {
            // Modifier or virtual. Ignore.
        }
        return true;
    }
    return true;
}

bool
ui::PrefixPopup::handleMouse(gfx::Point /*pt*/, MouseButtons_t pressedButtons)
{
    // Mouse events with a pressed button cause the prefix argument to be accepted;
    // it will usually be associated with the next button release.
    if (!pressedButtons.empty()) {
        m_root.setMousePrefixArgument(m_logic.getValue());
        m_loop.stop(0);
        m_root.postMouseEvent();
    }
    return true;
}

/** Update position on screen.
    Computes the new position, moves the widget if needed, and requests redraw. */
void
ui::PrefixPopup::update()
{
    // ex UIPrefixPopup::update
    const String_t text = m_logic.getText(afl::string::Translator::getSystemInstance());
    const afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().setStyle(FixedFont));
    gfx::Rectangle area(0, 0, font->getTextWidth(text) + 14, font->getTextHeight(text) + 2);
    area.moveToEdge(m_root.getExtent(), gfx::CenterAlign, gfx::MiddleAlign, 0);
    setExtent(area);
    requestRedraw();
}

void
ui::PrefixPopup::onTick()
{
    m_blink = !m_blink;
    requestRedraw();
    m_blinkTimer->setInterval(CURSOR_BLINK_INTERVAL);
}



/***************************** PrefixArgument ****************************/

ui::PrefixArgument::PrefixArgument(Root& root)
    : InvisibleWidget(),
      m_root(root)
{
    // ex UIPrefixArg::UIPrefixArg
    setState(DisabledState, true);
}

bool
ui::PrefixArgument::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex UIPrefixArg::handleEvent
    // \change PCC2 did distinguish between first and second pass here (first pass only accepts Alt+Number key)
    const util::Key_t rawKey = key & ~util::KeyMod_Alt;
    if (rawKey >= '1' && rawKey <= '9') {
        showPopup(rawKey - '0');
        return true;
    } else {
        return false;
    }
}

void
ui::PrefixArgument::showPopup(int initialValue)
{
    // ex ui/prefix.h:startPrefixArg
    EventLoop loop(m_root);
    PrefixPopup popup(initialValue, m_root, loop);
    m_root.add(popup);
    loop.run();
    m_root.remove(popup);
}
