/**
  *  \file game/pcc/browserhandler.hpp
  *  \brief Class game::pcc::BrowserHandler
  */
#ifndef C2NG_GAME_PCC_BROWSERHANDLER_HPP
#define C2NG_GAME_PCC_BROWSERHANDLER_HPP

#include "afl/base/closure.hpp"
#include "afl/data/access.hpp"
#include "afl/data/value.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/http/downloadlistener.hpp"
#include "afl/net/http/manager.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/handler.hpp"
#include "game/v3/rootloader.hpp"

namespace game { namespace pcc {

    /** PlanetsCentral server integration entrypoint.
        Provides functionality for accounts of type "pcc" talking to <https://planetscentral.com/>.

        Basic logic: the asynchronous login() flow is used to obtain an API key using the user/whoami endpoint.
        This will perform user interaction.
        When the API key is known, future commands use that key only.
        If the key expires, operations start to fail.

        The login() flow should therefore be invoked at all places where possible.
        For now, it cannot be used at places where network operations are wrapped into Directory/Stream,
        which requires "synchronous" operation.
        In theory, in the future, actions could also detect expired tokens and re-invoke the login() flow. */
    class BrowserHandler : public game::browser::Handler {
     public:
        typedef game::browser::Task_t Task_t;

        /** Constructor.
            @param b        Owning Browser
            @param mgr      HTTP Manager
            @param defaultSpecificationDirectory Default specification
            @param profile  Profile (for global configuration, e.g. lru.ini) */
        BrowserHandler(game::browser::Browser& b, afl::net::http::Manager& mgr, afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory, util::ProfileDirectory& profile);

        // Handler:
        virtual bool handleFolderName(String_t name, afl::container::PtrVector<game::browser::Folder>& result);
        virtual game::browser::Folder* createAccountFolder(game::browser::Account& acc);
        virtual std::auto_ptr<game::browser::Task_t> loadGameRootMaybe(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t>& then);

        /*
         *  PlanetsCentral-specific functions
         */

        /** Log in.
            Produces an asynchronous task that, when run, will try to make sure that the user is logged in.
            On success, the Account will have attributes `api_user`, `api_key` set.
            On failure, these attributes will be empty.

            @param acc  Account
            @param then Task to execute after logging in
            @return Task */
        std::auto_ptr<Task_t> login(game::browser::Account& acc, std::auto_ptr<Task_t> then);

        /** Call server.
            @param acc      Account (for API endpoint address)
            @param endpoint Endpoint name (e.g. 'file')
            @param args     Parameters to pass (including `api_key` etc.)
            @return Raw result; null on error */
        std::auto_ptr<afl::data::Value> callServer(game::browser::Account& acc, String_t endpoint, const afl::net::HeaderTable& args);

        /** Get game list, pre-authenticated.
            The account must have been logged in already.
            If the account is not or no longer logged in, the request will fail.
            @param acc      Account
            @return Result tree from API, hash. Null on error. Owned by BrowserHandler and valid until the next call. */
        afl::data::Access getGameListPreAuthenticated(game::browser::Account& acc);

        /** Get directory content, pre-authenticated.
            The account must have been logged in already.
            If the account is not or no longer logged in, the request will fail.
            @param acc      Account
            @param dirName  Path name of directory
            @param listener Receives result
            @return Result tree from API, hash:
            - result (success flag)
            - reply (on success, list of items)
            - error (on failure, error message)
            If not logged in, null. */
        std::auto_ptr<afl::data::Value> getDirectoryContentPreAuthenticated(game::browser::Account& acc, String_t dirName);

        /** Download a file, pre-authenticated.
            The account must have been logged in already.
            If the account is not or no longer logged in, the request will fail.
            @param acc      Account
            @param fileName Path name of file
            @param listener Receives result */
        void getFilePreAuthenticated(game::browser::Account& acc, String_t fileName, afl::net::http::DownloadListener& listener);

        /** Access translator.
            @return translator */
        afl::string::Translator& translator();

        /** Access logger.
            @return logger */
        afl::sys::LogListener& log();

        /** Access RootLoader.
            Persistent state in this object is the host version number parser definition (hostver.ini).
            @return RootLoader */
        game::v3::RootLoader& loader();

     private:
        game::browser::Browser& m_browser;
        afl::net::http::Manager& m_manager;
        afl::io::NullFileSystem m_nullFS;
        game::v3::RootLoader m_v3Loader;

        // Cache:
        std::auto_ptr<afl::data::Value> m_gameList;
        game::browser::Account* m_gameListAccount;
    };

} }

#endif
