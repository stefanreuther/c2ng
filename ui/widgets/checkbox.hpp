/**
  *  \file ui/widgets/checkbox.hpp
  *  \brief Class ui::widgets::Checkbox
  */
#ifndef C2NG_UI_WIDGETS_CHECKBOX_HPP
#define C2NG_UI_WIDGETS_CHECKBOX_HPP

#include "ui/widgets/abstractcheckbox.hpp"
#include "util/stringlist.hpp"
#include "afl/base/signal.hpp"
#include "afl/base/observable.hpp"

namespace ui { namespace widgets {

    /** Checkbox.
        This is a checkbox that cycles through a multitude of states.
        The state is stored in an Observable<int>.
        The number of states can be defined on a widget-by-widget basis.
        Each state is represented by 16x16 pixmap.

        Call addImage() to add images to the widget.
        Each click will cycle through these images in the order they were added. */
    class Checkbox : public AbstractCheckbox {
     public:
        /** Constructor.
            \param root UI root
            \param key  Key
            \param text Textual label for this checkbox
            \param value Underlying value */
        Checkbox(ui::Root& root, util::Key_t key, String_t text, afl::base::Observable<int>& value);

        /** Add an image for a state.
            \param id Value
            \param name Resource Id */
        void addImage(int id, String_t name);

        /** Add default images for a two-state on/off checkbox.
            Defines default values for a checkbox that has states 0 (=off) and 1 (=on). */
        void addDefaultImages();

        /** Access underlying value.
            \return value (same as constructor parameter) */
        afl::base::Observable<int>& value();

     private:
        util::StringList m_imageMap;
        afl::base::Observable<int>& m_value;
        afl::base::SignalConnection conn_change;

        void onClick();
        void onChange();
        void updateImage();
    };

} }

#endif
