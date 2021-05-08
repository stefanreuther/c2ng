/**
  *  \file client/widgets/markerkindselector.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_MARKERKINDSELECTOR_HPP
#define C2NG_CLIENT_WIDGETS_MARKERKINDSELECTOR_HPP

#include "afl/string/translator.hpp"
#include "ui/root.hpp"
#include "ui/widgets/icongrid.hpp"

namespace client { namespace widgets {

    /** Selector for marker kinds (shapes).
        Allows the user to choose a marker shape from 0..NUM_USER_MARKERS-1. */
    class MarkerKindSelector : public ui::widgets::IconGrid {
     public:
        /** Constructor.
            \param root Root */
        explicit MarkerKindSelector(ui::Root& root);
        ~MarkerKindSelector();

        /** Set kind (shape).
            \param kind Shape, [0,NUM_USER_MARKERS). Call is ignored if value is out of range. */
        void setMarkerKind(int k);

        /** Get kind (shape).
            \return kind, [0,NUM_USER_MARKERS) */
        int getMarkerKind() const;

        /** Standard dialog.
            \param [in] title Dialog title
            \param [in] tx    Translator
            \return true if user confirmed the dialog, false on cancel */
        bool doStandardDialog(String_t title, afl::string::Translator& tx);

     private:
        ui::Root& m_root;
        afl::base::Deleter m_deleter;
    };

} }

#endif
