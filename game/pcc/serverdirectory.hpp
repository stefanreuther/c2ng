/**
  *  \file game/pcc/serverdirectory.hpp
  */
#ifndef C2NG_GAME_PCC_SERVERDIRECTORY_HPP
#define C2NG_GAME_PCC_SERVERDIRECTORY_HPP

#include "afl/io/directory.hpp"
#include "game/browser/account.hpp"

namespace game { namespace pcc {

    class BrowserHandler;

    class ServerDirectory : public afl::io::Directory {
     public:
        ServerDirectory(BrowserHandler& handler, game::browser::Account& acc, String_t name);
        virtual ~ServerDirectory();

        virtual afl::base::Ptr<afl::io::DirectoryEntry> getDirectoryEntryByName(String_t name);
        virtual afl::base::Ptr<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > getDirectoryEntries();
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

#endif
