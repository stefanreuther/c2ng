/**
  *  \file client/widgets/combatdiagram.hpp
  *  \brief Class client::widgets::CombatDiagram
  */
#ifndef C2NG_CLIENT_WIDGETS_COMBATDIAGRAM_HPP
#define C2NG_CLIENT_WIDGETS_COMBATDIAGRAM_HPP

#include "afl/base/optional.hpp"
#include "game/proxy/vcroverviewproxy.hpp"
#include "game/teamsettings.hpp"
#include "game/vcr/overview.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"
#include "ui/tooltip.hpp"

namespace client { namespace widgets {

    /** Combat diagram.
        Displays a game::vcr::Overview::Diagram.
        Colors can be mapped by-player or by-team.
        Hovering a battle shows a tooltip, clicking raises sig_battleClick. */
    class CombatDiagram : public ui::SimpleWidget {
     public:
        /** Constructor.
            \param root Root
            \param tx   Translator */
        CombatDiagram(ui::Root& root, afl::string::Translator& tx);
        ~CombatDiagram();

        /** Set content of diagram.
            Note that changing the content affects getLayoutInfo().
            \param content Content */
        void setContent(const game::vcr::Overview::Diagram& content);

        /** Set team settings.
            These are used in colors-by-team mode.
            \param teams Teams */
        void setTeams(const game::TeamSettings& teams);

        /** Choose whether to use team colors.
            \param useTeamColors true: use team colors (i.e. green=me); false: use player colors */
        void setUseTeamColors(bool useTeamColors);

        /** Set battle currently being hovered on.
            Used internally.
            \param battle Battle index */
        void setHoverBattle(size_t battle);

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Signal: battle clicked.
            \param slot Clicked battle index */
        afl::base::Signal<void(size_t)> sig_battleClick;

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        ui::Tooltip m_tooltip;
        game::vcr::Overview::Diagram m_content;
        game::TeamSettings m_teams;
        bool m_useTeamColors;
        bool m_mouseDown;

        size_t m_hoverBattle;

        uint8_t getPlayerColor(int player) const;
        afl::base::Ref<gfx::Font> getFont() const;
        gfx::Rectangle getBattleBoundingBox(size_t battle);
        void onTooltipHover(gfx::Point pos);
    };

} }

#endif
