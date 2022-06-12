/**
  *  \file client/widgets/controlscreenheader.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_CONTROLSCREENHEADER_HPP
#define C2NG_CLIENT_WIDGETS_CONTROLSCREENHEADER_HPP

#include "afl/base/deleter.hpp"
#include "afl/bits/smallset.hpp"
#include "game/session.hpp"
#include "gfx/keyeventconsumer.hpp"
#include "gfx/resourceprovider.hpp"
#include "ui/colorscheme.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/imagebutton.hpp"
#include "ui/widgets/statictext.hpp"

namespace client { namespace widgets {

    // FIXME: the original code specifically prepared a widget Id (cm_CS_Xchg) to allow related code to enable/disable 'x'.
    // We currently have no way to enable/disable a button.
    // The optimal solution would be some generic enable/disable mechanism in KeymapWidget to put the conditions into keymaps.

    class ControlScreenHeader : public ui::Widget {
     public:
        enum Button {
            btnF1,
            btnF2,
            btnF3,
            btnF4,
            btnF6,
            btnF7,
            btnUp,
            btnDown,
            btnSend,            // "I"
            btnAuto,
            btnCScr,
            btnX,
            btnAdd,
            btnTab,
            btnJoin,
            btnHelp,
            btnESC,
            btnName,
            btnImage
        };
        static const size_t NUM_BUTTONS = btnImage+1;
        typedef afl::bits::SmallSet<Button> Buttons_t;

        enum Text {
            txtHeading,
            txtSubtitle
        };
        static const size_t NUM_TEXTS = txtSubtitle+1;

        ControlScreenHeader(ui::Root& root, gfx::KeyEventConsumer& kmw);
        ~ControlScreenHeader();

        void enableButton(Button btn, ui::FrameType type);
        void disableButton(Button btn);

        void setText(Text which, String_t value);
        void setImage(String_t name);
        void setHasMessages(bool flag);

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual void handleChildPositionChange(Widget& child, gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        class TitleWidget;

        afl::base::Deleter m_deleter;
        ui::widgets::FrameGroup* m_frames[NUM_BUTTONS];
        // ui::widgets::StaticText* m_texts[NUM_TEXTS];
        TitleWidget* m_title;
        ui::widgets::ImageButton* m_image;
        Buttons_t m_visibleButtons;

        void createChildWidgets(ui::Root& root, gfx::KeyEventConsumer& kmw);
        void setChildWidgetPositions();
        void setChildPosition(ui::Widget* widget, int x, int y, int w, int h);
    };

    /** Convert game::Session::TaskStatus to ui::FrameType.
        This is used to highlight the CScr/Auto buttons.
        \param st TaskStatus
        \return FrameType */
    ui::FrameType getFrameTypeFromTaskStatus(game::Session::TaskStatus st);

} }

#endif
