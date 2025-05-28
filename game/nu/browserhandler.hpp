/**
  *  \file game/nu/browserhandler.hpp
  *  \brief Class game::nu::BrowserHandler
  */
#ifndef C2NG_GAME_NU_BROWSERHANDLER_HPP
#define C2NG_GAME_NU_BROWSERHANDLER_HPP

#include <memory>
#include "afl/data/access.hpp"
#include "afl/data/value.hpp"
#include "afl/net/http/manager.hpp"
#include "afl/string/translator.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/handler.hpp"

namespace game { namespace nu {

    /** planets.nu server integration entrypoint.
        Provides functionality for accounts of type "nu" talking to <https://planets.nu/>.

        Basic logic: the asynchronous login() flow is used to obtain an API key using the /account/login endpoint.
        This will perform user interaction.
        When the API key is known, future commands use that key only.
        If the key expires, operations start to fail.

        The login() flow should therefore be invoked at all places where possible. */
    class BrowserHandler : public game::browser::Handler {
     public:
        /** Constructor.
            @param b        Owning Browser
            @param mgr      HTTP Manager
            @param defaultSpecificationDirectory Default specification */
        BrowserHandler(game::browser::Browser& b,
                       afl::net::http::Manager& mgr,
                       afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory);

        // Handler:
        virtual bool handleFolderName(String_t name, afl::container::PtrVector<game::browser::Folder>& result);
        virtual game::browser::Folder* createAccountFolder(const afl::base::Ref<game::browser::Account>& acc);
        virtual std::auto_ptr<Task_t> loadGameRootMaybe(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t>& then);

        /*
         *  Nu specific functions
         */

        /** Log in.
            Produces an asynchronous task that, when run, will try to make sure that the user is logged in.
            On success, the Account will have the attribute `api_key` set.
            On failure, this attribute will be empty.

            @param acc  Account
            @param then Task to execute after logging in
            @return Task */
        std::auto_ptr<Task_t> login(const afl::base::Ref<game::browser::Account>& acc, std::auto_ptr<Task_t> then);

        /** Call server.
            @param acc      Account (for API endpoint address)
            @param endpoint Endpoint name (must start with slash, e.g. '/account/mygames?version=2')
            @param args     Parameters to pass (including `apikey` etc.)
            @return Raw result; null on error */
        std::auto_ptr<afl::data::Value> callServer(const afl::base::Ref<game::browser::Account>& acc,
                                                   String_t endpoint,
                                                   const afl::net::HeaderTable& args);

        /** Get game list, pre-authenticated.
            The account must have been logged in already.
            If the account is not or no longer logged in, the request will fail.
            @param acc      Account
            @return Result tree from API, hash. Null on error. Owned by BrowserHandler and valid until the next call. */
        afl::data::Access getGameListPreAuthenticated(const afl::base::Ref<game::browser::Account>& acc);

        /** Get account info, pre-authenticated.
            The account must have been logged in already.
            If the account is not or no longer logged in, the request will fail.
            @param acc      Account
            @return Result tree from API, hash. Null on error. Owned by BrowserHandler and valid until the next call. */
        afl::data::Access getAccountInfoPreAuthenticated(const afl::base::Ref<game::browser::Account>& acc);

        /** Access translator.
            @return translator */
        afl::string::Translator& translator();

        /** Access logger.
            @return logger */
        afl::sys::LogListener& log();

        /** Access browser.
            @return browser */
        game::browser::Browser& browser();

        /** Get default specification directory.
            @return default specification directory as passed to constructor */
        afl::base::Ref<afl::io::Directory> getDefaultSpecificationDirectory();

     private:
        class LoginTask;

        game::browser::Browser& m_browser;
        afl::net::http::Manager& m_manager;
        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;

        // Cache:
        std::auto_ptr<afl::data::Value> m_gameList;
        afl::base::Ptr<game::browser::Account> m_gameListAccount;

        std::auto_ptr<afl::data::Value> m_accountInfo;
        afl::base::Ptr<game::browser::Account> m_accountInfoAccount;
    };

} }

#endif
