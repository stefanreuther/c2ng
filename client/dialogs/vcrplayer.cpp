/**
  *  \file client/dialogs/vcrplayer.cpp
  *  \brief VCR Player Dialog
  */

#include "client/dialogs/vcrplayer.hpp"
#include "client/dialogs/classicvcrdialog.hpp"
#include "client/dialogs/flakvcrdialog.hpp"
#include "client/downlink.hpp"
#include "game/proxy/vcrdatabaseproxy.hpp"
#include "client/vcr/classic/playbackscreen.hpp"

namespace {
    class ClassicPlayHandler : public afl::base::Closure<void(size_t)> {
     public:
        ClassicPlayHandler(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender, afl::sys::LogListener& log)
            : m_root(root), m_translator(tx), m_vcrSender(vcrSender), m_log(log)
            { }
        virtual void call(size_t index)
            {
                client::vcr::classic::PlaybackScreen screen(m_root, m_translator, m_vcrSender, index, m_log);
                screen.run();
            }
     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::proxy::VcrDatabaseAdaptor> m_vcrSender;
        afl::sys::LogListener& m_log;
    };
}

game::Reference
client::dialogs::playCombat(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender, util::RequestSender<game::Session> gameSender, afl::sys::LogListener& log)
{
    game::Reference result;

    // Determine type of VCRs
    using game::proxy::VcrDatabaseProxy;
    VcrDatabaseProxy::Status st;
    Downlink ind(root, tx);
    VcrDatabaseProxy(vcrSender, root.engine().dispatcher(), tx, std::auto_ptr<game::spec::info::PictureNamer>()).getStatus(ind, st);

    // Type-specific dialog
    switch (st.kind) {
     case VcrDatabaseProxy::ClassicCombat: {
        ClassicVcrDialog dlg(root, tx, vcrSender, gameSender);
        dlg.sig_play.addNewClosure(new ClassicPlayHandler(root, tx, vcrSender, log));
        result = dlg.run();
        break;
     }
     case VcrDatabaseProxy::FlakCombat: {
        FlakVcrDialog dlg(root, tx, vcrSender, gameSender);
        // dlg.sig_play.addNewClosure(new PlayHandler(iface, ctl));
        result = dlg.run();
        break;
     }
     case VcrDatabaseProxy::UnknownCombat:
        break;
    }

    return result;
}

