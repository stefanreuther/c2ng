/**
  *  \file client/widgets/hullspecificationsheet.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_HULLSPECIFICATIONSHEET_HPP
#define C2NG_CLIENT_WIDGETS_HULLSPECIFICATIONSHEET_HPP

#include "afl/base/signal.hpp"
#include "afl/string/translator.hpp"
#include "client/widgets/playerlist.hpp"
#include "game/playerarray.hpp"
#include "game/playerset.hpp"
#include "game/proxy/hullspecificationproxy.hpp"
#include "ui/group.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/widgets/imagebutton.hpp"
#include "ui/widgets/simpletable.hpp"
#include "ui/widgets/statictext.hpp"
#include "util/numberformatter.hpp"

namespace client { namespace widgets {

    class HullSpecificationSheet : public ui::Group {
     public:
        typedef game::proxy::HullSpecificationProxy::HullSpecification HullSpecification_t;

        HullSpecificationSheet(ui::Root& root,
                               afl::string::Translator& tx,
                               game::PlayerSet_t allPlayers,
                               const game::PlayerArray<String_t>& playerNames,
                               util::NumberFormatter fmt,
                               bool useIcons);

        void setContent(const HullSpecification_t& data);

        afl::base::Signal<void(int)> sig_playerClick;

     private:
        void init();
        void initPlayerLists(game::PlayerSet_t allPlayers, const game::PlayerArray<String_t>& playerNames);

        afl::base::Deleter m_deleter;
        ui::Root& m_root;
        afl::string::Translator& m_translator;

        util::NumberFormatter m_formatter;
        bool m_useIcons;

        ui::widgets::StaticText* m_pTitle;
        ui::widgets::ImageButton* m_pImage;
        ui::widgets::SimpleTable* m_pBaseTable;
        ui::widgets::SimpleTable* m_pBuildTable;
        ui::rich::DocumentView* m_pHullFunctions;
        PlayerList* m_pPlayerLists[3];
    };

} }

#endif
