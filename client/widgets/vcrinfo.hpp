/**
  *  \file client/widgets/vcrinfo.hpp
  *  \brief Class client::widgets::VcrInfo
  */
#ifndef C2NG_CLIENT_WIDGETS_VCRINFO_HPP
#define C2NG_CLIENT_WIDGETS_VCRINFO_HPP

#include "game/playerarray.hpp"
#include "game/teamsettings.hpp"
#include "game/vcr/info.hpp"
#include "ui/root.hpp"
#include "ui/widgets/button.hpp"

namespace client { namespace widgets {

    /** VCR Info Widget.

        Displays information about a single fight:
        - "Battle 1 of X", type, location, seed
        - ship details, list of ships, fleets, or players depending on available room
        - List (L,R), Overview (Tab), Score (S) buttons

        This is a merge of the previous FlakVcrInfo and ClassicVcrInfo widgets.
        It supports all types of combat and chooses an optimum layout internally.

        To use,
        - call setPlayerNames() and setTeams() to set player names and relations
        - call setData() with the data for the battle */
    class VcrInfo : public ui::Widget {
     public:
        typedef game::vcr::BattleInfo Data_t;

        /** Constructor.
            @param root   UI root
            @param tx     Translator */
        VcrInfo(ui::Root& root, afl::string::Translator& tx);
        ~VcrInfo();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual void handleChildPositionChange(Widget& child, gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Set player names.
            These are used for condensed formats that show only one line per group/player.
            @param adjNames  Player names
            @see game::proxy::PlayerProxy::getPlayerNames */
        void setPlayerNames(const game::PlayerArray<String_t>& adjNames);

        /** Set team definitions.
            These are used for condensed formats that show only one line per group/player.
            @param teams  Team settings
            @see game::proxy::TeamProxy::init */
        void setTeams(const game::TeamSettings& teams);

        /** Set data.
            Defines the data to present.
            @param data Data */
        void setData(const Data_t& data);

        /** Set availability of "tab" button (combat diagram).
            @param flag Availability */
        void setTabAvailable(bool flag);

        /** Signal: show detail information.
            @param slot Unit number to display information for. For classic combat, 0=left, 1=right */
        afl::base::Signal<void(size_t)> sig_info;

        /** Signal: show combat diagram. */
        afl::base::Signal<void()> sig_tab;

        /** Signal: show score summary. */
        afl::base::Signal<void()> sig_score;

        /** Signal: show fight location on map.
            @param pos Position */
        afl::base::Signal<void(game::map::Point)> sig_showMap;

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        ui::widgets::Button m_leftButton;          ///< "L" (left, list).
        ui::widgets::Button m_rightButton;         ///< "R" (right).
        ui::widgets::Button m_tabButton;           ///< "Tab" (table).
        ui::widgets::Button m_scoreButton;         ///< "S" (score).
        ui::widgets::Button m_showMapButton;       ///< "F4" (map).
        Data_t m_data;
        game::PlayerArray<String_t> m_adjectiveNames;
        game::TeamSettings m_teamSettings;

        afl::base::Ref<gfx::Font> getLargeFont() const;
        afl::base::Ref<gfx::Font> getNormalFont() const;
        afl::base::Ref<gfx::Font> getBoldFont() const;

        bool isClassic() const;
        void setChildPositions();
        void updateButtonState();
        void onLeft();
        void onRight();
        void onTab();
        void onScore();
        void onMap();
    };

} }

#endif
