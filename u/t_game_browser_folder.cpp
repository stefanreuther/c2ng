/**
  *  \file u/t_game_browser_folder.cpp
  *  \brief Test for game::browser::Folder
  */

#include "game/browser/folder.hpp"

#include "t_game_browser.hpp"

/** Interface test. */
void
TestGameBrowserFolder::testIt()
{
    class Tester : public game::browser::Folder {
     public:
        virtual void loadContent(afl::container::PtrVector<game::browser::Folder>&)
            { }
        virtual afl::base::Ptr<game::Root> loadGameRoot()
            { return 0; }
        virtual String_t getName() const
            { return String_t(); }
        virtual util::rich::Text getDescription() const
            { return ""; }
        virtual bool isSame(const game::browser::Folder&) const
            { return true; }
        virtual bool canEnter() const
            { return false; }
        virtual Kind getKind() const
            { return kFavorite; }
    };
    Tester t;
}

