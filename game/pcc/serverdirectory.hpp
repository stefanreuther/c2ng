/**
  *  \file game/pcc/serverdirectory.hpp
  *  \brief Class game::pcc::ServerDirectory
  */
#ifndef C2NG_GAME_PCC_SERVERDIRECTORY_HPP
#define C2NG_GAME_PCC_SERVERDIRECTORY_HPP

#include "afl/io/directory.hpp"
#include "game/browser/account.hpp"

namespace game { namespace pcc {

    class BrowserHandler;

    /** Server directory.
        Implements access to the server-side file structure published by the PCC file.cgi API.
        Allows retrieval of files, subdirectories, and parent directory.

        Limitations:
        - read-only for now;
        - cannot authenticate: the account must be logged in previously (use BrowserHandler::login()).
          If the login expires, future accesses will fail until an external component logs in the account again;
        - takes it easy on caching. */
    class ServerDirectory : public afl::io::Directory {
     public:
        /** Constructor.
            @param handler BrowserHandler instance
            @param acc     Account instance (mutable; might eventually invalidate tokens or update caches)
            @param name    Name */
        ServerDirectory(BrowserHandler& handler, game::browser::Account& acc, String_t name);

        /** Destructor. */
        virtual ~ServerDirectory();

        /** Store a file.
            @param name File name
            @param content File content */
        void putFile(String_t name, afl::base::ConstBytes_t content);

        /** Access underlying BrowserHandler.
            @return BrowserHandler */
        BrowserHandler& handler();

        /** Access underlying Account.
            @return Account */
        game::browser::Account& account();

        // Directory virtuals:
        virtual afl::base::Ref<afl::io::DirectoryEntry> getDirectoryEntryByName(String_t name);
        virtual afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > getDirectoryEntries();
        virtual afl::base::Ptr<afl::io::Directory> getParentDirectory();
        virtual String_t getDirectoryName();
        virtual String_t getTitle();

     private:
        typedef std::vector<afl::base::Ptr<afl::io::DirectoryEntry> > ContentVector_t;
        BrowserHandler& m_handler;
        game::browser::Account& m_account;
        String_t m_name;
        bool m_loaded;
        afl::base::Ptr<ContentVector_t> m_entries;

        void load();

        class Entry;
    };

} }

inline game::pcc::BrowserHandler&
game::pcc::ServerDirectory::handler()
{
    return m_handler;
}

inline game::browser::Account&
game::pcc::ServerDirectory::account()
{
    return m_account;
}

#endif
