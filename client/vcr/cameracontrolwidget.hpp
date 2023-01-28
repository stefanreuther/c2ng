/**
  *  \file client/vcr/cameracontrolwidget.hpp
  *  \brief Class client::vcr::CameraControlWidget
  */
#ifndef C2NG_CLIENT_VCR_CAMERACONTROLWIDGET_HPP
#define C2NG_CLIENT_VCR_CAMERACONTROLWIDGET_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/string/translator.hpp"
#include "ui/root.hpp"
#include "ui/widgets/button.hpp"

namespace client { namespace vcr {

    /** Camera control widget.
        Displays buttons to control the rendering of a (FLAK) fight.
        Keystrokes are forwarded to a defineable widget. */
    class CameraControlWidget : public ui::Widget {
     public:
        /** Constructor.
            \param root UI Root
            \param tx   Translator */
        CameraControlWidget(ui::Root& root, afl::string::Translator& tx);
        ~CameraControlWidget();

        /** Set status of camera.
            \param enable true=auto camera, false=manual camera */
        void setAutoCamera(bool enable);

        /** Set status of grid display.
            \param enable status */
        void setGrid(bool enable);

        /** Set mode name.
            \param name Name */
        void setModeName(String_t name);

        /** Forward keys to widget.
            \param w Widget that receives keystrokes/button clicks */
        void dispatchKeysTo(gfx::KeyEventConsumer& w);

        // Widget methods:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange();
        virtual void handleChildPositionChange(Widget& child, const gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        afl::container::PtrVector<ui::widgets::Button> m_buttons;
        bool m_autoCamera;
        bool m_grid;
        String_t m_modeName;

        gfx::Point getGridSize() const;
        void addButton(const char* label, util::Key_t key);
        void placeButton(size_t which, gfx::Rectangle r);
    };

} }

#endif
