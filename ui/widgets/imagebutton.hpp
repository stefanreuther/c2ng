/**
  *  \file ui/widgets/imagebutton.hpp
  *  \brief Class ui::widgets::ImageButton
  */
#ifndef C2NG_UI_WIDGETS_IMAGEBUTTON_HPP
#define C2NG_UI_WIDGETS_IMAGEBUTTON_HPP

#include "ui/widgets/basebutton.hpp"
#include "afl/base/signalconnection.hpp"
#include "gfx/fontrequest.hpp"
#include "ui/icons/icon.hpp"

namespace ui { namespace widgets {

    /** Image button.
        Displays an image that can be clicked with an optional overlay text.
        (If you just want an image, ignore the "can be clicked" part.) */
    class ImageButton : public BaseButton {
     public:
        /** Constructor.
            \param image    Image name (for ResourceProvider::getImage)
            \param key      Key
            \param provider ResourceProvider
            \param size     Size of this widget */
        ImageButton(String_t image, util::Key_t key, ui::Root& root, gfx::Point size);

        /** Destructor. */
        ~ImageButton();

        /** Set image.
            Will request the new image and update display.
            \param image New image name.  */
        void setImage(String_t image);

        /** Set overlay text.
            \param text New text */
        void setText(String_t text);

     private:
        /* Private Icon implementation. We could probably use Image? */
        class Icon : public ui::icons::Icon {
         public:
            Icon(String_t imageName, Root& root, gfx::Point size);
            virtual gfx::Point getSize() const;
            virtual void draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const;

            String_t m_imageName;
            String_t m_text;
            Root& m_root;
            gfx::Point m_size;
            gfx::FontRequest m_font;
        };

        Icon m_icon;
        afl::base::SignalConnection conn_imageChange;

        void onImageChange();
    };

} }

#endif
