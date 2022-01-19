/**
  *  \file game/pcc/browserhandler.hpp
  */
#ifndef C2NG_GAME_PCC_BROWSERHANDLER_HPP
#define C2NG_GAME_PCC_BROWSERHANDLER_HPP

#include "afl/data/access.hpp"
#include "afl/data/value.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/http/downloadlistener.hpp"
#include "afl/net/http/manager.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/handler.hpp"
#include "game/v3/rootloader.hpp"

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
        virtual std::auto_ptr<game::browser::Task_t> loadGameRootMaybe(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t>& then);

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

        game::v3::RootLoader& loader();

     private:
        game::browser::Browser& m_browser;
        afl::net::http::Manager& m_manager;
        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;
        util::ProfileDirectory& m_profile;

        afl::io::NullFileSystem m_nullFS;
        game::v3::RootLoader m_v3Loader;

        // Cache:
        std::auto_ptr<afl::data::Value> m_gameList;
        String_t m_gameListAccount;
        // FIXME: time...
    };

} }

#endif
