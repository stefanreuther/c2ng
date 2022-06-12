/**
  *  \file client/widgets/standarddataview.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_STANDARDDATAVIEW_HPP
#define C2NG_CLIENT_WIDGETS_STANDARDDATAVIEW_HPP

#include "afl/container/ptrvector.hpp"
#include "client/widgets/collapsibledataview.hpp"
#include "gfx/keyeventconsumer.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/widgets/basebutton.hpp"
#include "ui/widgets/framegroup.hpp"

namespace client { namespace widgets {

    class StandardDataView : public CollapsibleDataView {
     public:
        enum ButtonAlignment {
            Top,
            Bottom
        };

        StandardDataView(ui::Root& root, gfx::Point sizeInCells, gfx::KeyEventConsumer& widget);
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
        gfx::KeyEventConsumer& m_keys;

        void updateText();
        Button* findButton(util::Key_t key);
    };

} }

#endif
