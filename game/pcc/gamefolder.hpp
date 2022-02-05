/**
  *  \file game/pcc/gamefolder.hpp
  *  \brief Class game::pcc::GameFolder
  */
#ifndef C2NG_GAME_PCC_GAMEFOLDER_HPP
#define C2NG_GAME_PCC_GAMEFOLDER_HPP

#include "afl/data/access.hpp"
#include "game/browser/account.hpp"
#include "game/browser/synchronousfolder.hpp"

namespace game { namespace pcc {

    class BrowserHandler;

    /** Constructor.
        Displays one game of an account. */
    class GameFolder : public game::browser::SynchronousFolder {
     public:
        /** Constructor.
            @param handler Parent BrowserHandler
            @param acc     Account
            @param path    Path name; used to identify the game
            @param hint    Index into game list, used as an optimisation */
        GameFolder(BrowserHandler& handler, game::browser::Account& acc, String_t path, size_t hint);

        // Folder:
        virtual void loadContent(afl::container::PtrVector<Folder>& result);
        virtual bool loadConfiguration(game::config::UserConfiguration& config);
        virtual void saveConfiguration(const game::config::UserConfiguration& config);
        virtual bool setLocalDirectoryName(String_t directoryName);
        virtual std::auto_ptr<Task_t> loadGameRoot(const game::config::UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t> then);
        virtual String_t getName() const;
        virtual util::rich::Text getDescription() const;
        virtual bool isSame(const Folder& other) const;
        virtual bool canEnter() const;
        virtual Kind getKind() const;

     private:
        BrowserHandler& m_handler;
        game::browser::Account& m_account;
        String_t m_path;
        mutable size_t m_hint;

        afl::data::Access getGameListEntry() const;
    };

} }

#endif
