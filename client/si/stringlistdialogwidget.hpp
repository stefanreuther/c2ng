/**
  *  \file client/si/stringlistdialogwidget.hpp
  *  \brief Class client::si::StringListDialogWidget
  */
#ifndef C2NG_CLIENT_SI_STRINGLISTDIALOGWIDGET_HPP
#define C2NG_CLIENT_SI_STRINGLISTDIALOGWIDGET_HPP

#include "ui/widgets/stringlistbox.hpp"
#include "ui/root.hpp"

namespace client { namespace si {

    /** String list dialog widget.
        The scripting language includes a "With Listbox()" command which constructs a listbox widget and creates a simple dialog.
        The most simple implementation for this command within c2ng makes this a separate widget that can contain the additional attributes,
        and re-use the regular ScriptSide/UserSide/WidgetValue infrastructure. */
    class StringListDialogWidget : public ui::widgets::StringListbox {
     public:
        StringListDialogWidget(gfx::ResourceProvider& provider, ui::ColorScheme& scheme,
                               String_t dialogTitle, int32_t current, int32_t width, int32_t height, String_t help);

        bool run(ui::Root& root);

     private:
        String_t m_dialogTitle;
        int32_t m_current;
        int32_t m_width;
        int32_t m_height;
        String_t m_help;
    };

} }

#endif
