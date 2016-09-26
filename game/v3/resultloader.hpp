/**
  *  \file game/v3/resultloader.hpp
  */
#ifndef C2NG_GAME_V3_RESULTLOADER_HPP
#define C2NG_GAME_V3_RESULTLOADER_HPP

#include "game/turnloader.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/map/universe.hpp"
#include "game/playerset.hpp"
#include "game/v3/directoryscanner.hpp"
#include "game/playerarray.hpp"
#include "afl/io/filesystem.hpp"

namespace game { namespace v3 {

    class ResultLoader : public TurnLoader {
     public:
        ResultLoader(afl::charset::Charset& charset,
                     afl::string::Translator& tx,
                     afl::sys::LogListener& log,
                     const DirectoryScanner& scanner,
                     afl::io::FileSystem& fs);

        virtual PlayerStatusSet_t getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const;

        virtual void loadCurrentTurn(Turn& turn, Game& game, int player, Root& root);

        virtual void getHistoryStatus(int player, int turn, afl::base::Memory<HistoryStatus> status, const Root& root);

        virtual void loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root);

        void loadResult(Turn& trn, Root& root, Game& game, afl::io::Stream& file, int player, bool withBackup);

     private:
        afl::charset::Charset& m_charset;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
        afl::io::FileSystem& m_fileSystem;

        PlayerArray<DirectoryScanner::PlayerFlags_t> m_playerFlags;
    };

} }

#endif
