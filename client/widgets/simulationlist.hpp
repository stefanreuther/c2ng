/**
  *  \file client/widgets/simulationlist.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_SIMULATIONLIST_HPP
#define C2NG_CLIENT_WIDGETS_SIMULATIONLIST_HPP

#include "game/proxy/simulationsetupproxy.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"

namespace client { namespace widgets {

    class SimulationList : public ui::widgets::AbstractListbox {
     public:
        typedef game::proxy::SimulationSetupProxy::ListItem ListItem_t;
        typedef game::proxy::SimulationSetupProxy::ListItems_t ListItems_t;

        SimulationList(ui::Root& root, afl::string::Translator& tx);
        ~SimulationList();

        void setContent(const ListItems_t& items);
        const ListItem_t* getItem(size_t index) const;
        void setPreferredHeight(int numLines);

        // AbstractListbox:
        virtual size_t getNumItems() const;
        virtual bool isItemAccessible(size_t n) const;
        virtual int getItemHeight(size_t n) const;
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        // Widget:
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        ListItems_t m_content;
        int m_numLines;

        int getLineHeight() const;
    };

} }

#endif
