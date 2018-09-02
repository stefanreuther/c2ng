/**
  *  \file client/dialogs/visualscandialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_VISUALSCANDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_VISUALSCANDIALOG_HPP

#include "afl/bits/smallset.hpp"
#include "afl/string/translator.hpp"
#include "game/map/point.hpp"
#include "game/ref/list.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    class VisualScanDialog {
     public:
        VisualScanDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);
        ~VisualScanDialog();

        void setTitle(String_t title);
        void setOkName(String_t okName);
        void setAllowForeignShips(bool flag);
        void setEarlyExit(bool flag);

        bool loadCurrent(game::map::Point pos, game::ref::List::Options_t options, game::Id_t excludeShip);

        game::Reference run();

     private:
        struct ShipData;

        class Listener;
        class KeyHandler;
        class ListPeer;
        class SpecPeer;
        class Window;

        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;
        String_t m_title;
        String_t m_okName;
        bool m_allowForeignShips;
        bool m_earlyExit;
        bool m_allowRemoteControl;
        game::ref::List m_list;
    };
    
} }

#endif
