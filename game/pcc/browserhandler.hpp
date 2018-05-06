/**
  *  \file game/pcc/browserhandler.hpp
  */
#ifndef C2NG_GAME_PCC_BROWSERHANDLER_HPP
#define C2NG_GAME_PCC_BROWSERHANDLER_HPP

#include "game/browser/handler.hpp"
#include "afl/net/http/manager.hpp"
#include "game/browser/browser.hpp"
#include "afl/data/value.hpp"
#include "afl/data/access.hpp"
#include "afl/net/http/downloadlistener.hpp"

namespace game { namespace pcc {

    class BrowserHandler : public game::browser::Handler {
     public:
        BrowserHandler(game::browser::Browser& b,
                       afl::net::http::Manager& mgr,
                       afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                       util::ProfileDirectory& profile);

        // Handler:
        virtual bool handleFolderName(String_t name, afl::container::PtrVector<game::browser::Folder>& result);
        virtual game::browser::Folder* createAccountFolder(game::browser::Account& acc);
        virtual afl::base::Ptr<Root> loadGameRoot(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config);

        // pcc::BrowserHandler
        bool login(game::browser::Account& acc);

        std::auto_ptr<afl::data::Value> callServer(game::browser::Account& acc,
                                                   String_t endpoint,
                                                   const afl::net::HeaderTable& args);

        afl::data::Access getGameList(game::browser::Account& acc);
        std::auto_ptr<afl::data::Value> getDirectoryContent(game::browser::Account& acc, String_t name);
        void getFile(game::browser::Account& acc, String_t name, afl::net::http::DownloadListener& listener);

        afl::string::Translator& translator();

        afl::sys::LogListener& log();

        afl::base::Ref<afl::io::Directory> getDefaultSpecificationDirectory();
        util::ProfileDirectory& profile();

     private:
        game::browser::Browser& m_browser;
        afl::net::http::Manager& m_manager;
        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;
        util::ProfileDirectory& m_profile;

        // Cache:
        std::auto_ptr<afl::data::Value> m_gameList;
        String_t m_gameListAccount;
        // FIXME: time...
    };

} }

#endif
