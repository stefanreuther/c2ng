/**
  *  \file game/browser/rootfolder.hpp
  */
#ifndef C2NG_GAME_BROWSER_ROOTFOLDER_HPP
#define C2NG_GAME_BROWSER_ROOTFOLDER_HPP

#include "game/browser/folder.hpp"

namespace game { namespace browser {

    class Browser;

    class RootFolder : public Folder {
     public:
        RootFolder(Browser& parent);

        ~RootFolder();

        virtual void loadContent(afl::container::PtrVector<Folder>& result);

        virtual afl::base::Ptr<Root> loadGameRoot();

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
