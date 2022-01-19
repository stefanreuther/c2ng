/**
  *  \file game/browser/filesystemrootfolder.hpp
  *  \brief Class game::browser::FileSystemRootFolder
  */
#ifndef C2NG_GAME_BROWSER_FILESYSTEMROOTFOLDER_HPP
#define C2NG_GAME_BROWSER_FILESYSTEMROOTFOLDER_HPP

#include "afl/io/filesystem.hpp"
#include "game/browser/synchronousfolder.hpp"

namespace game { namespace browser {

    class Browser;

    /** File system root folder.
        Publishes the root of the file system, as defined by afl::io::FileSystem. */
    class FileSystemRootFolder : public SynchronousFolder {
     public:
        /** Constructor.
            @param parent Browser instance */
        explicit FileSystemRootFolder(Browser& parent);

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
