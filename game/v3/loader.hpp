/**
  *  \file game/v3/loader.hpp
  *  \brief Class game::v3::Loader
  */
#ifndef C2NG_GAME_V3_LOADER_HPP
#define C2NG_GAME_V3_LOADER_HPP

#include "afl/charset/charset.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/map/universe.hpp"
#include "game/msg/inbox.hpp"
#include "game/playerset.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "game/v3/structures.hpp"

namespace game { namespace v3 {

    /** v3 Loader Utilities.
        Conventions for v3:
        - most objects are created beforehand: ships, planets, ion storms (prepareUniverse() function)
        - data segments for those objects are loaded by individual functions. Each of those only accesses existing objects
          and thus implicitly detects out-of-range Ids. */
    class Loader {
     public:
        /** Constructor.
            \param charset Game character set
            \param tx Translator
            \param log Logger */
        Loader(afl::charset::Charset& charset, afl::string::Translator& tx, afl::sys::LogListener& log);

        enum LoadMode {
            LoadCurrent,
            LoadPrevious,
            LoadBoth
        };

        /** Target file format. */
        enum TargetFormat {
            TargetPlaintext,            ///< Plaintext file. Standard in Dosplan etc.
            TargetEncrypted             ///< Encrypted file. Winplan's additional targets.
        };

        /** Prepare universe.
            This creates all objects that are not created by the load functions.
            \param univ Target universe */
        void prepareUniverse(game::map::Universe& univ) const;

        /** Prepare turn.
            Creates v3 stuff.
            - Reverter
            - CommandExtra
            - alliance handler
            Call before loading data.
            \param turn Target turn
            \param root Associated root */
        void prepareTurn(Turn& turn, const Root& root, Session& session, int player) const;

        /** Load planets.
            Loads PDATAx.DAT or the appropriate section from a result.
            \param univ Target universe
            \param file File to read from
            \param count Number of planets to load
            \param mode Load mode (load current and/or previous data)
            \param source Source of this information */
        void loadPlanets(game::map::Universe& univ, afl::io::Stream& file, int count, LoadMode mode, PlayerSet_t source) const;

        /** Load planet coordinates.
            Loads an XYPLAN.DAT file.
            \param univ Target universe
            \param file File to read from */
        void loadPlanetCoordinates(game::map::Universe& univ, afl::io::Stream& file) const;

        /** Load Planet Names.
            Loads a PLANET.NM file.
            \param univ Target universe
            \param file File to read from */
        void loadPlanetNames(game::map::Universe& univ, afl::io::Stream& file) const;

        /** Load Ion Storm Names.
            Loads a STORM.NM file.
            \param univ Target universe
            \param file File to read from */
        void loadIonStormNames(game::map::Universe& univ, afl::io::Stream& file) const;

        /** Load starbases.
            Loads BDATAx.DAT or the appropriate section from a RST.
            \param univ Target universe
            \param file File to read from
            \param count Number of starbases to load
            \param mode Load mode (load current and/or previous data)
            \param source Source of this information */
        void loadBases(game::map::Universe& univ, afl::io::Stream& file, int count, LoadMode mode, PlayerSet_t source) const;

        /** Load SHIPXY file.
            \param univ Target universe
            \param file File to read from
            \param bytes Number of bytes known to be available.
            \param mode Load mode (load current and/or previous data)
            \param source Source of this information
            \param reject Reject these players. This is used for allied file loading; to not wreak havoc if player accidentially mixed up his files. */
        void loadShipXY(game::map::Universe& univ, afl::io::Stream& file, afl::io::Stream::FileSize_t bytes, LoadMode mode, PlayerSet_t source, PlayerSet_t reject) const;

        /** Load Ships.
            Loads SHIPx.DAT or the appropriate section from a RST.
            \param univ Target universe
            \param file File to read from
            \param count Number of ships to load
            \param mode Load mode (load current and/or previous data)
            \param source Source of this information */
        void loadShips(game::map::Universe& univ, afl::io::Stream& file, int count, LoadMode mode, bool remapExplore, PlayerSet_t source) const;

        /** Load targets.
            Load TARGETx.DAT, TARGETx.EXT, or appropriate section from KOREx.DAT, UTILx.DAT or RST.
            \param univ Target universe
            \param file File to read from
            \param count Number of targets to read
            \param fmt Format of targets (encrypted or plaintext)
            \param source Source of this information
            \param turnNumber Turn number */
        void loadTargets(game::map::Universe& univ, afl::io::Stream& file, int count, TargetFormat fmt, PlayerSet_t source, int turnNumber) const;

        /** Add a target.
            Use when you have a ready-made target structure.
            \param univ Target universe
            \param target Target object
            \param source Source of this information
            \param turnNumber Turn number */
        void addTarget(game::map::Universe& univ, const game::v3::structures::ShipTarget& target, PlayerSet_t source, int turnNumber) const;

        /** Load Minefields from KORE-style file.
            \param univ Target universe
            \param file File to read from
            \param count Number of minefields to load
            \param player Player who owns the KORE file */
        void loadKoreMinefields(game::map::Universe& univ, afl::io::Stream& file, int count, int player, int turnNumber) const;

        /** Load ion storms from KOREx.DAT.
            \param univ Target universe
            \param file File to read from
            \param count Number of ion storms to read */
        void loadKoreIonStorms(game::map::Universe& univ, afl::io::Stream& file, int count) const;

        /** Load explosions from KOREx.DAT.
            \param univ Target universe
            \param file File to read from
            \param count Number of explosions to read */
        void loadKoreExplosions(game::map::Universe& univ, afl::io::Stream& file, int count) const;

        /** Load inbox.
            Load MDATAx.DAT, or appropriate section from RST or VPA.DB. */
        void loadInbox(game::msg::Inbox& inbox, afl::io::Stream& file, int turn) const;

        /** Load battles.
            \param turn Target turn
            \param file File to read from */
        void loadBattles(game::Turn& turn, afl::io::Stream& file, const game::config::HostConfiguration& config) const;

        /** Load FLAK battles.
            \param turn Target turn
            \param gameDir Game directory
            \param playerNr Player number */
        void loadFlakBattles(game::Turn& turn, afl::io::Directory& gameDir, int playerNr);

        /** Load Ufos.
            \param univ Target universe
            \param file File to read from
            \param firstId Id of first Ufo in file
            \param count Number of Ufos to load */
        void loadUfos(game::map::Universe& univ, afl::io::Stream& file, int firstId, int count) const;

        /** Load PConfig.
            \param root Root
            \param pconfig  "pconfig.src"
            \param shiplist "shiplist.txt" (can be null)
            \param source   Source flag to set */
        void loadPConfig(Root& root, afl::io::Stream& pconfig, afl::base::Ptr<afl::io::Stream> shiplist, game::config::ConfigurationOption::Source source);

        /** Load HConfig.
            \param root Root
            \param hconfig "hconfig.hst"
            \param source Source flag to set */
        void loadHConfig(Root& root, afl::io::Stream& hconfig, game::config::ConfigurationOption::Source source);

        /** Load SRace race mapping.
            \param root Root
            \param file "friday.dat"
            \param source Source flag to set */
        void loadRaceMapping(Root& root, afl::io::Stream& file, game::config::ConfigurationOption::Source source);

        /*
         *  Combined Operations
         */

        /** Load common files.
            - xyplan
            - planet.nm
            - storm.nm
            \param gameDir Game directory
            \param specDir Specification directory (union of gameDir and share/specs)
            \param univ Target universe
            \param player Player number */
        void loadCommonFiles(afl::io::Directory& gameDir, afl::io::Directory& specDir, game::map::Universe& univ, int player) const;

        /** Load result file.
            \param turn Target turn
            \param root Associated root
            \param game Target game (receive scores)
            \param file File to read from
            \param player Player */
        void loadResult(Turn& turn, const Root& root, Game& game, afl::io::Stream& file, int player) const;

        /** Load configuration.
            - pconfig.src
            - shiplist.txt (config part)
            - friday.dat
            - hconfig.hst
            - add-on configuration (FLAK, ...)
            \param root Root
            \param dir Directory to read from */
        void loadConfiguration(Root& root, afl::io::Directory& dir);
        
     private:
        afl::charset::Charset& m_charset;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
    };

} }

#endif
