/**
  *  \file client/vcr/unitstatuswidget.cpp
  *
  *  \todo
  *  - make a left-aligned vs. right-aligned version (swap columns, right-aligned text)
  *  - implement fade-in/fade-out
  *  - consider some sort of ownership coloring (m_data.relation)
  *  - reconsider geometry constants
  */

#include <algorithm>
#include "client/vcr/unitstatuswidget.hpp"
#include "afl/string/format.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/colorscheme.hpp"

namespace {
    const int GAP = 5;
}


client::vcr::UnitStatusWidget::UnitStatusWidget(ui::Root& root, afl::string::Translator& tx)
    : m_root(root),
      m_translator(tx),
      m_data(),
      m_status(),
      conn_imageChange()
{
    conn_imageChange = root.provider().sig_imageChange.add(this, &UnitStatusWidget::onImageChange);
}

void
client::vcr::UnitStatusWidget::setData(const Data& data)
{
    m_data = data;
    m_status = Status();
    m_status.launcherStatus.resize(m_data.numLaunchers);
    m_status.beamStatus.resize(m_data.numBeams);
    m_image = m_root.provider().getImage(m_data.unitImageName);
}

void
client::vcr::UnitStatusWidget::setProperty(Property p, int value)
{
    if (int* pValue = getProperty(p)) {
        if (*pValue != value) {
            *pValue = value;
            requestRedraw();
        }
    }
}

void
client::vcr::UnitStatusWidget::addProperty(Property p, int delta)
{
    if (delta != 0) {
        if (int* pValue = getProperty(p)) {
            *pValue += delta;
            requestRedraw();
        }
    }
}

void
client::vcr::UnitStatusWidget::setWeaponLevel(Weapon w, int slot, int value)
{
    if (WeaponStatus* st = getWeapon(w, slot)) {
        st->actual = value;
        if (!st->blocked && st->actual != st->displayed) {
            st->displayed = st->actual;
            requestRedraw();
        }
    }
}

void
client::vcr::UnitStatusWidget::setWeaponStatus(Weapon w, int slot, bool blocked)
{
    if (WeaponStatus* st = getWeapon(w, slot)) {
        if (st->blocked != blocked) {
            st->blocked = blocked;
            if (!blocked && st->actual != st->displayed) {
                st->displayed = st->actual;
                requestRedraw();
            }
        }
    }
}

void
client::vcr::UnitStatusWidget::unblockAllWeapons()
{
    bool a = unblockWeapons(m_status.launcherStatus);
    bool b = unblockWeapons(m_status.beamStatus);
    if (a || b) {
        requestRedraw();
    }
}

void
client::vcr::UnitStatusWidget::draw(gfx::Canvas& can)
{
    gfx::Rectangle area = getExtent();
    getColorScheme().drawBackground(can, area);

    // Title
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.setColor(ui::Color_White);
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    ctx.setTransparentBackground();
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addSize(1)));
    outTextF(ctx, area.splitY(ctx.getFont()->getCellSize().getY()), m_data.unitName);

    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
    outTextF(ctx, area.splitY(ctx.getFont()->getCellSize().getY()), m_data.ownerName);

    area.consumeY(GAP);

    // Data
    drawMainColumn(can, area.splitX(100));
    area.consumeX(GAP);
    drawWeaponColumn(can, area.splitX(100));
}

void
client::vcr::UnitStatusWidget::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::vcr::UnitStatusWidget::handlePositionChange()
{ }

ui::layout::Info
client::vcr::UnitStatusWidget::getLayoutInfo() const
{
    // FIXME: more elaborate computation
    return gfx::Point(222, 222);
}

bool
client::vcr::UnitStatusWidget::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return false;
}

bool
client::vcr::UnitStatusWidget::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}

int*
client::vcr::UnitStatusWidget::getProperty(Property p)
{
    switch (p) {
     case NumFighters:  return &m_status.numFighters;
     case NumTorpedoes: return &m_status.numTorpedoes;
     case Shield:       return &m_status.shield;
     case Damage:       return &m_status.damage;
     case Crew:         return &m_status.crew;
    }
    return 0;
}

void
client::vcr::UnitStatusWidget::onImageChange()
{
    if (m_image.get() == 0) {
        m_image = m_root.provider().getImage(m_data.unitImageName);
        if (m_image.get() != 0) {
            requestRedraw();
        }
    }
}

void
client::vcr::UnitStatusWidget::drawMainColumn(gfx::Canvas& can, gfx::Rectangle r)
{
    // Prepare
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    afl::base::Ref<gfx::Font> normalFont = m_root.provider().getFont(gfx::FontRequest());
    afl::base::Ref<gfx::Font> boldFont = m_root.provider().getFont("b");
    const int lineHeight = normalFont->getCellSize().getY();

    // Ship image
    const gfx::Rectangle imageArea = r.splitY(100);
    if (m_image.get() != 0) {
        can.blit(imageArea.getTopLeft(), *m_image, gfx::Rectangle(gfx::Point(0, 0), imageArea.getSize()));
    }
    r.consumeY(GAP);

    // Shield level
    // ex WVcrShieldWidget::drawContent
    const gfx::Rectangle shieldArea = r.splitY(lineHeight);
    const int shield      = m_status.shield;
    const int shieldColor = std::min(ui::Color_Status + shield/6, ui::Color_Status+15);
    const int shieldWidth = shieldArea.getWidth() * std::min(100, std::max(0, shield)) / 100;
    drawSolidBar(ctx, gfx::Rectangle(shieldArea.getTopLeft(), gfx::Point(shieldWidth, shieldArea.getHeight())), uint8_t(shieldColor));

    // ctx.setColor(shield > 50 ? ui::Color_White : shield > 0 ? ui::Color_Gray : ui::Color_Dark);
    ctx.setColor(shield > 85 ? ui::Color_GreenBlack : shield > 50 ? ui::Color_White : shield > 0 ? ui::Color_Gray : ui::Color_Dark);
    ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
    ctx.useFont(shield > 100 ? *boldFont : *normalFont);
    outText(ctx, shieldArea.getCenter(), afl::string::Format(m_translator("Shields: %d%%").c_str(), shield));
    r.consumeY(GAP);

    // Damage/Crew
    // ex WVcrDamageWidget::drawContent
    ctx.setColor(ui::Color_White);
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    ctx.setTransparentBackground();
    ctx.useFont(*normalFont);
    outTextF(ctx, r.splitY(lineHeight), afl::string::Format(m_translator("Damage: %d%%").c_str(), m_status.damage));
    if (!m_data.isPlanet) {
        outTextF(ctx, r.splitY(lineHeight), afl::string::Format(m_translator("Crew: %d").c_str(), m_status.crew));
    }
    r.consumeY(GAP);
}

void
client::vcr::UnitStatusWidget::drawWeaponColumn(gfx::Canvas& can, gfx::Rectangle r)
{
    // Prepare
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addSize(-1));
    ctx.setColor(ui::Color_White);
    ctx.useFont(*font);
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    ctx.setTransparentBackground();
    const int lineHeight = font->getCellSize().getY();

    // Determine sizes
    int numLines = 0;
    int numBars = 0;
    if (m_data.numBeams > 0) {
        numLines += 1;
        numBars += m_data.numBeams;
    }
    if (m_data.numLaunchers > 0) {
        numLines += 2;
        numBars += m_data.numLaunchers;
    }
    if (m_data.numBays > 0) {
        numLines += 1;
    }

    int roomForBars = r.getHeight() - (lineHeight+GAP) * numLines;
    int barHeight;
    if (numBars > 0 && roomForBars > 0) {
        // Regular case
        barHeight = std::max(2, std::min(4, roomForBars / numBars));
    } else {
        // Border case (no bars, so value is irrelevant; or too little room)
        barHeight = 2;
    }

    // Draw beams
    // ex WVcrBeamDisplay::drawContent
    if (m_data.numBeams > 0) {
        outTextF(ctx, r.splitY(lineHeight), m_data.beamName);
        for (size_t i = 0, n = m_status.beamStatus.size(); i < n; ++i) {
            drawWeaponBar(ctx, r.splitY(barHeight), m_status.beamStatus[i].displayed);
        }
        r.consumeY(GAP);
    }

    // Draw launchers
    // ex WVcrTorpDisplay::drawContent
    if (m_data.numLaunchers > 0) {
        outTextF(ctx, r.splitY(lineHeight), m_data.launcherName);
        for (size_t i = 0, n = m_status.launcherStatus.size(); i < n; ++i) {
            drawWeaponBar(ctx, r.splitY(barHeight), m_status.launcherStatus[i].displayed);
        }
        // FIXME: PCC2 uses numToString
        outTextF(ctx, r.splitY(lineHeight), afl::string::Format(m_translator("Torpedoes: %d").c_str(), m_status.numTorpedoes));
        r.consumeY(GAP);
    }

    // Draw fighters
    // ex WVcrFighterDisplay::drawContent
    if (m_data.numBays > 0) {
        outTextF(ctx, r.splitY(lineHeight), afl::string::Format(m_translator("%d fighter bay%!1{s%}").c_str(), m_data.numBays));
        outTextF(ctx, r.splitY(lineHeight), afl::string::Format(m_translator("Fighters: %d").c_str(), m_status.numFighters));
    }
}

void
client::vcr::UnitStatusWidget::drawWeaponBar(gfx::Context<uint8_t>& ctx, gfx::Rectangle r, int level)
{
    // ex drawWeaponBar
    int effLevel = std::min(100, std::max(0, level));
    int color = std::min(ui::Color_Status + (effLevel >> 2), ui::Color_Status + 15);
    int width = r.getWidth() * effLevel / 100;
    drawSolidBar(ctx, gfx::Rectangle(r.getTopLeft(), gfx::Point(width, r.getHeight()-1)), uint8_t(color));
}


client::vcr::UnitStatusWidget::WeaponStatus*
client::vcr::UnitStatusWidget::getWeapon(Weapon w, int slot)
{
    switch (w) {
     case Launcher:
        return (slot >= 0 && slot < int(m_status.launcherStatus.size())
                ? &m_status.launcherStatus[slot]
                : 0);
     case Beam:
        return (slot >= 0 && slot < int(m_status.beamStatus.size())
                ? &m_status.beamStatus[slot]
                : 0);
    }
    return 0;
}

bool
client::vcr::UnitStatusWidget::unblockWeapons(std::vector<WeaponStatus>& w)
{
    bool result = false;
    for (size_t i = 0, n = w.size(); i < n; ++i) {
        WeaponStatus& ww = w[i];
        if (ww.blocked && ww.actual != ww.displayed) {
            ww.displayed = ww.actual;
            result = true;
        }
    }
    return result;
}
