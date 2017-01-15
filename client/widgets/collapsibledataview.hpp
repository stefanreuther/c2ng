/**
  *  \file client/widgets/collapsibledataview.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_COLLAPSIBLEDATAVIEW_HPP
#define C2NG_CLIENT_WIDGETS_COLLAPSIBLEDATAVIEW_HPP

#include "ui/widget.hpp"
#include "ui/root.hpp"

namespace client { namespace widgets {

    class CollapsibleDataView : public ui::Widget {
     public:
        enum ViewState {
            Complete,
            HeadingOnly,
            DataOnly
        };

        static const int LeftAligned = 1;
        static const int DataAligned = 2;

        CollapsibleDataView(ui::Root& root);
        ~CollapsibleDataView();

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

        // New virtuals:
        virtual void setChildPositions() = 0;
        virtual gfx::Point getPreferredChildSize() const = 0;

        // New nonvirtuals:
        void setViewState(ViewState state);
        void setTitle(String_t title);
        gfx::Point getAnchorPoint(int flags) const;
        ui::Root& root() const;

     private:
        ui::Root& m_root;
        ViewState m_viewState;
        String_t m_title;

        afl::base::Ref<gfx::Font> getTitleFont() const;
    };

} }

#endif
