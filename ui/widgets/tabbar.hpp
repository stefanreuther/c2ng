/**
  *  \file ui/widgets/tabbar.hpp
  */
#ifndef C2NG_UI_WIDGETS_TABBAR_HPP
#define C2NG_UI_WIDGETS_TABBAR_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/container/ptrvector.hpp"
#include "ui/cardgroup.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"
#include "util/keystring.hpp"

namespace ui { namespace widgets {

    class TabBar : public ui::Widget {
     public:
        TabBar(Root& root, CardGroup& g);
        ~TabBar();

        void addPage(const String_t& name, util::Key_t key, Widget& w);
        void addPage(const util::KeyString& name, Widget& w);

        void setFocusedPage(size_t n);

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
        struct TabInfo;

        Root& m_root;
        CardGroup& m_group;
        afl::container::PtrVector<TabInfo> m_tabs;
        afl::base::SignalConnection conn_focusChange;

        const TabInfo* getCurrentTab() const;
    };

} }

#endif
