/**
  *  \file game/browser/filesystemrootfolder.hpp
  */
#ifndef C2NG_GAME_BROWSER_FILESYSTEMROOTFOLDER_HPP
#define C2NG_GAME_BROWSER_FILESYSTEMROOTFOLDER_HPP

#include "game/browser/folder.hpp"
#include "afl/io/filesystem.hpp"

namespace game { namespace browser {

    class Browser;

    class FileSystemRootFolder : public Folder {
     public:
        FileSystemRootFolder(Browser& parent);

        virtual void loadContent(afl::container::PtrVector<Folder>& result);
        virtual bool loadConfiguration(game::config::UserConfiguration& config);
        virtual void saveConfiguration(const game::config::UserConfiguration& config);
        virtual bool setLocalDirectoryName(String_t directoryName);
        virtual afl::base::Ptr<Root> loadGameRoot(const game::config::UserConfiguration& config);
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
