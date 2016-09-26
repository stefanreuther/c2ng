/**
  *  \file game/nu/gamefolder.hpp
  */
#ifndef C2NG_GAME_NU_GAMEFOLDER_HPP
#define C2NG_GAME_NU_GAMEFOLDER_HPP

#include "game/browser/folder.hpp"
#include "game/browser/account.hpp"
#include "afl/data/access.hpp"
#include "game/nu/gamestate.hpp"

namespace game { namespace nu {

    class BrowserHandler;

    /** Game folder on a Nu server.
        This is a virtual folder representing a single game. */
    class GameFolder : public game::browser::Folder {
     public:
        /** Constructor.
            \param handler BrowserHandler Main browser handler
            \param acc     Account
            \param gameNr  Game number
            \param hint    Position hint; the game is at this index in the game list.
                           This hint is optional but is used to make constructing a list of GameFolder's an O(n) instead of O(n**2) operation. */
        GameFolder(BrowserHandler& handler,
                   game::browser::Account& acc,
                   int32_t gameNr,
                   size_t hint);

        // Folder:
        virtual void loadContent(afl::container::PtrVector<Folder>& result);
        virtual afl::base::Ptr<Root> loadGameRoot();
        virtual String_t getName() const;
        virtual util::rich::Text getDescription() const;
        virtual bool isSame(const Folder& other) const;
        virtual bool canEnter() const;
        virtual Kind getKind() const;

     private:
        BrowserHandler& m_handler;
        game::browser::Account& m_account;

        int32_t m_gameNr;

        afl::base::Ptr<GameState> m_state;
    };

} }

#endif
