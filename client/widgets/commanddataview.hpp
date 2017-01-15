/**
  *  \file client/widgets/commanddataview.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_COMMANDDATAVIEW_HPP
#define C2NG_CLIENT_WIDGETS_COMMANDDATAVIEW_HPP

#include "client/widgets/collapsibledataview.hpp"
#include "util/rich/text.hpp"
#include "util/key.hpp"
#include "ui/widgets/framegroup.hpp"

namespace client { namespace widgets {

    class KeymapWidget;

    class CommandDataView : public CollapsibleDataView {
     public:
        enum Mode {
            ButtonsLeft,
            ButtonsRight
        };

        CommandDataView(ui::Root& root, KeymapWidget& widget, Mode mode);
        ~CommandDataView();

        virtual void setChildPositions();
        virtual gfx::Point getPreferredChildSize() const;

        void addButton(String_t title, util::Key_t key);
        bool setText(util::Key_t key, bool left, const util::rich::Text& text);
        bool setFrame(util::Key_t key, ui::widgets::FrameGroup::Type type);

     private:
        struct Line;

        KeymapWidget& m_keys;

        afl::container::PtrVector<Line> m_lines;
        Mode m_mode;

        Line* findLine(util::Key_t key);

        gfx::Point findButtonSize() const;
    };

} }

#endif
