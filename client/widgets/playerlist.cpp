/**
  *  \file client/widgets/playerlist.cpp
  */

#include "client/widgets/playerlist.hpp"
#include "afl/base/countof.hpp"
#include "ui/colorscheme.hpp"
#include "gfx/context.hpp"
#include "game/playerlist.hpp"

/*
 *  FIXME: initialisation determines team or player names
 *
 *      if (flags & ShowTeamNames)
 *          cellwidths[i] = fonts[FONT_NORMAL]->getTextWidth(getTeamName(i));
 *      else
 *          cellwidths[i] = fonts[FONT_NORMAL]->getTextWidth(player_racenames.getAdjName(i));
 */

// /** Constructor.
//     \param preferred_width preferred width of widget, in pixels. For FlowLayout: maximum width.
//                            For VerticalLayout: minimum width. For HorizontalLayout: ignored.
//     \param flags           Layout flags
//     \param from            first player to display, [1,NUM_OWNERS]
//     \param to              last player to display, inclusive, [from,NUM_OWNERS] */
client::widgets::PlayerList::PlayerList(ui::Root& root, Layout layout, TextMode textMode, ColorMode colorMode, int preferredWidth, game::PlayerSet_t players)
    : SimpleWidget(),
      m_root(root),
      m_layout(layout),
      m_textMode(textMode),
      m_colorMode(colorMode),
      m_preferredWidth(preferredWidth),
      m_players(players),
      m_currentPlayer(0),
      m_positions(),
      m_playerNames(),
      m_highlightedPlayers()
{
    // ex WRaceList::WRaceList
}

client::widgets::PlayerList::~PlayerList()
{ }

void
client::widgets::PlayerList::setName(int player, const String_t& name)
{
    m_playerNames.set(player, name);
    requestRedraw();
}

void
client::widgets::PlayerList::setNames(const game::PlayerArray<String_t>& names)
{
    m_playerNames = names;
    requestRedraw();
}

void
client::widgets::PlayerList::setHighlightedPlayers(game::PlayerSet_t highlighedPlayers)
{
    // ex WRaceList::setHighlight
    if (highlighedPlayers != m_highlightedPlayers) {
        m_highlightedPlayers = highlighedPlayers;
        requestRedraw();
    }
}

void
client::widgets::PlayerList::draw(gfx::Canvas& can)
{
    // ex WRaceList::drawContent
    getColorScheme().drawBackground(can, getExtent());

    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
    ctx.setTransparentBackground();
    calcLayout(m_positions, getExtent().getWidth()); // FIXME
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        if (m_players.contains(i)) {
            // Set color
            if (m_highlightedPlayers.contains(i)) {
                ctx.setColor(ui::Color_Green);
            } else if (m_colorMode == SameColors) {
                ctx.setColor(ui::Color_Black);
            } else {
                ctx.setColor(getPlayerColor(i));
            }

            // Draw text
            gfx::Rectangle pos = m_positions.get(i);
            pos.moveBy(getExtent().getTopLeft());
            if (m_textMode == ShowLetters) {
                outTextF(ctx, pos, String_t(1, game::PlayerList::getCharacterFromPlayer(i)));
            } else {
                outTextF(ctx, pos, m_playerNames.get(i));
            }
        }
    }
}

void
client::widgets::PlayerList::handleStateChange(State st, bool enable)
{
    if (st == ActiveState && !enable) {
        m_currentPlayer = 0;
    }
}

void
client::widgets::PlayerList::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::widgets::PlayerList::getLayoutInfo() const
{
    // ex WRaceList::getLayoutInfo
    game::PlayerArray<gfx::Rectangle> pos;
    calcLayout(pos, m_preferredWidth);

    gfx::Point br = pos.get(game::MAX_PLAYERS).getBottomRight();

    switch (m_layout) {
     case FlowLayout:
        return gfx::Point(m_preferredWidth, br.getY());

     case VerticalLayout:
        return ui::layout::Info(br, br, ui::layout::Info::GrowHorizontal);

     case HorizontalLayout:
        return br;
    }

    return ui::layout::Info();
}

bool
client::widgets::PlayerList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::widgets::PlayerList::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // ex WRaceList::handleEvent
    if (getExtent().contains(pt)) {
        if (!pressedButtons.empty()) {
            // mouse clicked into this widget
            int where = 0;
            gfx::Point rel = pt - getExtent().getTopLeft();
            for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
                if (m_positions.get(i).contains(rel)) {
                    where = i;
                    break;
                }
            }
            if (where != m_currentPlayer && where != 0) {
                sig_playerClick.raise(where);
            }
            m_currentPlayer = where;
            return true;
        } else {
            m_currentPlayer = 0;
            return true;
        }
    } else {
        m_currentPlayer = 0;
        return false;
    }
}

uint8_t
client::widgets::PlayerList::getPlayerColor(int player)
{
    // ex WRaceList::colors
    // FIXME: explicit support for >11 races
    /* Color mapping. Indexed by player-number-minus-one. In PCC 1.x, the statistic
       and combat diagram screens have a separate palette for races 7-11. Since we
       usually have close colors, we use those instead. */
    const uint8_t COLORS[] = {
        ui::Color_BlueBlack,      /* Aliens: blue/gray // wrapped due to modulo operation below */

        ui::Color_Black,          /* Fed: black */
        ui::Color_White,          /* Lizard: white */
        ui::Color_Yellow,         /* Bird: yellow */
        ui::Color_Green,          /* Klingon: green */
        ui::Color_Blue,           /* Pirate: blue */
        ui::Color_Red,            /* Cyborg: red */
        ui::Color_BrightMagenta,  /* Crystal: lilac. 63/24/63 [255/97/255] in PCC 1.x, 255/85/255 here */
        ui::Color_BrightBrown,    /* Empire: brown. 48/24/0 [194/97/255] in PCC 1.x, 194/97/255 here */
        ui::Color_BrightOrange,   /* Robot: orange. 63/48/0 [255/194/0] in PCC 1.x, 255/194/0 here */
        ui::Color_BrightCyan,     /* Rebel: cyan. 32/63/63 [128/255/255] in PCC 1.x, 85/255/255 here */
        ui::Color_DarkGreen,      /* Colony: green. 0/48/0 [0/194/0] in PCC 1.x, 0/170/0 here */
    };
    return COLORS[size_t(player) % countof(COLORS)];
}

// /** Compute layout of widget.
//     \param positions [out] Relative positions of player boxes
//     \param w [in] Assumed widget width in pixels */
void
client::widgets::PlayerList::calcLayout(game::PlayerArray<gfx::Rectangle>& positions, int w) const
{
    // ex WRaceList::calcLayout
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());

    int x = 0, y = 0;
    int cellHeight = font->getCellSize().getY();

    // Compute cell width
    game::PlayerArray<int> cellWidths;
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        if (!m_players.contains(i)) {
            cellWidths.set(i, 0);
        } else if (m_textMode == ShowLetters) {
            cellWidths.set(i, cellHeight);
        } else {
            cellWidths.set(i, font->getTextWidth(m_playerNames.get(i)));
        }
    }

    // Compute layout
    if (m_layout == VerticalLayout) {
        // VerticalLayout. Find maximum width:
        int maxWidth = m_preferredWidth;
        for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
            if (m_players.contains(i)) {
                maxWidth = std::max(maxWidth, cellWidths.get(i));
            }
        }

        // Assign positions:
        for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
            if (m_players.contains(i)) {
                positions.set(i, gfx::Rectangle(x, y, maxWidth, cellHeight));
                y += cellHeight;
            } else {
                // Non-present players still receive a location so that positions.get(MAX_PLAYERS) yields the bottom-right corner.
                positions.set(i, gfx::Rectangle(x, y, maxWidth, 0));
            }
        }
    } else {
        // FlowLayout or HorizontalLayout
        for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
            int cellWidth = cellWidths.get(i);
            if (cellWidth > 0) {
                // Cell with content
                if (m_layout == FlowLayout && x + cellWidth > w) {
                    // start new line
                    x = 0;
                    y += cellHeight;
                }
                positions.set(i, gfx::Rectangle(x, y, cellWidth, cellHeight));
                x += cellWidth;
                if (m_textMode != ShowLetters) {
                    x += font->getCellSize().getX() / 2;
                }
            } else {
                // Empty cell (could also be non-present player)
                positions.set(i, gfx::Rectangle(x, y, 0, cellHeight));
            }
        }
    }
}
