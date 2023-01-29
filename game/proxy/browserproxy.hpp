/**
  *  \file game/proxy/browserproxy.hpp
  *  \brief Class game::proxy::BrowserProxy
  */
#ifndef C2NG_GAME_PROXY_BROWSERPROXY_HPP
#define C2NG_GAME_PROXY_BROWSERPROXY_HPP

#include <vector>
#include "afl/base/optional.hpp"
#include "afl/base/signal.hpp"
#include "afl/io/filesystem.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/folder.hpp"
#include "game/browser/session.hpp"
#include "game/browser/usercallback.hpp"
#include "game/playerarray.hpp"
#include "game/registrationkey.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/rich/text.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Game browser proxy.
        Proxies a game::browser::Browser.

        Provides bidirectional, synchronous and asynchronous operations for various use-cases:
        - browsing
        - configuration
        - information inquiry

        Game-side UserCallback requests will be reflected into the UI side.

        As of 20220205, this proxy's behaviour is a direct port of the original ad-hoc code
        with almost no changes to semantics. */
    class BrowserProxy {
     public:
        /** Shortcut: optional position in list. */
        typedef game::browser::Browser::OptionalIndex_t OptionalIndex_t;

        /** Shortcut: directory status. */
        typedef game::browser::Browser::DirectoryStatus DirectoryStatus_t;

        /** Information about a folder (path or content item). */
        struct Item {
            String_t name;                            ///< Name (shown to user). @see game::browser::Folder::getName().
            game::browser::Folder::Kind kind;         ///< Kind. @see game::browser::Folder::getKind().
            bool canEnter;                            ///< Can be entered. @see game::browser::Folder::canEnter().
            Item(const String_t& name, game::browser::Folder::Kind kind, bool canEnter)
                : name(name), kind(kind), canEnter(canEnter)
                { }
        };

        /** Information about a browsing context. */
        struct Info {
            std::vector<Item> path;                   ///< Current position as list of nested path items.
            std::vector<Item> content;                ///< Content.
            OptionalIndex_t index;                    ///< Cursor position. When going up, index of the item we're coming from.
        };

        /** Detail information about a folder. */
        struct FolderInfo {
            String_t title;                           ///< Title (name of folder or game). @see game::browser::Folder::getName().
            util::rich::Text subtitle;                ///< Subtitle (description of folder or game). @see game::browser::Folder::getDescription().
            bool canEnter;                            ///< Can be entered. @see game::browser::Folder::canEnter().

            PlayerSet_t availablePlayers;             ///< Set of available players.
            PlayerArray<String_t> playerNames;        ///< Names of available players.
            PlayerArray<String_t> playerExtra;        ///< Extra information of available players (subtitle). @see game::TurnLoader::getPlayerStatus().

            Root::Actions_t possibleActions;          ///< Set of possible actions. @see game::Root::getPossibleActions().

            RegistrationKey::Status keyStatus;        ///< Status of registration key. Relevant only if availablePlayers is not empty. @see game::RegistrationKey::getStatus().
            String_t keyName;                         ///< Name of registration key.

            FolderInfo()
                : title(), subtitle(), canEnter(),
                  availablePlayers(), playerNames(), playerExtra(),
                  possibleActions(),
                  keyStatus(RegistrationKey::Unknown), keyName()
                { }
        };

        /** Folder configuration.
            All items are optional (unset/unsettable, or actual value). */
        struct Configuration {
            afl::base::Optional<String_t> charsetId;  ///< Character set name (Game_Charset, aConfigureCharset).
            afl::base::Optional<bool> finished;       ///< true if game is finished (Game_Finished, aConfigureFinished).
            afl::base::Optional<bool> readOnly;       ///< true if game shall be opened read-only (Game_ReadOnly, aConfigureReadOnly).
            Configuration()
                : charsetId(), finished(), readOnly()
                { }
        };

        /** Constructor.
            @param sender Requests to game side
            @param reply  Responses to UI side
            @param callback Callback for user actions */
        BrowserProxy(util::RequestSender<game::browser::Session> sender, util::RequestDispatcher& reply, game::browser::UserCallback& callback);

        /** Destructor. */
        ~BrowserProxy();

        /** Load content of current position.
            Responds with sig_update.
            @see game::browser::Browser::loadContent() */
        void loadContent();

        /** Open child folder.
            Loads the folder's content and responds with sig_update.
            @param nr Child index (0-based); call is ignored if value is out-of-range
            @see game::browser::Browser::openChild */
        void openChild(size_t nr);

        /** Open parent folder.
            Goes up a number of levels and responds with sig_update.
            @param nr Number of levels to go up
            @see game::browser::Browser::openParent */
        void openParent(size_t nr);

        /** Open folder by name/URL.
            Tries to resolve the given name into a Folder and open it.
            Responds with sig_update.
            @param name Folder name
            @see game::browser::Browser::openFolder */
        void openFolder(String_t name);

        /** Select folder and report information.
            Loads the content and reports sig_selectedInfoUpdate.
            @param index Index into current folder (unset: report information about current folder; index: report information about index'th child)
            @see game::browser::Browser::loadChildRoot */
        void selectFolder(OptionalIndex_t index);

        /** Check whether to suggest setting up a local folder.
            Examines the folder previously loaded using selectFolder();
            must be called after the sig_selectedInfoUpdate callback.
            @param ind WaitIndicator
            @return true UI should suggest configuring a local folder
            @see game::browser::Browser::isSelectedFolderSetupSuggested() */
        bool isSelectedFolderSetupSuggested(WaitIndicator& ind);

        /** Set local directory, automatically.
            Updates the folder previously loaded using selectFolder();
            must be called after the sig_selectedInfoUpdate callback.
            @see game::browser::Browser::setLocalDirectoryAutomatically() */
        void setLocalDirectoryAutomatically();

        /** Set local directory to given name.
            Updates the folder previously loaded using selectFolder();
            must be called after the sig_selectedInfoUpdate callback.
            @param dirName Directory name
            @see game::browser::Browser::setSelectedLocalDirectoryName() */
        void setLocalDirectoryName(const String_t& dirName);

        /** Verify status of a directory.
            @param ind WaitIndicator
            @param dirName Directory name to check
            @return status
            @see game::browser::Browser::verifyLocalDirectory() */
        DirectoryStatus_t verifyLocalDirectory(WaitIndicator& ind, const String_t& dirName);

        /** Set local directory to none.
            Updates the folder previously loaded using selectFolder();
            must be called after the sig_selectedInfoUpdate callback. */
        void setLocalDirectoryNone();

        /** Get current configuration.
            Retrieves configuration of the folder previously loaded using selectFolder();
            must be called after the sig_selectedInfoUpdate callback.

            Only the values that are configurable for this folder will be set.

            @param [in,out] ind     WaitIndicator
            @param [out]    config  Configuration */
        void getConfiguration(WaitIndicator& ind, Configuration& config);

        /** Change configuration.
            Updates configuration of the folder previously loaded using selectFolder();
            must be called after the sig_selectedInfoUpdate callback.

            Updates the values that are set in the given configuration object.

            @param [in,out] ind     WaitIndicator
            @param [in]     config  Configuration */
        void setConfiguration(WaitIndicator& ind, const Configuration& config);

        /** Add an account.
            @param ind WaitIndicator
            @param user User name
            @param type Account type
            @param host Host name
            @return true on success, false if this account already exists */
        bool addAccount(WaitIndicator& ind, String_t user, String_t type, String_t host);

        /** Access underlying file system.
            @return RequestSender to access file system */
        util::RequestSender<afl::io::FileSystem> fileSystem();

        /** Signal: folder content update.
            The proxy tries to avoid out-of-date callbacks, that is,
            a sequence of multiple open() calls will generate only one callback.
            @param info Folder identification and content */
        afl::base::Signal<void(const Info&)> sig_update;

        /** Signal: update information about selected folder.
            @param index Index (unset: current folder; index: n-th child)
            @param info  Information */
        afl::base::Signal<void(OptionalIndex_t, const FolderInfo&)> sig_selectedInfoUpdate;

     private:
        class UpdateTask;
        class PostLoadTask;
        class Trampoline;
        class TrampolineFromSession;

        game::browser::UserCallback& m_callback;
        util::RequestReceiver<BrowserProxy> m_reply;
        util::RequestSender<Trampoline> m_sender;
        afl::base::SignalConnection conn_passwordResult;
        int m_numUpdatesPending;

        void onPasswordResult(game::browser::UserCallback::PasswordResponse resp);
    };

} }

#endif
