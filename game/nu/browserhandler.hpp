/**
  *  \file game/nu/browserhandler.hpp
  */
#ifndef C2NG_GAME_NU_BROWSERHANDLER_HPP
#define C2NG_GAME_NU_BROWSERHANDLER_HPP

#include <memory>
#include "game/browser/handler.hpp"
#include "afl/net/http/manager.hpp"
#include "game/browser/browser.hpp"
#include "afl/data/value.hpp"
#include "afl/string/translator.hpp"
#include "afl/data/access.hpp"

namespace game { namespace nu {

    class BrowserHandler : public game::browser::Handler {
     public:
        BrowserHandler(game::browser::Browser& b,
                       afl::net::http::Manager& mgr,
                       afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory);

        // Handler:
        virtual bool handleFolderName(String_t name, afl::container::PtrVector<game::browser::Folder>& result);
        virtual game::browser::Folder* createAccountFolder(game::browser::Account& acc);
        virtual afl::base::Ptr<Root> loadGameRoot(afl::base::Ref<afl::io::Directory> dir);

        // nu::BrowserHandler
        bool login(game::browser::Account& acc);

        std::auto_ptr<afl::data::Value> callServer(game::browser::Account& acc,
                                                   String_t endpoint,
                                                   const afl::net::HeaderTable& args);

        afl::data::Access getGameList(game::browser::Account& acc);

        afl::string::Translator& translator();

        afl::sys::LogListener& log();

        afl::base::Ref<afl::io::Directory> getDefaultSpecificationDirectory();

     private:
        game::browser::Browser& m_browser;
        afl::net::http::Manager& m_manager;
        afl::base::Ref<afl::io::Directory> m_defaultSpecificationDirectory;

        // Cache:
        std::auto_ptr<afl::data::Value> m_gameList;
        String_t m_gameListAccount;
        // FIXME: time...
    };

} }

#endif
