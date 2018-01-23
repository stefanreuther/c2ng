/**
  *  \file server/host/rank/refereefilereader.hpp
  *  \brief Class server::host::rank::RefereeFileReader
  */
#ifndef C2NG_SERVER_HOST_RANK_REFEREEFILEREADER_HPP
#define C2NG_SERVER_HOST_RANK_REFEREEFILEREADER_HPP

#include "util/fileparser.hpp"
#include "server/host/rank/rank.hpp"

namespace server { namespace host { namespace rank {

    /** Reading referee files.
        Add-ons can decide to end the game.
        To do so, they create a file "c2ref.txt":
        <code>
            # Ranking: lower means better, unlisted means last place
            Rank1 = 1
            Rank2 = 2
            # End signalisation: 1=end, 0=keep playing
            End = 1
        </code>
        This class parses such a file.

        Usage:
        - construct
        - call parseFile()
        - inquire using isEnd(), getRanks() */
    class RefereeFileReader : public util::FileParser {
     public:
        /** Default constructor. */
        RefereeFileReader();

        // FileParser:
        virtual void handleLine(const String_t& fileName, int lineNr, String_t line);
        virtual void handleIgnoredLine(const String_t& fileName, int lineNr, String_t line);

        /** Check for game end.
            \return value of the "End=" assignment */
        bool isEnd() const;

        /** Get ranks.
            \return ranks in raw (uncompacted) form. */
        const Rank_t& getRanks() const;

     private:
        bool m_end;
        Rank_t m_ranks;
    };

} } }

#endif
