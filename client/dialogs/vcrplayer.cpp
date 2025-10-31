/**
  *  \file client/dialogs/vcrplayer.cpp
  *  \brief VCR Player Dialog
  */

#include "client/dialogs/vcrplayer.hpp"
#include "client/dialogs/vcrselection.hpp"
#include "client/downlink.hpp"
#include "client/vcr/classic/playbackscreen.hpp"
#include "client/vcr/flak/playbackscreen.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/vcrdatabaseproxy.hpp"

namespace {
    class ClassicPlayHandler : public afl::base::Closure<void(size_t)> {
     public:
        ClassicPlayHandler(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender, util::RequestSender<game::Session> gameSender, afl::sys::LogListener& log)
            : m_root(root), m_translator(tx), m_vcrSender(vcrSender), m_gameSender(gameSender), m_log(log)
            { }
        virtual void call(size_t index)
            {
                game::proxy::ConfigurationProxy confProxy(m_gameSender);
                client::vcr::classic::PlaybackScreen screen(m_root, m_translator, m_vcrSender, index, confProxy, m_log);
                screen.run();
            }
     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::proxy::VcrDatabaseAdaptor> m_vcrSender;
        util::RequestSender<game::Session> m_gameSender;
        afl::sys::LogListener& m_log;
    };

    class FlakPlayHandler : public afl::base::Closure<void(size_t)> {
     public:
        FlakPlayHandler(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::proxy::VcrDatabaseAdaptor> vcrSender, util::RequestSender<game::Session> gameSender, afl::sys::LogListener& log)
            : m_root(root), m_translator(tx), m_vcrSender(vcrSender), m_gameSender(gameSender), m_log(log)
            { }
        virtual void call(size_t index)
            {
                client::vcr::flak::PlaybackScreen screen(m_root, m_translator, m_vcrSender, index, m_gameSender, m_log);
                screen.run();
            }
     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::proxy::VcrDatabaseAdaptor> m_vcrSender;
        util::RequestSender<game::Session> m_gameSender;
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
    VcrSelection dlg(root, tx, vcrSender, gameSender);
    switch (st.kind) {
     case VcrDatabaseProxy::ClassicCombat:
        dlg.sig_play.addNewClosure(new ClassicPlayHandler(root, tx, vcrSender, gameSender, log));
        break;

     case VcrDatabaseProxy::FlakCombat:
        dlg.sig_play.addNewClosure(new FlakPlayHandler(root, tx, vcrSender, gameSender, log));
        break;

     case VcrDatabaseProxy::UnknownCombat:
        break;
    }
    return dlg.run();
}

