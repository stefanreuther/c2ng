/**
  *  \file ui/widgets/tabbar.hpp
  *  \brief Class ui::widgets::TabBar
  */
#ifndef C2NG_UI_WIDGETS_TABBAR_HPP
#define C2NG_UI_WIDGETS_TABBAR_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/container/ptrvector.hpp"
#include "ui/root.hpp"
#include "ui/widgets/iconbox.hpp"
#include "util/keystring.hpp"

namespace ui { namespace widgets {

    /** Horizontal tab bar.
        Implements the look and feel of a tab bar.
        Users can click a tab, or select one using a keystroke;
        this will cause the new tab to be highlighted and a signal to be generated. */
    class TabBar : public IconBox {
     public:
        explicit TabBar(Root& root);
        ~TabBar();

        void addPage(size_t id, const String_t& name, util::Key_t key);
        void addPage(size_t id, const util::KeyString& name);

        void setFont(gfx::FontRequest font);

        size_t getCurrentTabId() const;
        void setCurrentTabId(size_t id);

        // Widget:
        virtual ui::layout::Info getLayoutInfo() const;

        // IconBox:
        virtual int getItemWidth(size_t nr) const;
        virtual bool isItemKey(size_t nr, util::Key_t key) const;
        virtual size_t getNumItems() const;
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);
        virtual void drawBlank(gfx::Canvas& can, gfx::Rectangle area);

     private:
        struct TabInfo;

        Root& m_root;
        afl::container::PtrVector<TabInfo> m_tabs;
        gfx::FontRequest m_font;
    };

} }

#endif
