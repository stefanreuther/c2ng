/**
  *  \file client/widgets/playerlist.hpp
  *  \brief Class client::widgets::PlayerList
  */
#ifndef C2NG_CLIENT_WIDGETS_PLAYERLIST_HPP
#define C2NG_CLIENT_WIDGETS_PLAYERLIST_HPP

#include "afl/base/signal.hpp"
#include "afl/string/string.hpp"
#include "game/playerarray.hpp"
#include "game/playerset.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"

namespace client { namespace widgets {

    /** List of Race or Team names.
        Displays (a subrange of) all player names, optionally each in his own color.
        Clicking a name emits a signal.
        Used for score charts and other tables.

        @change In PCC2 version, initialisation determines whether to show team or player names (ShowTeamNames).
        In this version, you set the names explicitly. */
    class PlayerList : public ui::SimpleWidget {
     public:
        enum Layout {
            HorizontalLayout,     /**< A single horizontal line. */
            VerticalLayout,       /**< A vertical column, all lines with same length. */
            FlowLayout            /**< Lines wrapped at preferredWidth. */
        };

        enum TextMode {
            ShowLetters,          /**< Display single letters, all same size. (ex ShowLetters) */
            ShowNames             /**< Display names. */
        };

        enum ColorMode {
            PlayerColors,         /**< Use per-player colors. */
            SameColors            /**< Do not use colors. All in same color. (ex NoColors) */
        };

        /** Constructor.
            \param root      UI root
            \param layout    Layout mode
            \param textMode  Text mode
            \param colorMode Color mode
            \param preferredWidth Preferred width of widget, in pixels. For FlowLayout: maximum width. For VerticalLayout: minimum width. For HorizontalLayout: ignored.
            \param players   Set of players to display; see setVisiblePlayers() */
        PlayerList(ui::Root& root, Layout layout, TextMode textMode, ColorMode colorMode, int preferredWidth, game::PlayerSet_t players);
        ~PlayerList();

        /** Set name.
            \param player Player number
            \param name   Name to show */
        void setName(int player, const String_t& name);

        /** Set names for all players.
            \param names Names */
        void setNames(const game::PlayerArray<String_t>& names);

        /** Set set of visible players.
            \param players Set */
        void setVisiblePlayers(game::PlayerSet_t players);

        /** Set minimum number of lines.
            This is used for layout purposes and makes senses for FlowLayout only.
            \param numLines Number of lines */
        void setMinimumLines(int numLines);

        /** Set highlighted players.
            Those players are shown in a different color.
            \param highlighedPlayers Set */
        void setHighlightedPlayers(game::PlayerSet_t highlighedPlayers);

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Get player color.
            Implements PCC's player/color mapping.
            \param player Player number
            \return color index */
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
        int m_minimumLines;

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
