/**
  *  \file ui/widgets/cardtabbar.hpp
  *  \brief Class ui::widgets::CardTabBar
  */
#ifndef C2NG_UI_WIDGETS_CARDTABBAR_HPP
#define C2NG_UI_WIDGETS_CARDTABBAR_HPP

#include "ui/widgets/tabbar.hpp"
#include "ui/cardgroup.hpp"

namespace ui { namespace widgets {

    /** Horizontal tab bar for a CardGroup.
        This connects a TabBar and a CardGroup to always show the tab
        corresponding to the currently-active widget of the CardGroup. */
    class CardTabBar : public TabBar {
     public:
        CardTabBar(Root& root, CardGroup& g);
        ~CardTabBar();

        void addPage(const String_t& name, util::Key_t key, Widget& w);
        void addPage(const util::KeyString& name, Widget& w);

        void setFocusedPage(size_t n);

     private:
        void onFocusChange();

        Root& m_root;
        CardGroup& m_group;
        std::vector<Widget*> m_tabs;
        afl::base::SignalConnection conn_focusChange;
    };

} }

#endif
