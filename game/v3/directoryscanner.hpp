/**
  *  \file game/v3/directoryscanner.hpp
  *  \brief Class game::v3::DirectoryScanner
  */
#ifndef C2NG_GAME_V3_DIRECTORYSCANNER_HPP
#define C2NG_GAME_V3_DIRECTORYSCANNER_HPP

#include "afl/bits/smallset.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/directory.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/hostversion.hpp"
#include "game/v3/structures.hpp"
#include "game/parser/messageparser.hpp"

namespace game { namespace v3 {

    /** Game Directory Overview.
        This class scans a directory for usable game files.

        For each player, we can find a result file (HaveResult) or an unpacked game directory (HaveUnpacked),
        or both belonging to the same turn. In addition, there can be a matching TRN (HaveTurn).

        In addition, we might find a new RST (HaveNewResult) or another RST (HaveOtherResult).

        If there is a conflict, the conflicting data is marked (HaveConflict);
        essentially, this says one can safely load all non-conflicting data. */
    class DirectoryScanner {
     public:
        static const int NUM_PLAYERS = structures::NUM_PLAYERS;
        enum PlayerFlag {
            HaveResult,         ///< We have a current result.
            HaveTurn,           ///< We have a current turn.
            HaveUnpacked,       ///< We have a current game directory.
            HaveNewResult,      ///< We have a new result (along with the game directory).
            HaveConflict,       ///< We have a conflict. For a player: another player has newer data.
            HaveOtherResult     ///< We have a result which is neither current nor new.
        };
        typedef afl::bits::SmallSet<PlayerFlag> PlayerFlags_t;

        /** Mode for scan(). */
        enum Mode {
            /** Look for unpacked files; if none found, look for results.
                If some unpacked files exist, determines their result status, but does not look at other results.
                This is the correct mode for loading games.
                Replaces resultOnly=false. */
            UnpackedThenResult,

            /** Only look for result files.
                Will produce only HaveResult.
                Replaces resultOnly=true. */
            ResultOnly,

            /** Only look for unpacked files, does not look at result files.
                Will not produce HaveResult, HaveNewResult, HaveOtherResult.
                Useful for Maketurn. */
            UnpackedOnly,

            /** Look for unpacked files and results.
                Unlike UnpackedThenResult, also looks for results when some unpacked data exists. */
            UnpackedAndResult
        };

        /** Construct empty overview.
            Call scan() to fill it.
            \param specificationDirectory Specification directory (for host version detection)
            \param tx Message translator
            \param log Logger */
        DirectoryScanner(afl::io::Directory& specificationDirectory, afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Construct empty overview without host-version detection.
            Call scan() to fill it.
            \param tx Message translator
            \param log Logger */
        DirectoryScanner(afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Scan for files.
            This will populate this object with information about the specified directory.

            The basic idea is to scan for unpacked data first, and if none found, accept RSTs.
            PCC2 does not need data be unpacked before playing, but if there is unpacked data,
            we should of course use that instead of the RST and a probably older TRN.
            \param dir Directory to scan
            \param charset Directory character set
            \param mode Scanner mode */
        void scan(afl::io::Directory& dir, afl::charset::Charset& charset, Mode mode);

        /** Clear stored state. */
        void clear();

        /** Get flags for one player.
            \param playerId Player number (1..NUM_PLAYERS)
            \return flags. Empty if playerId is out of range */
        PlayerFlags_t getPlayerFlags(int playerId) const;

        /** Get directory flags.
            \return directory flags; union of all player flags. */
        PlayerFlags_t getDirectoryFlags() const;

        /** Get players that have a particular flag set.
            \param flags flags Flags to check.
            \return set of all players that have at least one of the requested flags set */
        PlayerSet_t getPlayersWhere(PlayerFlags_t flags) const;

        /** Get host version.
            Returns the best guess.

            There is the possibility that data from different host versions is mixed in one directory.
            Normally, these will have different timestamps, and therefore one set is picked as valid, the other ones are marked conflicting.
            The assumption is that these valid files are all from the same host version.

            If the two-argument constructor has been used to construct this DirectoryScanner,
            no host version will be produced; the return value will be a default-initialized HostVersion object.

            \return host version */
        HostVersion getDirectoryHostVersion() const;

        /** Get default player.
            If this directory contains only one player, return that.
            \return player number; 0 if none or ambiguous */
        int getDefaultPlayer() const;

     private:
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
        PlayerFlags_t m_playerFlags[NUM_PLAYERS];
        HostVersion m_hostVersions[NUM_PLAYERS];
        game::parser::MessageParser m_messageParser;

        bool checkResult(afl::io::Directory& dir, afl::charset::Charset& charset, int playerId, game::v3::structures::ResultGen& rgen, HostVersion* pVersion);
        void checkHostVersion(afl::io::Stream& stream, afl::charset::Charset& charset, game::HostVersion& version);
        void initMessageParser(afl::io::Directory& dir);
    };

} }

#endif
