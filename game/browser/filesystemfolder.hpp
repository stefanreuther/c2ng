/**
  *  \file game/browser/filesystemfolder.hpp
  *  \brief Class game::browser::FileSystemFolder
  */
#ifndef C2NG_GAME_BROWSER_FILESYSTEMFOLDER_HPP
#define C2NG_GAME_BROWSER_FILESYSTEMFOLDER_HPP

#include "afl/base/ref.hpp"
#include "afl/io/directory.hpp"
#include "game/browser/synchronousfolder.hpp"

namespace game { namespace browser {

    class Browser;

    /** File system folder.
        Publishes an arbitrary file system directory (afl::io::Directory).
        The directory can be virtual (=no path name, Directory::getDirectoryName()).

        If the directory contains index a Winplan gamestat.dat file, it is parsed to label subdirectories. */
    class FileSystemFolder : public SynchronousFolder {
     public:
        /** Constructor.
            @param parent      Browser instance
            @param dir         Directory
            @param title       Title to use; allows using different names than the actual directory title (Directory::getTitle())
            @param ignoreIndex true to ignore index information (gamestat.dat). Used to implement the "[Directory content]" entry */
        FileSystemFolder(Browser& parent, afl::base::Ref<afl::io::Directory> dir, String_t title, bool ignoreIndex);

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
        afl::base::Ref<afl::io::Directory> m_directory;
        String_t m_title;
        bool m_ignoreIndex;
    };

} }

#endif
