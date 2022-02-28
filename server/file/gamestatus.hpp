/**
  *  \file server/file/gamestatus.hpp
  */
#ifndef C2NG_SERVER_FILE_GAMESTATUS_HPP
#define C2NG_SERVER_FILE_GAMESTATUS_HPP

#include <memory>
#include <utility>
#include <vector>
#include "server/interface/filegame.hpp"

namespace server { namespace file {

    class DirectoryItem;
    class Root;

    class GameStatus {
     public:
        // typedef server::interface::FileGame::GameInfo GameInfo_t;
        // typedef server::interface::FileGame::KeyInfo KeyInfo_t;

        typedef std::pair<int32_t, std::string> Slot_t;
        typedef std::vector<Slot_t> Slots_t;

        struct GameInfo {
            // pathName: filled in when creating the external GameInfo
            // gameName, gameId, hostTime, isFinished: taken from properties when creating the external GameInfo
            Slots_t slots;
            afl::data::StringList_t missingFiles;
            String_t hostVersion;
            // conflictSlots: not implemented currently
        };

        struct KeyInfo {
            String_t fileName;
            bool isRegistered;
            String_t label1;
            String_t label2;
            String_t keyId;
        };

        GameStatus();
        ~GameStatus();

        void load(Root& root, DirectoryItem& dir);

        const GameInfo* getGameInfo() const;

        const KeyInfo* getKeyInfo() const;

     private:
        std::auto_ptr<GameInfo> m_game;
        std::auto_ptr<KeyInfo> m_key;
    };

} }

#endif
