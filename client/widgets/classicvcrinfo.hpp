/**
  *  \file client/widgets/classicvcrinfo.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_CLASSICVCRINFO_HPP
#define C2NG_CLIENT_WIDGETS_CLASSICVCRINFO_HPP

#include "ui/root.hpp"
#include "ui/widgets/button.hpp"
#include "util/skincolor.hpp"

namespace client { namespace widgets {

    class ClassicVcrInfo : public ui::Widget {
     public:
        ClassicVcrInfo(ui::Root& root);
        ~ClassicVcrInfo();

        enum Text {
            Heading,
            Type,
            Result,
            Position
        };

        static const size_t NUM_UNITS = 2;
        static const size_t NUM_LINES_PER_UNIT = 4;
        static const size_t NUM_TEXT_LINES = 4;

        struct Data {
            String_t text[NUM_TEXT_LINES];
            String_t info[NUM_UNITS][NUM_LINES_PER_UNIT];
            util::SkinColor::Color color[NUM_UNITS][NUM_LINES_PER_UNIT];
            Data();
        };

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

        void setData(const Data& data);

     private:
        ui::Root& m_root;
        ui::widgets::Button m_leftButton;
        ui::widgets::Button m_rightButton;
        ui::widgets::Button m_tabButton;
        Data m_data;

        afl::base::Ref<gfx::Font> getLargeFont() const;
        afl::base::Ref<gfx::Font> getNormalFont() const;
        afl::base::Ref<gfx::Font> getBoldFont() const;

        void setChildPositions();
    };

} }

#endif
