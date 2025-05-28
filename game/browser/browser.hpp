/**
  *  \file game/browser/browser.hpp
  *  \brief Class game::browser::Browser
  */
#ifndef C2NG_GAME_BROWSER_BROWSER_HPP
#define C2NG_GAME_BROWSER_BROWSER_HPP

#include <memory>
#include "afl/base/inlineoptional.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/browser/handlerlist.hpp"
#include "game/browser/rootfolder.hpp"
#include "game/browser/types.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/root.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace browser {

    class Folder;
    class Handler;
    class Account;
    class AccountManager;
    class UserCallback;

    /** Game browser.
        Provides a virtual hierarchy of games for the user to browse, starting with a RootFolder.

        The Browser object aggregates all objects required for browsing.
        In particular, it owns a list of Handlers that define the set of account types supported.

        <b>Domain Model:</b>

        - the current path is represented as a (possibly empty) sequence of Folder instances
          which is implicitly prepended with a RootFolder.
          The effective position is determined by the last item in the list;
        - content is only known after loadContent() completed, and consists of a list of child folders;
        - after content is loaded, an item can be selected;
          the selected item's game content can be loaded using loadChildRoot();
        - when navigating to a parent folder, the originating folder is tracked
          so that it can be automatically selected in the parent's list.

        <b>Operations:</b>

        Browser operations can be asynchronous.
        In this case, they take a Task_t @c then parameter, and return a Task_t.
        You should not start a new operation before the previous one completed (but you may create it).

        Non-asynchronous methods report whatever status the browser is in.
        They should therefore only be called when no unknown tasks are active.

        Browser provides no way to manage asynchronous tasks; see Session::addTask() for a possibility.

        <b>Lifetimes:</b>

        Folder and Handler instances can assume to not out-live a Browser instance,
        and, transitively, the contained objects (FileSystem, Translator, AccountManager, etc.). */
    class Browser {
     public:
        typedef afl::base::InlineOptional<size_t,size_t(-1)> OptionalIndex_t;

        /** Status for a local directory. */
        enum DirectoryStatus {
            Missing,                                  ///< Directory does not exist.
            Success,                                  ///< Success.
            NotEmpty,                                 ///< Directory exists but is not empty.
            NotWritable                               ///< Directory exists but is not writable.
        };

        /** Constructor.

            @param fileSystem    File system instance (for browsing).
            @param tx            Translator.
            @param log           Logger.
            @param accounts      Configured accounts.
            @param profile       Profile directory (for automatically managed games and global configuration files).
            @param callback      Callback for user interactions. */
        Browser(afl::io::FileSystem& fileSystem,
                afl::string::Translator& tx,
                afl::sys::LogListener& log,
                AccountManager& accounts,
                util::ProfileDirectory& profile,
                UserCallback& callback);
        ~Browser();


        /*
         *  Related objects
         */

        /** Access file system instance,
            @return file system instance */
        afl::io::FileSystem& fileSystem();

        /** Access translator.
            @return translator */
        afl::string::Translator& translator();

        /** Access logger.
            @return logger */
        afl::sys::LogListener& log();

        /** Access account manager.
            @return account manager */
        AccountManager& accounts();

        /** Access callback.
            @return callback */
        UserCallback& callback();

        /** Access profile directory.
            @return profile directory */
        util::ProfileDirectory& profile();

        /** Add new Handler.
            Ownership will be taken over by Browser.
            @param h Handler (should not be null) */
        void addNewHandler(Handler* h);


        /*
         *  Navigation and Data Access
         */

        /** Open folder by name/URL.
            Tries to resolve the given name into a Folder path using Handler::handleFolderName and, if that succeeds, opens it.
            If the name cannot be resolved, does not change anything.

            This does not yet load the folder content; see loadContent().

            @param name Name
            @return true on success, false on failure. */
        bool openFolder(String_t name);

        /** Open child folder.
            If a folder's content has been loaded, enter its n-th child
            by appending the n-th element of content to the current path.

            This does not yet load the folder content; see loadContent().

            @param n Child index (0-based); call is ignored if value is out-of-range */
        void openChild(size_t n);

        /** Open parent folder.
            Goes up one level by discarding the current path's last element.
            Call is ignored if already at top-level.

            This does not yet load the folder content; see loadContent(). */
        void openParent();

        /** Select child.
            This does not yet load the child root; see loadChildRoot().

            @param n Child index (0-based); call is ignored if value is out-of-range */
        void selectChild(size_t n);

        /** Access current folder.
            @return current folder (last in path list) */
        Folder& currentFolder();

        /** Access current path.
            @return path list (not including root folder) */
        const afl::container::PtrVector<Folder>& path();

        /** Access current content.
            @return content */
        const afl::container::PtrVector<Folder>& content();

        /** Discard current content.
            @post content().empty() */
        void clearContent();

        /** Get selected child.
            @return If a valid child folder has been selected, pointer to child. Otherwise, null. */
        Folder* getSelectedChild() const;

        /** Get index of selected child.
            @return Child index as set by selectChild(), if any */
        OptionalIndex_t getSelectedChildIndex() const;

        /** Get root of selected child.
            @return root, if loaded successfully using loadChildRoot(). Otherwise, null. */
        afl::base::Ptr<Root> getSelectedRoot() const;

        /** Get configuration of selected child.
            @return configuration, if loaded successfully using loadChildRoot(); valid until a new child is selected.
                    If loadChildRoot() has not yet been called successfully, null. */
        game::config::UserConfiguration* getSelectedConfiguration() const;

        /** Check whether to suggest setting up a local folder.
            Examines the folder previously loaded using loadChildRoot().
            @return true UI should suggest configuring a local folder */
        bool isSelectedFolderSetupSuggested() const;


        /*
         *  Tasks
         */

        /** Load content of a the current folder.
            Causes the content to be loaded (using Folder::loadContent).
            @param then Completion callback. At this time, the Browser object's content has been updated (content, selected child). */
        std::auto_ptr<Task_t> loadContent(std::auto_ptr<Task_t> then);

        /** Load root of selected child.
            If no child has been selected at this time, does nothing.
            @param then Completion callback. At this time, the Browser object's content has been updated (root, config). */
        std::auto_ptr<Task_t> loadChildRoot(std::auto_ptr<Task_t> then);

        /** Update configuration of selected child.
            Use after updating the configuration.
            Will persist the configuration (Folder::saveConfiguration) and rebuild the root.
            @param then Completion callback. At this time, the Browser object's content has been updated (root, config). */
        std::auto_ptr<Task_t> updateConfiguration(std::auto_ptr<Task_t> then);


        /*
         *  Utilities
         */

        /** Load game root for a directory.
            Wrapper for HandlerList::loadGameRootMaybe, for implementation of FileSystemFolder.

            @param dir    [in] Directory
            @param config [in] Configuration. Must live until the result callback completes.
            @param then   [in] Result callback (always moved-away)
            @return Task (never null) */
        std::auto_ptr<Task_t> loadGameRoot(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config, std::auto_ptr<LoadGameRootTask_t>& then);

        /** Create account folder.
            Wrapper for HandlerList::createAccountFolder, for implementation of RootFolder.

            @param account Account (mutable because it can eventually be modified through the created Folder)
            @return Folder (never null) */
        Folder* createAccountFolder(const afl::base::Ref<Account>& account);

        /** Expand shortcuts in directory name.
            If the directoryName starts with "game:", replaces that by the actual profile directory name.
            This is used to store location-independant game/directory associations in network.ini.

            @param directoryName Directory name to expand
            @return expanded name */
        String_t expandGameDirectoryName(String_t directoryName) const;

        /** Set local directory name for selected child.
            (Call is ignored if no child is selected.)

            @param directoryName Name to store

            @see Folder::setLocalDirectoryName() */
        void setSelectedLocalDirectoryName(String_t directoryName);

        /** Set local directory name for selected child to an automatically derived name.
            (Call is ignored if no child is selected.) */
        void setSelectedLocalDirectoryAutomatically();

        /** Verify status of a directory.
            Check whether the directory can be used as a local directory.

            @param directoryName Directory name you want to use for setSelectedLocalDirectoryName
            @return status */
        DirectoryStatus verifyLocalDirectory(const String_t directoryName);

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

        // After going up, originating folder for initial selection.
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
        std::auto_ptr<Task_t> loadGameRoot(size_t n, std::auto_ptr<Task_t>& then);
    };

} }

#endif
