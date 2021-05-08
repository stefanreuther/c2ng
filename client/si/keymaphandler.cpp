/**
  *  \file client/si/keymaphandler.cpp
  *  \brief Class client::si::KeymapHandler
  */

#include "client/si/keymaphandler.hpp"
#include "afl/string/format.hpp"
#include "client/si/userside.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"

namespace {
    const int POPUP_TIMEOUT = 2000;
}


client::si::KeymapHandler::KeymapHandler(Control& parentControl, String_t name, int prefix)
    : Control(parentControl.interface(), parentControl.root(), parentControl.translator()),
      m_parentControl(parentControl),
      m_timer(parentControl.root().engine().createTimer()),
      m_keymapName(name),
      m_prefix(prefix),
      m_proxy(parentControl.interface().gameSender(), parentControl.root().engine().dispatcher()),
      m_keys(),
      m_shown(false),
      m_result(),
      m_loop(parentControl.root())
{
    // ex WKeymapHandler::WKeymapHandler
    m_timer->sig_fire.add(this, &KeymapHandler::show);
    m_timer->setInterval(POPUP_TIMEOUT);
    m_proxy.setListener(*this);
    requestKeys();
}

client::si::KeymapHandler::~KeymapHandler()
{ }

client::si::KeymapHandler::Result
client::si::KeymapHandler::run(RequestLink2 link)
{
    // Show myself
    root().add(*this);

    // Continue inbound process (the process that called UseKeymap)
    continueProcessWait(link);

    // Wait for something to happen; everything that stops the loop sets m_result.
    m_loop.run();

    // Hide myself (optional, caller is supposed to delete this)
    root().removeChild(*this);

    return m_result;
}

// Widget:
void
client::si::KeymapHandler::draw(gfx::Canvas& can)
{
    // ex WKeymapHandler::drawContent
    if (m_shown) {
        gfx::Rectangle r = getExtent();
        gfx::Context<uint8_t> ctx(can, root().colorScheme());

        drawSolidBar(ctx, r, ui::Color_Fire + 29);

        ctx.setColor(ui::Color_DarkYellow);
        drawHLine(ctx, r.getLeftX(), r.getBottomY()-1, r.getRightX()-1);
        drawVLine(ctx, r.getRightX()-1, r.getTopY(), r.getBottomY()-2);

        ctx.setColor(ui::Color_Fire + 30);
        drawHLine(ctx, r.getLeftX()+1, r.getTopY(), r.getRightX()-1);
        drawVLine(ctx, r.getLeftX(), r.getTopY(), r.getBottomY()-2);

        ctx.setColor(ui::Color_Black);
        ctx.useFont(*getFont());
        ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
        outTextF(ctx, r, getText());
    }
}

void
client::si::KeymapHandler::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::si::KeymapHandler::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
client::si::KeymapHandler::getLayoutInfo() const
{
    // We're doing our own layout
    return ui::layout::Info();
}

bool
client::si::KeymapHandler::handleKey(util::Key_t key, int prefix)
{
    // ex WKeymapHandler::handleEvent
    // An inbound prefix is ignored because this widget cannot create a new one;
    // we use the prefix provided by the invoking UseKeymap command.
    if (m_keys.find(key) != m_keys.end()) {
        // Key from keymap
        m_result.action = KeyCommand;
        m_result.key = key;
        m_result.keymapName = m_keymapName;
        m_result.prefix = m_prefix;
        m_loop.stop(0);
    } else if (key == util::Key_Escape) {
        // ESC, not bound in keymap
        m_loop.stop(0);
    } else if (key == util::Key_Quit) {
        // Quit (emulate Quit widget)
        root().ungetKeyEvent(key, prefix);
        m_loop.stop(0);
    } else if (util::classifyKey(key & util::Key_Mask) == util::NormalKey) {
        // Not bound, but something that looks like a key (i.e. not a modifier).
        show();
    } else {
        // Something else, e.g. Shift.
    }

    // Swallow all keys!
    return true;
}

bool
client::si::KeymapHandler::handleMouse(gfx::Point /*pt*/, MouseButtons_t pressedButtons)
{
    // ex WKeymapHandler::handleEvent
    if (!pressedButtons.empty()) {
        // Mouse pressed: discard prefix, call original handler
        root().postMouseEvent();
        m_loop.stop(0);
    }
    return true;
}

// Control:
void
client::si::KeymapHandler::handleStateChange(RequestLink2 link, OutputState::Target target)
{
    // This is called if UseKeymap, UI.GotoScreen are called after another.
    m_result.action = StateChange;
    m_result.target = target;
    m_result.link = link;
    m_loop.stop(0);
}

void
client::si::KeymapHandler::handleEndDialog(RequestLink2 link, int code)
{
    // This is called if UseKeymap, UI.EndDialog are called after another.
    m_result.action = EndDialog;
    m_result.code = code;
    m_result.link = link;
    m_loop.stop(0);
}

void
client::si::KeymapHandler::handlePopupConsole(RequestLink2 link)
{
    // This is called if UseKeymap, UI.PopupConsole are called after another.
    m_result.action = PopupConsole;
    m_result.link = link;
    m_loop.stop(0);
}

void
client::si::KeymapHandler::handleSetViewRequest(RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetViewRequest(link, name, withKeymap);
}

void
client::si::KeymapHandler::handleUseKeymapRequest(RequestLink2 link, String_t name, int prefix)
{
    // Handle internally
    // This is called if UseKeymap, UseKeymap is called twice
    m_keymapName = name;
    m_prefix = prefix;
    requestKeys();

    // Re-enter show(); this will set layout for the new name.
    show();

    // Continue inbound process.
    // This is tricky; see comment in continueProcessWait().
    // If we are already waiting, this will not actually wait to avoid recursive frames
    // of handleUseKeymapRequest > continueProcessWait piling up.
    continueProcessWait(link);
}

void
client::si::KeymapHandler::handleOverlayMessageRequest(RequestLink2 link, String_t text)
{
    defaultHandleOverlayMessageRequest(link, text);
}

client::si::ContextProvider*
client::si::KeymapHandler::createContextProvider()
{
    return m_parentControl.createContextProvider();
}

void
client::si::KeymapHandler::updateKeyList(util::KeySet_t& keys)
{
    m_keys = keys;
}

void
client::si::KeymapHandler::show()
{
    // ex WKeymapHandler::show
    // I am now visible
    m_shown = true;

    // Set layout
    const String_t text = getText();
    afl::base::Ref<gfx::Font> font = getFont();
    gfx::Rectangle r(0, 0, font->getTextWidth(text) + 6, font->getTextHeight(text) + 2);
    r.centerWithin(root().getExtent());
    setExtent(r);
    requestRedraw();
}

void
client::si::KeymapHandler::requestKeys()
{
    m_proxy.setKeymapName(m_keymapName);
}

String_t
client::si::KeymapHandler::getText()
{
    // ex WKeymapHandler::getText
    return afl::string::Format(translator()("Keymap %s"), m_keymapName);
}

afl::base::Ref<gfx::Font>
client::si::KeymapHandler::getFont()
{
    return root().provider().getFont("b");
}
