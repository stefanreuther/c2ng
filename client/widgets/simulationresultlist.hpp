/**
  *  \file client/widgets/simulationresultlist.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_SIMULATIONRESULTLIST_HPP
#define C2NG_CLIENT_WIDGETS_SIMULATIONRESULTLIST_HPP

#include "game/playerarray.hpp"
#include "game/playerset.hpp"
#include "game/proxy/simulationrunproxy.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"

namespace client { namespace widgets {

    class SimulationResultList : public ui::widgets::AbstractListbox {
     public:
        typedef game::proxy::SimulationRunProxy::ClassInfo_t ClassInfo_t;
        typedef game::proxy::SimulationRunProxy::ClassInfos_t ClassInfos_t;

        SimulationResultList(ui::Root& root);
        ~SimulationResultList();

        void setPlayerNames(const game::PlayerArray<String_t>& names);
        void setPlayers(game::PlayerSet_t set);
        void setClassResults(const ClassInfos_t& list);

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
        game::PlayerArray<String_t> m_playerNames;
        game::PlayerSet_t m_playerSet;
        ClassInfos_t m_classResults;

        int m_labelWidth;
        int m_cellWidth;
    };

} }

#endif
