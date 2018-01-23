/**
  *  \file server/host/rank/scorefilereader.hpp
  *  \brief Class server::host::rank::ScoreFileReader
  */
#ifndef C2NG_SERVER_HOST_RANK_SCOREFILEREADER_HPP
#define C2NG_SERVER_HOST_RANK_SCOREFILEREADER_HPP

#include "afl/base/types.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "server/host/game.hpp"
#include "util/fileparser.hpp"

namespace server { namespace host { namespace rank {

    typedef int32_t Score_t[Game::NUM_PLAYERS];

    /** Reading score files.
        Add-ons can provide scores by creating a file "c2score.txt".
        This file has the format
        <code>
           # Section delimiter starts a named score. 'score' will be the game's main score.
           %score
           # Description (should actually just be the name)
           Description = PTScore
           # Scores
           Score7 = 59999

           # There can be multiple scores:
           %another
           Description = Another Score
           Score7 = 12
        </code>
        This class parses such a file and populates the database.

        Usage:
        - construct, passing it database keys
        - call parseFile()
        - call flush() */
    class ScoreFileReader : public util::FileParser {
     public:
        /** Constructor.
            \param scoreKey Database key for scores (Turn::scores())
            \param descriptionKey Database key for score descriptions (Game::scoreDescriptions()) */
        ScoreFileReader(afl::net::redis::HashKey scoreKey, afl::net::redis::HashKey descriptionKey);

        // FileParser:
        virtual void handleLine(const String_t& fileName, int lineNr, String_t line);
        virtual void handleIgnoredLine(const String_t& fileName, int lineNr, String_t line);

        /** Finalize.
            Writes out pending last information. */
        void flush();

     private:
        afl::net::redis::HashKey m_scoreKey;
        afl::net::redis::HashKey m_descriptionKey;
        String_t m_name;
        String_t m_description;
        Score_t m_values;
    };

    /** Pack a Score_t into binary format.
        \param score Input
        \return packed scores */
    String_t packScore(const Score_t& score);

} } }

#endif
