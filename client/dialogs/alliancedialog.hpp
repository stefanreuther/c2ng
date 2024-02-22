/**
  *  \file client/dialogs/alliancedialog.hpp
  *  \brief Class client::dialogs::AllianceDialog
  */
#ifndef C2NG_CLIENT_DIALOGS_ALLIANCEDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_ALLIANCEDIALOG_HPP

#include "afl/base/deleter.hpp"
#include "afl/string/translator.hpp"
#include "client/widgets/alliancelevelgrid.hpp"
#include "client/widgets/alliancestatuslist.hpp"
#include "game/proxy/allianceproxy.hpp"
#include "game/session.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "ui/window.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Alliance dialog.
        Controls an AllianceProxy to display and edit alliances. */
    class AllianceDialog : public ui::Window {
     public:
        /** Constructor.

            Loads data from the proxy (and thus, blocks!).

            @param root       UI root
            @param gameSender Game sender
            @param tx         Translator */
        AllianceDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

        /** Run dialog.

            Opens the dialog, lets user interact with it, and optionally writes back;
            displays an error if alliances are not available.

            @param gameSender Game sender
            @param tx         Translator */
        void run(util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

     private:
        void writeBack(util::RequestSender<game::Session>& gameSender);
        void initData(util::RequestSender<game::Session>& gameSender);
        void initDialog(util::RequestSender<game::Session>& gameSender, afl::string::Translator& tx);
        void initContent();

        afl::base::Deleter m_deleter;

        ui::EventLoop m_loop;
        ui::Root& m_root;
        afl::string::Translator& m_translator;

        client::widgets::AllianceStatusList* m_pList;
        client::widgets::AllianceLevelGrid* m_pGrid;

        game::proxy::AllianceProxy::Status m_data;

        client::widgets::AllianceStatusList::ItemFlags_t getPlayerFlags(int player) const;

        void onSelectPlayer(int player);
        void onToggleAlliance(int player);
        void onChange();
        void onToggleOffer(size_t index);
    };

} }

#endif
