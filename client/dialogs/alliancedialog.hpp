/**
  *  \file client/dialogs/alliancedialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_ALLIANCEDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_ALLIANCEDIALOG_HPP

#include "afl/base/deleter.hpp"
#include "afl/string/translator.hpp"
#include "client/widgets/alliancelevelgrid.hpp"
#include "client/widgets/alliancestatuslist.hpp"
#include "game/alliance/container.hpp"
#include "game/session.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "ui/window.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    class AllianceDialog : public ui::Window {
     public:
        AllianceDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

        void run(util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

     private:
        void initDialog(util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);
        void initContent(util::RequestSender<game::Session> gameSender);
        void writeBack(util::RequestSender<game::Session> gameSender);

        afl::base::Deleter m_deleter;

        ui::EventLoop m_loop;
        ui::Root& m_root;
        afl::string::Translator& m_translator;

        client::widgets::AllianceStatusList* m_pList;
        client::widgets::AllianceLevelGrid* m_pGrid;

        struct Data {
            game::alliance::Container alliances;
            game::PlayerArray<String_t> names;
            game::PlayerSet_t players;
            int self;

            Data()
                : alliances(), names(), players(), self()
                { }
        };
        Data m_data;

        client::widgets::AllianceStatusList::ItemFlags_t getPlayerFlags(int player) const;

        void onSelectPlayer(int player);
        void onToggleAlliance(int player);
        void onChange();
        void onToggleOffer(size_t index);
    };

} }

#endif
