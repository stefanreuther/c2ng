/**
  *  \file game/browser/browser.hpp
  */
#ifndef C2NG_GAME_BROWSER_BROWSER_HPP
#define C2NG_GAME_BROWSER_BROWSER_HPP

#include <memory>
#include "afl/string/string.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/browser/rootfolder.hpp"
#include "game/root.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/base/inlineoptional.hpp"
#include "game/browser/handlerlist.hpp"
#include "game/config/userconfiguration.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace browser {

    class Folder;
    class Handler;
    class Account;
    class AccountManager;
    class UserCallback;

    class Browser {
     public:
        typedef afl::base::InlineOptional<size_t,size_t(-1)> OptionalIndex_t;

        Browser(afl::io::FileSystem& fileSystem,
                afl::string::Translator& tx,
                afl::sys::LogListener& log,
                AccountManager& accounts,
                util::ProfileDirectory& profile,
                UserCallback& callback);
        ~Browser();

        afl::io::FileSystem& fileSystem();
        afl::string::Translator& translator();
        afl::sys::LogListener& log();
        AccountManager& accounts();
        UserCallback& callback();
        HandlerList& handlers();

        void openFolder(String_t name);
        void openChild(size_t n);
        void openParent();

        void selectChild(size_t n);

        Folder& currentFolder();

        void loadContent();
        void loadChildRoot();
        void clearContent();

        const afl::container::PtrVector<Folder>& path();
        const afl::container::PtrVector<Folder>& content();

        OptionalIndex_t getSelectedChild() const;
        afl::base::Ptr<Root> getSelectedRoot() const;
        game::config::UserConfiguration* getSelectedConfiguration() const;

        void updateConfiguration();

        Folder* createAccountFolder(Account& account);
        afl::base::Ptr<Root> loadGameRoot(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config);

        String_t expandGameDirectoryName(String_t directoryName) const;
        void setSelectedLocalDirectoryName(String_t directoryName);
        void setSelectedLocalDirectoryAutomatically();

     private:
        afl::io::FileSystem& m_fileSystem;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
        AccountManager& m_accounts;
        util::ProfileDirectory& m_profile;
        UserCallback& m_callback;

        // List of handlers.
        HandlerList m_handlers;

        // Path of folders.
        afl::container::PtrVector<Folder> m_path;
        std::auto_ptr<Folder> m_pathOrigin;

        // Content of folder.
        afl::container::PtrVector<Folder> m_content;

        // Root folder
        RootFolder m_rootFolder;

        // Selecting a child
        OptionalIndex_t m_selectedChild;
        bool m_childLoaded;
        afl::base::Ptr<Root> m_childRoot;
        std::auto_ptr<game::config::UserConfiguration> m_childConfig;

        bool trySetLocalDirectoryName(afl::io::Directory& gamesDir, String_t directoryName);
    };

} }

#endif
