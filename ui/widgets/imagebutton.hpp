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
    class ImageButton : public BaseButton, public Root::PaletteHandler {
     public:
        /** Constructor.
            \param image    Image name (for ResourceProvider::getImage)
            \param key      Key
            \param root     UI root
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

        /** Set background color.
            \param color Color. By default, skin background (potentially patterned) is drawn. */
        void setBackgroundColor(uint8_t color);

        // PaletteHandler:
        void loadPalette(Root::PaletteLoader& ldr);
        void unloadPalette();

     private:
        /* Private Icon implementation. We could probably use Image? */
        class Icon : public ui::icons::Icon {
         public:
            Icon(Root& root, gfx::Point size);
            virtual gfx::Point getSize() const;
            virtual void draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const;

            afl::base::Ptr<gfx::Canvas> m_image;
            String_t m_text;
            Root& m_root;
            gfx::Point m_size;
            gfx::FontRequest m_font;
            int m_backgroundColor;
        };

        bool m_paletteLoaded;
        Icon m_icon;
        String_t m_imageName;
        afl::base::SignalConnection conn_imageChange;

        void onImageChange();
    };

} }

#endif
