/**
  *  \file client/dialogs/classicvcrdialog.hpp
  *  \brief Class client::dialogs::ClassicVcrDialog
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

    /** Classic VCR dialog.
        Displays a classic VCR database and allows users to chose a fight.
        When a fight is chosen, raises sig_play; caller can use that to start playback. */
    class ClassicVcrDialog {
     public:
        /** Constructor.
            \param root       UI root
            \param tx         Translator
            \param vcrSender  VCR sender (to access VCR database)
            \param gameSender Game sender (to access remainder of game) */
        ClassicVcrDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender, util::RequestSender<game::Session> gameSender);
        ~ClassicVcrDialog();

        /** Run dialog.
            If user chooses to go to an object, returns a reference that you should pass to Control::executeGoToReferenceWait.
            \return reference (!isSet() if dialog was closed without choosing an object) */
        game::Reference run();

        /** Signal: play battle.
            \param index Index of chosen battle */
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
        void onFirst();
        void onLast();
        void onPlay();
        void setCurrentIndex(size_t index);
        void postLoad();

        void onUpdate(size_t index, const game::vcr::BattleInfo& data);
        void onLeftInfo();
        void onRightInfo();
        void onSideInfo(size_t side);
        void onTab();
        void onScore();
        void onShowMap(game::map::Point pt);
    };

} }

#endif
