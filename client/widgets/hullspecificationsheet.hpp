/**
  *  \file client/widgets/hullspecificationsheet.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_HULLSPECIFICATIONSHEET_HPP
#define C2NG_CLIENT_WIDGETS_HULLSPECIFICATIONSHEET_HPP

#include "ui/group.hpp"
#include "ui/widgets/imagebutton.hpp"
#include "client/widgets/playerlist.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/simpletable.hpp"
#include "game/playerset.hpp"
#include "game/playerarray.hpp"
#include "client/proxy/hullspecificationproxy.hpp"
#include "afl/base/signal.hpp"
#include "util/numberformatter.hpp"

namespace client { namespace widgets {

    class HullSpecificationSheet : public ui::Group {
     public:
        typedef client::proxy::HullSpecificationProxy::HullSpecification HullSpecification_t;

        HullSpecificationSheet(ui::Root& root,
                               bool hasPerTurnCosts,
                               game::PlayerSet_t allPlayers,
                               const game::PlayerArray<String_t>& playerNames,
                               util::NumberFormatter fmt);

        void setContent(const HullSpecification_t& data);

        afl::base::Signal<void(int)> sig_playerClick;

     private:
        void init();
        void initPlayerLists(game::PlayerSet_t allPlayers, const game::PlayerArray<String_t>& playerNames);
        
        afl::base::Deleter m_deleter;
        ui::Root& m_root;

        bool m_hasPerTurnCosts;
        util::NumberFormatter m_formatter;

        ui::widgets::StaticText* m_pTitle;
        ui::widgets::ImageButton* m_pImage;
        ui::widgets::SimpleTable* m_pTables[3];
        PlayerList* m_pPlayerLists[3];
    };

} }

#endif
