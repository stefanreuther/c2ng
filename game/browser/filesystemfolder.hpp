/**
  *  \file game/browser/filesystemfolder.hpp
  */
#ifndef C2NG_GAME_BROWSER_FILESYSTEMFOLDER_HPP
#define C2NG_GAME_BROWSER_FILESYSTEMFOLDER_HPP

#include "game/browser/folder.hpp"
#include "afl/base/ptr.hpp"
#include "afl/io/directory.hpp"

namespace game { namespace browser {

    class Browser;

    class FileSystemFolder : public Folder {
     public:
        FileSystemFolder(Browser& parent, afl::base::Ptr<afl::io::Directory> dir, String_t title);

        virtual void loadContent(afl::container::PtrVector<Folder>& result);

        virtual afl::base::Ptr<Root> loadGameRoot();

        virtual String_t getName() const;

        virtual util::rich::Text getDescription() const;

        virtual bool isSame(const Folder& other) const;

        virtual bool canEnter() const;

        virtual Kind getKind() const;

     private:
        Browser& m_parent;
        afl::base::Ptr<afl::io::Directory> m_directory;
        String_t m_title;
    };

} }

#endif
