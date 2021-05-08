/**
  *  \file client/map/keymapoverlay.cpp
  *  \brief Class client::map::KeymapOverlay
  */

#include "client/map/keymapoverlay.hpp"
#include "afl/string/format.hpp"
#include "client/map/screen.hpp"
#include "client/si/userside.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"


namespace {
    const int POPUP_TIMEOUT = 2000;
}

client::map::KeymapOverlay::KeymapOverlay(Screen& parent, String_t keymapName, int prefix)
    : m_parent(parent),
      m_keymapName(keymapName),
      m_prefix(prefix),
      m_proxy(parent.interface().gameSender(), parent.root().engine().dispatcher()),
      m_keys(),
      m_timer(parent.root().engine().createTimer()),
      m_shown(false)
{
    m_proxy.setListener(*this);
    m_proxy.setKeymapName(keymapName);
    m_timer->sig_fire.add(this, &KeymapOverlay::show);
    m_timer->setInterval(POPUP_TIMEOUT);
}

client::map::KeymapOverlay::~KeymapOverlay()
{ }

void
client::map::KeymapOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

void
client::map::KeymapOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WKeymapChartMode::drawOverlays
    if (m_shown) {
        afl::base::Ref<gfx::Font> font(m_parent.root().provider().getFont("b"));
        gfx::Context<uint8_t> ctx(can, m_parent.root().colorScheme());
        ctx.useFont(*font);

        String_t text = afl::string::Format(m_parent.translator()("Keymap %s"), m_keymapName);

        gfx::Rectangle r(0, 0, font->getTextWidth(text) + 6, font->getTextHeight(text) + 2);
        r.centerWithin(ren.getExtent());
        r.moveBy(gfx::Point(0, 20));

        drawSolidBar(ctx, r, ui::Color_Fire + 29);
        ctx.setColor(ui::Color_DarkYellow);
        drawHLine(ctx, r.getLeftX(), r.getBottomY()-1, r.getRightX()-1);
        drawVLine(ctx, r.getRightX()-1, r.getTopY(), r.getBottomY()-2);

        ctx.setColor(ui::Color_Fire + 30);
        drawHLine(ctx, r.getLeftX()+1, r.getTopY(), r.getRightX()-1);
        drawVLine(ctx, r.getLeftX(), r.getTopY(), r.getBottomY()-2);

        ctx.setColor(ui::Color_Black);
        ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
        outTextF(ctx, r, text);
    }
}

// ex WChartMode::drawCursor
bool
client::map::KeymapOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::KeymapOverlay::handleKey(util::Key_t key, int prefix, const Renderer& /*ren*/)
{
    // ex WKeymapChartMode::handleEvent
    if (m_keys.find(key) != m_keys.end()) {
        // This key was bound. Use it.
        // First, copy the parameters to local variables so we can delete ourselves.
        int arg = m_prefix;
        String_t keymapName = m_keymapName;
        Screen& parent = m_parent;
        parent.removeOverlay(this);
        parent.executeKeyCommandWait(keymapName, key, arg);
    } else if (key == util::Key_Escape) {
        // ESC, not bound in keymap
        m_parent.removeOverlay(this);
    } else if (key == util::Key_Quit) {
        // Quit
        ui::Root& r = m_parent.root();
        m_parent.removeOverlay(this);
        r.postKeyEvent(key, prefix);
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
client::map::KeymapOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    // Leave the mousing to the parent modes
    return false;
}

void
client::map::KeymapOverlay::updateKeyList(util::KeySet_t& keys)
{
    m_keys.swap(keys);
}

void
client::map::KeymapOverlay::show()
{
    if (!m_shown) {
        m_shown = true;
        if (Callback* cb = getCallback()) {
            cb->requestRedraw();
        }
    }
}
