/**
  *  \file client/widgets/planetmineralinfo.cpp
  *  \brief Class client::widgets::PlanetMineralInfo
  */

#include "client/widgets/planetmineralinfo.hpp"
#include "afl/functional/stringtable.hpp"
#include "afl/string/format.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "ui/rich/document.hpp"
#include "util/translation.hpp"

/*
 *  Layout is:
 *
 *          __________________________        1px frame
 *                                            3px gap
 *          Neutronium   3 turns ago          1x text
 *          scattered                         1x text
 *                                            3px gap
 *          mined   #####::::::  100 kt       1x text/bar
 *          ground  ##:::::::::   40 kt       1x text/bar
 *          density #########::   80 %        1x text/bar
 *          __________________________        3px gap
 *                                            1px frame
 *
 *  Content:
 *
 *      .status = Unknown
 *         first:  "No information on minerals available"
 *         others: entirely blank
 *
 *      .status = Scanned
 *         first:  age
 *         others: nothing
 *
 *      .status = Reliable
 *         all:    mining info
 */

namespace {
    const int FRAME_SIZE = 1;
    const int GAP_SIZE = 3;
    const int HORIZ_PADDING = 3;

    const int AMOUNT_SCALE = 82;

    struct Metrics {
        int labelWidth;
        int unitWidth;
        int amountWidth;
    };

    void drawPercentBar(gfx::Context<util::SkinColor::Color>& skinContext,
                        gfx::Context<uint8_t>& paletteContext,
                        const Metrics& m,
                        gfx::Rectangle area,
                        String_t label,
                        int32_t barScale,
                        uint8_t barColor,
                        int32_t rawValue,
                        String_t unit,
                        const util::NumberFormatter& fmt)
    {
        // Label
        skinContext.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
        skinContext.setColor(util::SkinColor::Static);
        outTextF(skinContext, area.splitX(m.labelWidth), label);

        // Bar
        const int barWidth = std::max(0, area.getWidth() - m.unitWidth - m.amountWidth);
        gfx::Rectangle barArea = area.splitX(barWidth);
        int split = barArea.getWidth() * rawValue / (100 * barScale);
        if (split == 0 && rawValue != 0) {
            split = 1;
        }
        barArea.setHeight(barArea.getHeight() - 1);
        drawSolidBar(paletteContext, barArea.splitX(split), barColor);
        drawSolidBar(paletteContext, barArea,               ui::Color_Shield+3);

        // Value
        skinContext.setTextAlign(gfx::RightAlign, gfx::TopAlign);
        outTextF(skinContext, area.splitX(m.amountWidth), fmt.formatNumber(rawValue));

        // Unit
        skinContext.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
        area.consumeX(5);
        outTextF(skinContext, area, unit);
    }
}

client::widgets::PlanetMineralInfo::PlanetMineralInfo(ui::Root& root, util::NumberFormatter fmt, afl::string::Translator& tx)
    : SimpleWidget(),
      m_root(root),
      m_translator(tx),
      m_formatter(fmt),
      m_name(),
      m_info(),
      m_mode(Blank)
{ }

client::widgets::PlanetMineralInfo::~PlanetMineralInfo()
{ }

void
client::widgets::PlanetMineralInfo::setContent(String_t name,
                                               const Info_t& info,
                                               Mode mode)
{
    m_name = name;
    m_info = info;
    m_mode = mode;
    requestRedraw();
}

void
client::widgets::PlanetMineralInfo::draw(gfx::Canvas& can)
{
    // ex envscan.pas:ShowMining (part)
    switch (m_mode) {
     case Blank:
        drawNothing(can);
        break;

     case First:
        switch (m_info.status) {
         case Info_t::Unknown:
            drawExcuse(can);
            break;

         case Info_t::Scanned:
            drawBars(can, ShowAge);
            break;

         case Info_t::Reliable:
            drawBars(can, ShowMining);
            break;
        }
        break;

     case Second:
        switch (m_info.status) {
         case Info_t::Unknown:
            drawNothing(can);
            break;

         case Info_t::Scanned:
            drawBars(can, ShowNothing);
            break;

         case Info_t::Reliable:
            drawBars(can, ShowMining);
            break;
        }
        break;
    }
}

void
client::widgets::PlanetMineralInfo::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::PlanetMineralInfo::handlePositionChange()
{ }

ui::layout::Info
client::widgets::PlanetMineralInfo::getLayoutInfo() const
{
    gfx::Point size = m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(30, 5)
        + gfx::Point(0, 3*GAP_SIZE + 2*FRAME_SIZE);
    return ui::layout::Info(size, ui::layout::Info::GrowHorizontal);
}

bool
client::widgets::PlanetMineralInfo::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::widgets::PlanetMineralInfo::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

/*
 *  Private
 */

void
client::widgets::PlanetMineralInfo::drawNothing(gfx::Canvas& can)
{
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    drawBackground(ctx, getExtent());
}

void
client::widgets::PlanetMineralInfo::drawExcuse(gfx::Canvas& can)
{
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
    ctx.setColor(util::SkinColor::Static);
    ctx.setTransparentBackground();

    drawBackground(ctx, getExtent());
    outTextF(ctx, getExtent(), m_translator("No information on minerals available."));
}

void
client::widgets::PlanetMineralInfo::drawBars(gfx::Canvas& can, NoteType type)
{
    // ex WPlanetMiningTile::drawData
    static const char*const labels[] = {
        N_("mined"),
        N_("ground"),
        N_("density"),
        N_("total"),
    };
    static const char*const units[] = {
        N_(" kt"),
        " %",
    };

    // Running state
    gfx::Context<uint8_t> paletteContext(can, m_root.colorScheme());
    gfx::Rectangle area = getExtent();
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());

    // Frame around the widget
    ui::drawFrameDown(paletteContext, area);
    area.grow(-(FRAME_SIZE + HORIZ_PADDING), -FRAME_SIZE);

    // Geometry
    Metrics m;
    m.labelWidth = font->getMaxTextWidth(afl::functional::createStringTable(labels).map(m_translator)) + 5;
    m.unitWidth  = font->getMaxTextWidth(afl::functional::createStringTable(units) .map(m_translator)) + 5;
    m.amountWidth = font->getTextWidth("0") * 6 + 5;
    const int he = font->getCellSize().getY();

    // Prepare canvas
    gfx::Context<util::SkinColor::Color> skinContext(can, getColorScheme());
    skinContext.useFont(*font);
    drawBackground(skinContext, area);

    // Top gap
    area.consumeY(GAP_SIZE);

    // First line
    // - left: mineral type
    gfx::Rectangle line = area.splitY(he);
    skinContext.setColor(util::SkinColor::Heading);
    skinContext.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    outTextF(skinContext, line, m_name);

    // - right: age or mining
    util::SkinColor::Color miningColor = util::SkinColor::Green;
    switch (type) {
     case ShowNothing:
        break;
     case ShowAge: {
        int age;
        if (m_info.age.get(age)) {
            skinContext.setTextAlign(gfx::RightAlign, gfx::TopAlign);
            skinContext.setColor(age < 0
                                 ? util::SkinColor::Green
                                 : age >= 3
                                 ? util::SkinColor::Red
                                 : util::SkinColor::Yellow);
            outTextF(skinContext, line, m_info.ageLabel);
        }
        break;
     }
     case ShowMining: {
        int miningRate;
        int32_t groundAmount;
        if (m_info.miningPerTurn.get(miningRate)) {
            if (m_info.groundAmount.get(groundAmount) && miningRate > groundAmount) {
                miningColor = util::SkinColor::Yellow;
            }
            skinContext.setTextAlign(gfx::RightAlign, gfx::TopAlign);
            skinContext.setColor(miningColor);
            outTextF(skinContext, line, afl::string::Format(m_translator("%d kt/turn"), m_formatter.formatNumber(miningRate)));
        }
        break;
     }
    }

    // Second line
    // - left: summary
    line = area.splitY(he);
    skinContext.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    skinContext.setColor(util::SkinColor::Static);
    if (!m_info.groundSummary.empty()) {
        if (!m_info.densitySummary.empty()) {
            outTextF(skinContext, line, afl::string::Format("%s, %s", m_info.groundSummary, m_info.densitySummary));
        } else {
            outTextF(skinContext, line, m_info.groundSummary);
        }
    } else {
        outTextF(skinContext, line, m_info.densitySummary);
    }

    // - right: time
    switch (type) {
     case ShowNothing:
     case ShowAge:
        break;

     case ShowMining: {
        int duration;
        if (m_info.miningDuration.get(duration)) {
            skinContext.setTextAlign(gfx::RightAlign, gfx::TopAlign);
            skinContext.setColor(miningColor);
            outTextF(skinContext, line, afl::string::Format(duration >= game::map::MAX_MINING_DURATION
                                                            ? m_translator(">%d turns")
                                                            : m_translator("\xE2\x89\x88" "%d turn%!1{s%}"),
                                                            m_formatter.formatNumber(duration)));
        }
        break;
     }
    }

    // Second gap
    area.consumeY(GAP_SIZE);

    // Bars
    int32_t mined = 0, ground = 0;
    int density = 0;
    bool minedOK = m_info.minedAmount.get(mined);
    bool groundOK = m_info.groundAmount.get(ground);
    bool densityOK = m_info.density.get(density);

    if (minedOK && groundOK && densityOK) {
        drawPercentBar(skinContext, paletteContext, m, area.splitY(he),
                       m_translator(labels[0]),
                       AMOUNT_SCALE,
                       ui::Color_White,
                       mined,
                       m_translator(units[0]),
                       m_formatter);

        drawPercentBar(skinContext, paletteContext, m, area.splitY(he),
                       m_translator(labels[1]),
                       AMOUNT_SCALE,
                       ui::Color_White,
                       ground,
                       m_translator(units[0]),
                       m_formatter);

        drawPercentBar(skinContext, paletteContext, m, area.splitY(he),
                       m_translator(labels[2]),
                       1,
                       ui::Color_Gray,
                       density,
                       m_translator(units[1]),
                       m_formatter);
    } else {
        // FIXME: this is not a perfect rendering (for example, it is not appropriate when
        // density + ground are known), but it's consistent with PCC2.
        int32_t n;
        if (minedOK) {
            n = mined;
        } else if (groundOK) {
            n = ground;
        } else {
            n = -1;
        }

        if (n >= 0) {
            drawPercentBar(skinContext, paletteContext, m, area.splitY(he),
                           m_translator(labels[3]),
                           AMOUNT_SCALE,
                           ui::Color_White,
                           n,
                           m_translator(units[0]),
                           m_formatter);

            // Excuse text with word-wrap
            // (we don't have an outTextFormatted)
            skinContext.setColor(util::SkinColor::Static);
            area.consumeX(10);
            ui::rich::Document doc(m_root.provider());
            doc.setPageWidth(area.getWidth());
            doc.add(m_translator("(no information on density and mined/ground masses available.)"));
            doc.finish();
            doc.draw(skinContext, area, 0);
        }
    }
}
