/**
  *  \file client/dialogs/classicvcrdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_CLASSICVCRDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_CLASSICVCRDIALOG_HPP

#include "afl/base/signal.hpp"
#include "afl/string/translator.hpp"
#include "client/widgets/classicvcrinfo.hpp"
#include "game/proxy/vcrdatabaseproxy.hpp"
#include "game/session.hpp"
#include "game/vcr/info.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    class ClassicVcrDialog {
     public:
        ClassicVcrDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender, util::RequestSender<game::Session> gameSender);
        ~ClassicVcrDialog();

        game::Reference run();

        afl::base::Signal<void(size_t)> sig_play;

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        game::proxy::VcrDatabaseProxy m_proxy;
        util::RequestSender<game::proxy::VcrDatabaseAdaptor> m_vcrSender;
        util::RequestSender<game::Session> m_gameSender;
        client::widgets::ClassicVcrInfo m_info;
        ui::EventLoop m_loop;
        game::Reference m_result;
        
        size_t m_currentIndex;
        size_t m_numBattles;

        void initNumBattles();

        void onPrevious();
        void onNext();
        void onPlay();
        void setCurrentIndex(size_t index);
        void postLoad();

        void onUpdate(size_t index, const game::vcr::BattleInfo& data);
        void onLeftInfo();
        void onRightInfo();
        void onSideInfo(size_t side);
        void onTab();
    };

} }

#endif
