/**
  *  \file client/dialogs/classicvcrdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_CLASSICVCRDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_CLASSICVCRDIALOG_HPP

#include "client/widgets/classicvcrinfo.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"
#include "afl/base/signal.hpp"

namespace client { namespace dialogs {

    class ClassicVcrDialog {
     public:
        ClassicVcrDialog(ui::Root& root, util::RequestSender<game::Session> gameSender);
        ~ClassicVcrDialog();

        int run();

        afl::base::Signal<void(size_t)> sig_play;

     private:
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        client::widgets::ClassicVcrInfo m_info;
        util::RequestReceiver<ClassicVcrDialog> m_dialogReceiver;
        
        size_t m_currentIndex;
        size_t m_numBattles;
        bool m_isActiveQuery;

        void initNumBattles();
        void saveCurrentIndex();

        void onPrevious();
        void onNext();
        void onPlay();
        void setCurrentIndex(size_t index);
        void postLoad();
        void setData(size_t index, client::widgets::ClassicVcrInfo::Data data);
        void postprocessData(client::widgets::ClassicVcrInfo::Data& data);
        void setTempData();
    };

} }

#endif
