/**
  *  \file client/widgets/flakvcrinfo.hpp
  *  \brief Class client::widgets::FlakVcrInfo
  */
#ifndef C2NG_CLIENT_WIDGETS_FLAKVCRINFO_HPP
#define C2NG_CLIENT_WIDGETS_FLAKVCRINFO_HPP

#include "game/playerarray.hpp"
#include "game/teamsettings.hpp"
#include "game/vcr/info.hpp"
#include "ui/root.hpp"
#include "ui/widgets/button.hpp"

namespace client { namespace widgets {

    /** FLAK VCR Info Widget.

        Displays information about a single fight:
        - "Battle 1 of X", "FLAK"
        - list of ships, fleets, or players depending on available room
        - List (L), Overview (Tab), Score (S) buttons

        To use,
        - call setPlayerNames() and setTeams() to set player names and relations
        - call setData() with the data for the battle */
    class FlakVcrInfo : public ui::Widget {
     public:
        typedef game::vcr::BattleInfo Data_t;

        FlakVcrInfo(ui::Root& root, afl::string::Translator& tx);
        ~FlakVcrInfo();

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

        void setPlayerNames(const game::PlayerArray<String_t>& adjNames);
        void setTeams(const game::TeamSettings& teams);
        void setData(const Data_t& data);
        void setTabAvailable(bool flag);

        afl::base::Signal<void(int)> sig_list;
        afl::base::Signal<void(int)> sig_tab;
        afl::base::Signal<void(int)> sig_score;

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        ui::widgets::Button m_listButton;
        ui::widgets::Button m_tabButton;
        ui::widgets::Button m_scoreButton;
        Data_t m_data;
        game::PlayerArray<String_t> m_adjectiveNames;
        game::TeamSettings m_teamSettings;

        afl::base::Ref<gfx::Font> getLargeFont() const;
        afl::base::Ref<gfx::Font> getNormalFont() const;
        afl::base::Ref<gfx::Font> getBoldFont() const;

        void setChildPositions();
    };

} }

#endif
