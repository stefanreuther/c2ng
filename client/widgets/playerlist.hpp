/**
  *  \file client/widgets/playerlist.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_PLAYERLIST_HPP
#define C2NG_CLIENT_WIDGETS_PLAYERLIST_HPP

#include "ui/simplewidget.hpp"
#include "game/playerset.hpp"
#include "game/playerarray.hpp"
#include "afl/string/string.hpp"
#include "afl/base/signal.hpp"
#include "ui/root.hpp"

namespace client { namespace widgets {

    // /** List of Race or Team names. Displays (a subrange of) all player
    // names, optionally each in his own color. Clicking a name emits a
    // signal. Used for score charts and other tables. */
    class PlayerList : public ui::SimpleWidget {
     public:
        enum Layout {
            HorizontalLayout,     /**< A single horizontal line. */
            VerticalLayout,       /**< A vertical column, all lines with same length. */
            FlowLayout            /**< Lines wrapped at preferredWidth. */
        };

        enum TextMode {
            ShowLetters,          /**< Display single letters, all same size. (ex ShowLetters) */
            ShowNames
        };

        enum ColorMode {
            PlayerColors,
            SameColors            /**< Do not use colors. All in same color. (ex NoColors) */
        };
        
        PlayerList(ui::Root& root, Layout layout, TextMode textMode, ColorMode colorMode, int preferredWidth, game::PlayerSet_t players);
        ~PlayerList();

        void setName(int player, const String_t& name);
        void setNames(const game::PlayerArray<String_t>& names);

        void setHighlightedPlayers(game::PlayerSet_t highlighedPlayers);
        
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        static uint8_t getPlayerColor(int player);

        // ex sig_click
        afl::base::Signal<void(int)> sig_playerClick;

     private:
        ui::Root& m_root;

        // ex flags
        Layout m_layout;
        TextMode m_textMode;
        ColorMode m_colorMode;

        // ex preferred_width
        int m_preferredWidth;

        // ex from, to
        game::PlayerSet_t m_players;

        // ex current_click
        int m_currentPlayer;

        game::PlayerArray<gfx::Rectangle> m_positions;

        game::PlayerArray<String_t> m_playerNames;

        // ex highlight_race
        game::PlayerSet_t m_highlightedPlayers;

        void calcLayout(game::PlayerArray<gfx::Rectangle>& positions, int w) const;
    };

} }

#endif
