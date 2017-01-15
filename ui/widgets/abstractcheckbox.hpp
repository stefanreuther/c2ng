/**
  *  \file ui/widgets/abstractcheckbox.hpp
  *  \brief Class ui::widgets::AbstractCheckbox
  */
#ifndef C2NG_UI_WIDGETS_ABSTRACTCHECKBOX_HPP
#define C2NG_UI_WIDGETS_ABSTRACTCHECKBOX_HPP

#include "ui/widgets/abstractbutton.hpp"
#include "afl/base/signalconnection.hpp"

namespace ui { namespace widgets {

    /** Checkbox/Radiobutton base class.
        Displays a configurable image with an optional focus frame, and some text.

        This class is badly named; its excuse is that in PCC2 it was named similarly.
        In c2ng, this is not an abstract class.
        However, it has no checkbox/radiobutton behaviour built in, and only provides display and click recognition.

        Derived classes/users must
        - configure and update the checkbox pixmap (setImage()).
          These images must all be the same size.
        - hook sig_fire and do something meaningful like changing a value and updating the pixmap. */
    class AbstractCheckbox : public AbstractButton {
     public:
        /** Constructor.
            \param root UI root
            \param key  Key
            \param text Textual label for this checkbox
            \param imageSize Size of all the images */
        AbstractCheckbox(ui::Root& root, util::Key_t key, String_t text, gfx::Point imageSize);

        /** Destructor. */
        ~AbstractCheckbox();

        // AbstractButton/Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Set font.
            Should be called during setup of the widget.
            This setting will affect layout.
            \param font Font request */
        void setFont(gfx::FontRequest font);

        /** Set image name.
            \param imageName Name of image to request using the resource provider */
        void setImage(String_t imageName);        

     private:
        const String_t m_text;
        const gfx::Point m_imageSize;
        String_t m_imageName;
        gfx::FontRequest m_font;

        afl::base::SignalConnection conn_imageChange;
    };

} }

#endif
