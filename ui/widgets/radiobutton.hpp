/**
  *  \file ui/widgets/radiobutton.hpp
  *  \brief Class ui::widgets::RadioButton
  */
#ifndef C2NG_UI_WIDGETS_RADIOBUTTON_HPP
#define C2NG_UI_WIDGETS_RADIOBUTTON_HPP

#include "ui/widgets/abstractcheckbox.hpp"
#include "afl/base/signal.hpp"
#include "afl/base/observable.hpp"
#include "afl/base/signalconnection.hpp"

namespace ui { namespace widgets {

    /** Radio button.
        This is a button that displays whether one of many values is selected.
        The value is stored in an Observable<int>.
        The widget shows a check-mark if its value equals that of the Observable.
        Clicking it will set the value. */
    class RadioButton : public AbstractCheckbox {
     public:
        /** Constructor.
            \param root UI root
            \param key  Key
            \param text Textual label for this checkbox
            \param value Underlying value
            \param myValue Value of this widget */
        RadioButton(ui::Root& root, util::Key_t key, String_t text, afl::base::Observable<int>& value, int myValue);

        /** Access underlying value.
            \return value (same as constructor parameter) */
        afl::base::Observable<int>& value();

        /** Check whether this widget is checked. */
        bool isChecked() const;

     private:
        afl::base::Observable<int>& m_value;
        afl::base::SignalConnection conn_change;
        const int m_myValue;

        void onClick();
        void onChange();
        void updateImage();
    };

} }

#endif
