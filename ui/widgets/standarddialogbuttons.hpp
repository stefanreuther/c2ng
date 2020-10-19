/**
  *  \file ui/widgets/standarddialogbuttons.hpp
  *  \brief Class ui::widgets::StandardDialogButtons
  */
#ifndef C2NG_UI_WIDGETS_STANDARDDIALOGBUTTONS_HPP
#define C2NG_UI_WIDGETS_STANDARDDIALOGBUTTONS_HPP

#include "afl/base/deleter.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/widgets/button.hpp"

namespace ui { namespace widgets {

    /** Standard dialog buttons.
        Many dialogs have "OK", "Cancel" and (optionally) "Help" buttons.
        This widget simplifies and standardizes this button list.

        Convention for now: "OK", "Cancel" on the right, "Help" on the left.
        Rationale: looks best for most widgets.

        Having this in a widget allows possible future configurability or adaption to system convention. */
    class StandardDialogButtons : public ui::Group {
     public:
        /** Constructor.
            Creates just the widgets.
            Connect events manually, or use addStop().
            \param root Root */
        explicit StandardDialogButtons(ui::Root& root);

        /** Destructor. */
        ~StandardDialogButtons();

        /** Access "OK" button.
            \return OK button */
        ui::widgets::Button& ok();

        /** Access "Cancel" button.
            \return Cancel button */
        ui::widgets::Button& cancel();

        /** Attach "stop" events.
            \param loop EventLoop to use. "OK" will exit the loop with value 1, "Cancel" will exit with value 0. */
        void addStop(EventLoop& loop);

        /** Create "help" button.
            \param helper Widget to receive the button's keypress */
        void addHelp(Widget& helper);

     private:
        afl::base::Deleter m_deleter;
        ui::Root& m_root;
        ui::widgets::Button* m_pOK;
        ui::widgets::Button* m_pCancel;

        void init();
    };

    /** Execute dialog with standard dialog buttons.
        This is a convenience method for doing a standard data entry dialog.
        \param title   Title of dialog
        \param prompt  Prompt to show above content
        \param content Content of dialog
        \param framed  true to add a lowered frame around the content
        \param root    Root
        \return true if dialog was confirmed, false on cancel */
    bool doStandardDialog(String_t title, String_t prompt, Widget& content, bool framed, Root& root);

} }


// Access "OK" button.
inline ui::widgets::Button&
ui::widgets::StandardDialogButtons::ok()
{
    return *m_pOK;
}

// Access "Cancel" button.
inline ui::widgets::Button&
ui::widgets::StandardDialogButtons::cancel()
{
    return *m_pCancel;
}

#endif
