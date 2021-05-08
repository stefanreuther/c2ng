/**
  *  \file client/widgets/classicvcrinfo.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_CLASSICVCRINFO_HPP
#define C2NG_CLIENT_WIDGETS_CLASSICVCRINFO_HPP

#include "ui/root.hpp"
#include "ui/widgets/button.hpp"
#include "util/skincolor.hpp"
#include "game/vcr/info.hpp"

namespace client { namespace widgets {

    class ClassicVcrInfo : public ui::Widget {
     public:
        typedef game::vcr::BattleInfo Data_t;

        ClassicVcrInfo(ui::Root& root);
        ~ClassicVcrInfo();

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

        void setData(const Data_t& data);

        afl::base::Signal<void(int)> sig_left;
        afl::base::Signal<void(int)> sig_right;
        afl::base::Signal<void(int)> sig_tab;

     private:
        ui::Root& m_root;
        ui::widgets::Button m_leftButton;
        ui::widgets::Button m_rightButton;
        ui::widgets::Button m_tabButton;
        Data_t m_data;

        afl::base::Ref<gfx::Font> getLargeFont() const;
        afl::base::Ref<gfx::Font> getNormalFont() const;
        afl::base::Ref<gfx::Font> getBoldFont() const;

        void setChildPositions();
    };

} }

#endif
