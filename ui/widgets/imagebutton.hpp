/**
  *  \file ui/widgets/imagebutton.hpp
  *  \brief Class ui::widgets::ImageButton
  */
#ifndef C2NG_UI_WIDGETS_IMAGEBUTTON_HPP
#define C2NG_UI_WIDGETS_IMAGEBUTTON_HPP

#include "ui/widgets/abstractbutton.hpp"
#include "afl/base/signalconnection.hpp"
#include "gfx/fontrequest.hpp"

namespace ui { namespace widgets {

    /** Image button.
        Displays an image that can be clicked with an optional overlay text.
        (If you just want an image, ignore the "can be clicked" part.) */
    class ImageButton : public AbstractButton {
     public:
        /** Constructor.
            \param image    Image name (for ResourceProvider::getImage)
            \param key      Key
            \param provider ResourceProvider
            \param size     Size of this widget */
        ImageButton(String_t image, util::Key_t key, ui::Root& root, gfx::Point size);

        /** Destructor. */
        ~ImageButton();

        // AbstractButton/Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Set image.
            Will request the new image and update display.
            \param image New image name.  */
        void setImage(String_t image);

        /** Set overlay text.
            \param text New text */
        void setText(String_t text);

     private:
        String_t m_image;
        gfx::Point m_size;
        afl::base::SignalConnection conn_imageChange;

        String_t m_text;
        gfx::FontRequest m_font;

        void onImageChange();
    };

} }

#endif
