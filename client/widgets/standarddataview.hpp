/**
  *  \file client/widgets/standarddataview.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_STANDARDDATAVIEW_HPP
#define C2NG_CLIENT_WIDGETS_STANDARDDATAVIEW_HPP

#include "client/widgets/collapsibledataview.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/widgets/basebutton.hpp"
#include "afl/container/ptrvector.hpp"
#include "ui/widgets/framegroup.hpp"

namespace client { namespace widgets {

    class KeymapWidget;

    class StandardDataView : public CollapsibleDataView {
     public:
        enum ButtonAlignment {
            Top,
            Bottom
        };

        StandardDataView(ui::Root& root, gfx::Point sizeInCells, KeymapWidget& widget);
        ~StandardDataView();

        virtual void setChildPositions();
        virtual gfx::Point getPreferredChildSize() const;

        void addNewButton(ButtonAlignment alignment, int x, int y, ui::widgets::BaseButton* btn);

        void setText(const util::rich::Text& text);

        bool enableButton(util::Key_t key, ui::FrameType type);
        bool disableButton(util::Key_t key);

     private:
        struct Button;

        gfx::Point m_sizeInCells;
        ui::rich::DocumentView m_docView;
        util::rich::Text m_text;
        afl::container::PtrVector<Button> m_buttons;
        KeymapWidget& m_keys;

        void updateText();
        Button* findButton(util::Key_t key);
    };

} }

#endif
