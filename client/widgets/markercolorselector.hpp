/**
  *  \file client/widgets/markercolorselector.hpp
  *  \brief Class client::widgets::MarkerColorSelector
  */
#ifndef C2NG_CLIENT_WIDGETS_MARKERCOLORSELECTOR_HPP
#define C2NG_CLIENT_WIDGETS_MARKERCOLORSELECTOR_HPP

#include "afl/base/deleter.hpp"
#include "afl/string/translator.hpp"
#include "ui/root.hpp"
#include "ui/widgets/icongrid.hpp"

namespace client { namespace widgets {

    /** Color selector for marker colors.
        Allows the user to choose a color from 1..NUM_USER_COLORS. */
    class MarkerColorSelector : public ui::widgets::IconGrid {
     public:
        /** Constructor.
            \param root Root */
        explicit MarkerColorSelector(ui::Root& root);
        ~MarkerColorSelector();

        /** Set color.
            \param color Color, [1,NUM_USER_COLORS]. Call is ignored if value is out of range. */
        void setColor(uint8_t color);

        /** Get color.
            \return selected color, [1,NUM_USER_COLORS] */
        uint8_t getColor() const;

        /** Standard dialog.
            \param [in]           title     Dialog title
            \param [in]           tx        Translator
            \param [out,optional] adjacent  If given, offers the user the ability to choose "Adjacent". On output, true if user chose that.
            \return true if user confirmed the dialog, false on cancel */
        bool doStandardDialog(String_t title, afl::string::Translator& tx, bool* adjacent);

     private:
        ui::Root& m_root;
        afl::base::Deleter m_deleter;
    };

} }

#endif
