/**
  *  \file game/v3/maketurn.hpp
  *  \brief Class game::v3::Maketurn
  */
#ifndef C2NG_GAME_V3_MAKETURN_HPP
#define C2NG_GAME_V3_MAKETURN_HPP

#include "afl/io/directory.hpp"
#include "game/v3/trn/fileset.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/playerlist.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace v3 {

    /** Maketurn function.
        The Maketurn function produces a turn file from an unpacked game directory
        without loading and parsing the entire game.

        Turn files are generated in memory (see game::v3::trn::FileSet) and written out as a group.
        - create Maketurn object
        - call makeTurn() for each player
        - call saveAll() to write them out */
    class Maketurn {
     public:
        // FIXME: do we need a charset? Since we convert game->game, we probably don't need it

        /** Constructor.
            \param dir       Directory
            \param players   Player list
            \param charset   Character set
            \param tx        Translator */
        Maketurn(afl::io::Directory& dir, const PlayerList& players, afl::charset::Charset& charset, afl::string::Translator& tx);

        /** Destructor. */
        ~Maketurn();

        /** Generate turn for a player.
            The turn is generated in memory; this function does not write anything to the disk.
            Call this exactly once for every player in the game directory.

            \param pid Player number [1,NUM_PLAYERS]
            \param log Logger
            \return Number of commands in that turn*/
        size_t makeTurn(int playerNr, afl::sys::LogListener& log);

        /** Finish and write out turn files.
            \param log Logger */
        void saveAll(afl::sys::LogListener& log, afl::io::FileSystem& fs, const game::config::UserConfiguration& config);

        /** Get number of prepared turn files.
            \return number of prepared turn files. */
        size_t getNumFiles() const;

     private:
        afl::io::Directory& m_directory;
        const PlayerList& m_playerList;
        afl::charset::Charset& m_charset;
        afl::string::Translator& m_translator;

        game::v3::trn::FileSet m_turns;
    };

} }

#endif
