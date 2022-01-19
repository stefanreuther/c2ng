/**
  *  \file game/browser/rootfolder.hpp
  *  \brief Class game::browser::RootFolder
  */
#ifndef C2NG_GAME_BROWSER_ROOTFOLDER_HPP
#define C2NG_GAME_BROWSER_ROOTFOLDER_HPP

#include "game/browser/synchronousfolder.hpp"

namespace game { namespace browser {

    class Browser;

    /** Browser root folder.
        Publishes the root of the browsing structure,
        containing links to all accounts and the file system root,
        but no game data. */
    class RootFolder : public SynchronousFolder {
     public:
        /** Constructor.
            @param parent Browser instance */
        explicit RootFolder(Browser& parent);

        /** Destructor. */
        ~RootFolder();

        // Folder:
        virtual void loadContent(afl::container::PtrVector<Folder>& result);
        virtual bool loadConfiguration(game::config::UserConfiguration& config);
        virtual void saveConfiguration(const game::config::UserConfiguration& config);
        virtual bool setLocalDirectoryName(String_t directoryName);
        virtual std::auto_ptr<Task_t> loadGameRoot(const game::config::UserConfiguration& config, std::auto_ptr<LoadGameRootTask_t> then);
        virtual String_t getName() const;
        virtual util::rich::Text getDescription() const;
        virtual bool isSame(const Folder& other) const;
        virtual bool canEnter() const;
        virtual Kind getKind() const;

     private:
        Browser& m_parent;
    };

} }

#endif
