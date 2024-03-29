/**
  *  \file client/dialogs/vcrselection.hpp
  *  \brief Class client::dialogs::VcrSelection
  */
#ifndef C2NG_CLIENT_DIALOGS_VCRSELECTION_HPP
#define C2NG_CLIENT_DIALOGS_VCRSELECTION_HPP

#include "afl/base/signal.hpp"
#include "afl/string/translator.hpp"
#include "client/widgets/vcrinfo.hpp"
#include "game/proxy/vcrdatabaseadaptor.hpp"
#include "game/proxy/vcrdatabaseproxy.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "ui/eventloop.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** VCR selection dialog.
        Displays a VCR database and allows users to chose a fight.
        When a fight is chosen, raises sig_play; caller can use that to start playback.

        This is a merge of the previous FlakVcrDialog and ClassicVcrDialog.
        Therefore, it supports all combat types. */
    class VcrSelection {
     public:
        /** Constructor.
            \param root       UI root
            \param tx         Translator
            \param vcrSender  VCR sender (to access VCR database)
            \param gameSender Game sender (to access remainder of game) */
        VcrSelection(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender, util::RequestSender<game::Session> gameSender);
        ~VcrSelection();

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
        client::widgets::VcrInfo m_info;
        ui::EventLoop m_loop;
        game::Reference m_result;
        game::vcr::BattleInfo m_battleInfo;

        size_t m_currentIndex;
        size_t m_numBattles;
        game::proxy::VcrDatabaseProxy::Kind m_kind;

        void init();

        void onPrevious();
        void onNext();
        void onFirst();
        void onLast();
        void onPlay();
        void setCurrentIndex(size_t index);
        void postLoad();

        void onUpdate(size_t index, const game::vcr::BattleInfo& data);
        void onInfo(size_t pos);
        void onAction(client::widgets::VcrInfo::Action a);
        void onTab();
        void onScore();
        void onShowMap(game::map::Point pt);
        void onExportBattles();
        void onExportUnits();
        void onSave(size_t first, size_t num);
    };

} }


#endif
