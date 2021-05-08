/**
  *  \file game/browser/filesystemfolder.hpp
  */
#ifndef C2NG_GAME_BROWSER_FILESYSTEMFOLDER_HPP
#define C2NG_GAME_BROWSER_FILESYSTEMFOLDER_HPP

#include "game/browser/folder.hpp"
#include "afl/base/ref.hpp"
#include "afl/io/directory.hpp"

namespace game { namespace browser {

    class Browser;

    class FileSystemFolder : public Folder {
     public:
        FileSystemFolder(Browser& parent, afl::base::Ref<afl::io::Directory> dir, String_t title, bool ignoreIndex);

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
        afl::base::Ref<afl::io::Directory> m_directory;
        String_t m_title;
        bool m_ignoreIndex;
    };

} }

#endif
