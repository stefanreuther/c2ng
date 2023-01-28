/**
  *  \file ui/widgets/tabbar.hpp
  *  \brief Class ui::widgets::TabBar
  */
#ifndef C2NG_UI_WIDGETS_TABBAR_HPP
#define C2NG_UI_WIDGETS_TABBAR_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/container/ptrvector.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"
#include "util/keystring.hpp"

namespace ui { namespace widgets {

    /** Horizontal tab bar.
        Implements the look and feel of a tab bar.
        Users can click a tab, or select one using a keystroke;
        this will cause the new tab to be highlighted and a signal to be generated. */
    class TabBar : public ui::Widget {
     public:
        static const int Tab = 1;
        static const int CtrlTab = 2;
        static const int F6 = 4;
        static const int Arrows = 8;

        TabBar(Root& root);
        ~TabBar();

        void addPage(size_t id, const String_t& name, util::Key_t key);
        void addPage(size_t id, const util::KeyString& name);

        void setFocusedTab(size_t id);
        void setFont(gfx::FontRequest font);

        void setKeys(int keys);

        // Widget:
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

        afl::base::Signal<void(size_t)> sig_tabClick;

     private:
        struct TabInfo;

        Root& m_root;
        afl::container::PtrVector<TabInfo> m_tabs;
        size_t m_currentTabId;
        gfx::FontRequest m_font;
        int m_keys;

        size_t getCurrentIndex() const;
        void setCurrentIndex(size_t index);
    };

} }

#endif
