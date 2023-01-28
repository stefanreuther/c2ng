/**
  *  \file client/widgets/simplegauge.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_SIMPLEGAUGE_HPP
#define C2NG_CLIENT_WIDGETS_SIMPLEGAUGE_HPP

#include "ui/root.hpp"
#include "ui/simplewidget.hpp"

namespace client { namespace widgets {

    /** Simple Gauge widget.
        Displays a bar that fills from the left, derived from have/total values, and an optional text.
        Otherwise, this widget is completely passive. */
    class SimpleGauge : public ui::SimpleWidget {
     public:
        /** Constructor.
            \param root Root
            \param width Width of widget in pixels */
        SimpleGauge(ui::Root& root, int width);

        /** Destructor. */
        ~SimpleGauge();

        /** Set values.
            \param have  Amount we have
            \param total Maximum amount we can have
            \param text  Text to show in widget */
        void setValues(int have, int total, String_t text);

        /** Set color of bar.
            \param color Bar color, one of ui::Color_xxx */
        void setBarColor(uint8_t color);

        /** Set text color.
            \param color Text color, one of ui::Color_xxx */
        void setTextColor(uint8_t color);

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        ui::Root& m_root;
        int m_have;
        int m_total;
        int m_width;
        String_t m_text;
        uint8_t m_barColor;
        uint8_t m_textColor;
    };

} }

#endif
