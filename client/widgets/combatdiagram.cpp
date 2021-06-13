/**
  *  \file client/widgets/combatdiagram.cpp
  *  \brief Class client::widgets::CombatDiagram
  */

#include "client/widgets/combatdiagram.hpp"
#include "afl/string/format.hpp"
#include "client/marker.hpp"
#include "client/widgets/playerlist.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/icons/colortext.hpp"
#include "ui/icons/vbox.hpp"

namespace {
    const size_t nil = size_t(-1);

    const int TEXT_EMS = 20;
    const int RIGHT_PX = 10;
}

client::widgets::CombatDiagram::CombatDiagram(ui::Root& root, afl::string::Translator& tx)
    : m_root(root),
      m_translator(tx),
      m_tooltip(root),
      m_content(),
      m_teams(),
      m_useTeamColors(false),
      m_mouseDown(false),
      m_hoverBattle(nil)
{
    m_teams.sig_teamChange.add(this, (void (CombatDiagram::*)()) &CombatDiagram::requestRedraw);
    m_tooltip.sig_hover.add(this, &CombatDiagram::onTooltipHover);
}

client::widgets::CombatDiagram::~CombatDiagram()
{ }

void
client::widgets::CombatDiagram::setContent(const game::vcr::Overview::Diagram& content)
{
    m_content = content;
    requestRedraw();
}

void
client::widgets::CombatDiagram::setTeams(const game::TeamSettings& teams)
{
    m_teams.copyFrom(teams);
}

void
client::widgets::CombatDiagram::setUseTeamColors(bool useTeamColors)
{
    // ex WCombatDiagramWidget::setTeamColorState
    if (useTeamColors != m_useTeamColors) {
        m_useTeamColors = useTeamColors;
        requestRedraw();
    }
}

void
client::widgets::CombatDiagram::setHoverBattle(size_t battle)
{
    // ex WCombatDiagramWidget::setHoverSlot
    if (battle != m_hoverBattle) {
        m_hoverBattle = battle;
        requestRedraw();
    }
}

void
client::widgets::CombatDiagram::draw(gfx::Canvas& can)
{
    /* Compute layout */
    afl::base::Ref<gfx::Font> font = getFont();
    const int left       = font->getEmWidth()*TEXT_EMS;
    const int right      = RIGHT_PX;
    const int chartXRoom = getExtent().getWidth() - left - right;
    const int chartX     = getExtent().getLeftX() + left;
    const int chartY     = getExtent().getTopY();
    const int chartYRoom = getExtent().getHeight();
    const int nlines     = static_cast<int>(m_content.units.size());
    const int textHeight = font->getLineHeight();

    /* Set up */
    gfx::Context<util::SkinColor::Color> ctx1(can, getColorScheme());
    drawBackground(ctx1, getExtent());

    if (m_hoverBattle != nil) {
        /* Highlight hover. This is the same color as used in the WScoreIconBox. */
        ctx1.setRawColor(m_root.colorScheme().getColor(ui::Color_Grayscale + 6));

        gfx::Rectangle r(getBattleBoundingBox(m_hoverBattle));
        if (r.getHeight() >= 2 && r.getWidth() >= 2) {
            drawBar(ctx1, gfx::Rectangle(r.getLeftX(), r.getTopY()+1, r.getWidth(), r.getHeight()-2));
            drawHLine(ctx1, r.getLeftX()+1, r.getTopY(), r.getRightX()-2);
            drawHLine(ctx1, r.getLeftX()+1, r.getBottomY()-1, r.getRightX()-2);
        }
    }

    /* Labels on left side */
    ctx1.useFont(*font);
    ctx1.setColor(util::SkinColor::Static);
    int nextTextY = chartY;
    for (int i = 0; i < nlines; ++i) {
        int y = chartY + chartYRoom * (2*i + 1) / (2*nlines) - textHeight/2;
        if (y >= nextTextY) {
            outTextF(ctx1, gfx::Point(getExtent().getLeftX(), y), left, m_content.units[i].name);
            nextTextY = y + textHeight;
        }
    }

    /* Draw diagram */
    /*
     *  There is one difference between FLAK and classic combat here.
     *  Whereas classic combat reports a planet destroyed when it's captured,
     *  FLAK actually reports it captured, which means it gets its line through
     *  to the right. I'm not going to change this, though, because marking the
     *  planet destroyed in FLAK too would lose information about who destroyed
     *  (resp. captured) it, and marking it captured in classic combat would
     *  draw too many lines, making the diagram less intuitive for those.
     */

    gfx::Context<uint8_t> ctx2(can, m_root.colorScheme());
    std::vector<int> lastRelX(nlines);
    std::vector<int> lastOwners(nlines);
    for (int i = 0; i < nlines; ++i) {
        lastOwners[i] = m_content.units[i].initialOwner;
    }
    for (int i = 0, n = static_cast<int>(m_content.battles.size()); i < n; ++i) {
        /* Compute line color and figure out line range */
        const game::vcr::Overview::Diagram::Battle& entry = m_content.battles[i];
        size_t minLine = 0, maxLine = 0;
        for (size_t side = 0, nsides = entry.participants.size(); side < nsides; ++side) {
            /* Locate object line */
            size_t line = entry.participants[side].slot;
            if (side == 0 || line < minLine) {
                minLine = line;
            }
            if (side == 0 || line > maxLine) {
                maxLine = line;
            }
        }

        /* Draw vertical line */
        if (entry.status == 0) {
            /* nothing happened (stalemate) */
            ctx2.setColor(ui::Color_Dark);
            ctx2.setLinePattern(gfx::DOTTED_LINE);
        } else if (entry.status > 0) {
            /* unique captor */
            ctx2.setColor(getPlayerColor(entry.status));
        } else {
            /* non-unique captor, or destruction */
            ctx2.setColor(ui::Color_Black);
        }

        const int relX = chartXRoom * (i+1) / n;
        const int minY = chartY + chartYRoom * (2*static_cast<int>(minLine) + 1) / (2*nlines);
        const int maxY = chartY + chartYRoom * (2*static_cast<int>(maxLine) + 1) / (2*nlines);
        drawVLine(ctx2, chartX + relX, minY, maxY);
        ctx2.setLinePattern(gfx::SOLID_LINE);

        /* Draw horizontal lines and explosion markers */
        for (size_t side = 0, nsides = entry.participants.size(); side < nsides; ++side) {
            const int objLine    = static_cast<int>(entry.participants[side].slot);
            const int objY       = chartY + chartYRoom * (2*objLine + 1) / (2*nlines);
            const int objStatus  = entry.participants[side].status;
            if (objLine < 0 || objLine >= nlines) {
                continue;
            }

            ctx2.setLineThickness(3);
            ctx2.setColor(getPlayerColor(lastOwners[objLine]));
            drawHLine(ctx2, chartX + lastRelX[objLine], objY, chartX + relX);
            ctx2.setLineThickness(1);

            if (objStatus < 0) {
                /* killed */
                lastOwners[objLine] = 0;
                ctx2.setColor(ui::Color_Red);
                drawMarker(ctx2, *getUserMarker(0, true), gfx::Point(chartX + relX, objY));
                ctx2.setColor(ui::Color_Yellow);
                drawMarker(ctx2, *getUserMarker(2, true), gfx::Point(chartX + relX, objY));
            } else if (objStatus == 0) {
                /* survived: lastOwners unchanged*/
            } else {
                /* captured */
                lastOwners[objLine] = objStatus;
            }
            lastRelX[objLine] = relX;
        }
    }

    /* Final lines on right side */
    ctx2.setLineThickness(3);
    for (int i = 0; i < nlines; ++i) {
        if (lastOwners[i] != 0) {
            const int y = chartY + chartYRoom * (2*i + 1) / (2*nlines);
            ctx2.setColor(getPlayerColor(lastOwners[i]));
            drawHLine(ctx2, chartX + lastRelX[i], y, getExtent().getRightX()-1);
        }
    }
    ctx2.setLineThickness(1);
}

void
client::widgets::CombatDiagram::handleStateChange(State st, bool enable)
{
    // ex WCombatDiagramWidget::onStateChange
    if (st == ActiveState && !enable) {
        m_mouseDown = false;
        setHoverBattle(nil);
    }

    // Tooltip
    m_tooltip.handleStateChange(st, enable);
}

void
client::widgets::CombatDiagram::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
client::widgets::CombatDiagram::getLayoutInfo() const
{
    /* FIXME: we must artifically limit the preferred size because layout cannot
       yet handle size exceeding screen space. */

    int addX = getFont()->getEmWidth()*TEXT_EMS + RIGHT_PX;
    int numBattles = static_cast<int>(m_content.battles.size());
    int numUnits = static_cast<int>(m_content.units.size());

    gfx::Point minSize (std::min(400, numBattles*5  + addX), std::min(300, numUnits*2));
    gfx::Point prefSize(std::min(400, numBattles*20 + addX), std::min(300, numUnits*10));

    return ui::layout::Info(minSize, prefSize, ui::layout::Info::GrowBoth);
}

bool
client::widgets::CombatDiagram::handleKey(util::Key_t key, int prefix)
{
    m_tooltip.handleKey(key, prefix);
    return false;
}

bool
client::widgets::CombatDiagram::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    bool isInside = getExtent().contains(pt);
    if (isInside) {
        requestActive();
    }
    m_tooltip.handleMouse(pt, pressedButtons, isInside);

    if (isInside) {
        // Hover handling
        size_t battle = nil;
        for (size_t i = 0, n = m_content.battles.size(); i < n; ++i) {
            if (getBattleBoundingBox(i).contains(pt)) {
                battle = i;
            }
        }
        requestActive();
        setHoverBattle(battle);

        // Mouse click handling
        if (m_mouseDown && pressedButtons.empty()) {
            if (m_hoverBattle != nil) {
                sig_battleClick.raise(m_hoverBattle);
            }
        }
        m_mouseDown = !pressedButtons.empty();
    } else {
        setHoverBattle(nil);
    }
    return isInside;
}

/* Get color for a player's line */
uint8_t
client::widgets::CombatDiagram::getPlayerColor(int player) const
{
    // WCombatDiagramWidget::getColorForOwner
    if (m_useTeamColors) {
        return ui::GRAY_COLOR_SET[m_teams.getPlayerColor(player)];
    } else {
        return PlayerList::getPlayerColor(player);
    }
}

/* Shortcut for the font to use for drawing/sizing */
afl::base::Ref<gfx::Font>
client::widgets::CombatDiagram::getFont() const
{
    return m_root.provider().getFont("-");
}

/* Get bounding box for a battle */
gfx::Rectangle
client::widgets::CombatDiagram::getBattleBoundingBox(size_t battle)
{
    // ex WCombatDiagramWidget::getSlotBoundingBox
    /* Compute layout */
    afl::base::Ref<gfx::Font> font = getFont();
    const int left       = font->getEmWidth()*TEXT_EMS;
    const int right      = RIGHT_PX;
    const int chartXRoom = getExtent().getWidth() - left - right;
    const int chartX     = getExtent().getLeftX() + left;
    const int chartY     = getExtent().getTopY();
    const int chartYRoom = getExtent().getHeight();
    const int nlines     = static_cast<int>(m_content.units.size());
    const int n          = static_cast<int>(m_content.battles.size());

    /* Compute positions */
    gfx::Rectangle result;
    if (battle < m_content.battles.size()) {
        const game::vcr::Overview::Diagram::Battle& entry = m_content.battles[battle];
        if (!entry.participants.empty()) {
            size_t topLine = entry.participants[0].slot;
            size_t botLine = topLine;
            for (size_t i = 1; i < entry.participants.size(); ++i) {
                size_t line = entry.participants[i].slot;
                if (line < topLine) {
                    topLine = line;
                }
                if (line > botLine) {
                    botLine = line;
                }
            }

            const int x     = chartX + chartXRoom * (static_cast<int>(battle)+1) / n;
            const int topY  = chartY + chartYRoom * (2*static_cast<int>(topLine) + 1) / (2*nlines);
            const int botY  = chartY + chartYRoom * (2*static_cast<int>(botLine) + 1) / (2*nlines);
            const int width = std::min(chartXRoom / n, 20);
            const int yofs  = std::min(chartYRoom / (2*nlines), 10);

            /* Build result */
            result = gfx::Rectangle(x - width/2, topY - yofs, width, botY - topY + 2*yofs);
            result.intersect(getExtent());
        }
    }
    return result;
}

/* Callback: tooltip hover */
void
client::widgets::CombatDiagram::onTooltipHover(gfx::Point pos)
{
    // ex WCombatDiagramWidget::handleEvent (part)
    if (m_hoverBattle < m_content.battles.size()) {
        // Two lines
        ui::icons::ColorText battleNumber(afl::string::Format(m_translator("Battle %d:"), m_hoverBattle+1), m_root);
        ui::icons::ColorText battleName(m_content.battles[m_hoverBattle].name, m_root);
        battleNumber.setColor(ui::Color_Black);
        battleName.setColor(ui::Color_Black);
        battleName.setFont("b");

        // Show the popup
        ui::icons::VBox tooltipContent;
        tooltipContent.add(battleNumber);
        tooltipContent.add(battleName);
        m_tooltip.showPopup(pos, tooltipContent);
    }
}
