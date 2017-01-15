/**
  *  \file game/v3/loader.hpp
  *  \brief Class game::v3::Loader
  */
#ifndef C2NG_GAME_V3_LOADER_HPP
#define C2NG_GAME_V3_LOADER_HPP

#include "afl/charset/charset.hpp"
#include "afl/string/translator.hpp"
#include "game/map/universe.hpp"
#include "afl/io/stream.hpp"
#include "game/playerset.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/msg/inbox.hpp"
#include "game/turn.hpp"

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
        void prepareUniverse(game::map::Universe& univ);

        /** Load planets.
            Loads PDATAx.DAT or the appropriate section from a result.
            \param univ Target universe
            \param file File to read from
            \param count Number of planets to load
            \param mode Load mode (load current and/or previous data)
            \param source Source of this information */
        void loadPlanets(game::map::Universe& univ, afl::io::Stream& file, int count, LoadMode mode, PlayerSet_t source);

        /** Load planet coordinates.
            Loads an XYPLAN.DAT file.
            \param univ Target universe
            \param file File to read from */
        void loadPlanetCoordinates(game::map::Universe& univ, afl::io::Stream& file);

        /** Load Planet Names.
            Loads a PLANET.NM file.
            \param univ Target universe
            \param file File to read from */
        void loadPlanetNames(game::map::Universe& univ, afl::io::Stream& file);

        /** Load Ion Storm Names.
            Loads a STORM.NM file.
            \param univ Target universe
            \param file File to read from */
        void loadIonStormNames(game::map::Universe& univ, afl::io::Stream& file);

        /** Load starbases.
            Loads BDATAx.DAT or the appropriate section from a RST.
            \param univ Target universe
            \param file File to read from
            \param count Number of starbases to load
            \param mode Load mode (load current and/or previous data)
            \param source Source of this information */
        void loadBases(game::map::Universe& univ, afl::io::Stream& file, int count, LoadMode mode, PlayerSet_t source);

        /** Load SHIPXY file.
            \param univ Target universe
            \param file File to read from
            \param bytes Number of bytes known to be available.
            \param mode Load mode (load current and/or previous data)
            \param source Source of this information
            \param reject Reject these players. This is used for allied file loading; to not wreak havoc if player accidentially mixed up his files. */
        void loadShipXY(game::map::Universe& univ, afl::io::Stream& file, afl::io::Stream::FileSize_t bytes, LoadMode mode, PlayerSet_t source, PlayerSet_t reject);

        /** Load Ships.
            Loads SHIPx.DAT or the appropriate section from a RST.
            \param univ Target universe
            \param file File to read from
            \param count Number of ships to load
            \param mode Load mode (load current and/or previous data)
            \param source Source of this information */
        void loadShips(game::map::Universe& univ, afl::io::Stream& file, int count, LoadMode mode, bool remapExplore, PlayerSet_t source);

        /** Load targets.
            Load TARGETx.DAT, TARGETx.EXT, or appropriate section from KOREx.DAT, UTILx.DAT or RST.
            \param univ Target universe
            \param file File to read from
            \param count Number of targets to read
            \param fmt Format of targets (encrypted or plaintext)
            \param source Source of this information */
        void loadTargets(game::map::Universe& univ, afl::io::Stream& file, int count, TargetFormat fmt, PlayerSet_t source, int turnNumber);

        /** Load Minefields from KORE-style file.
            \param univ Target universe
            \param file File to read from
            \param count Number of minefields to load
            \param player Player who owns the KORE file */
        void loadKoreMinefields(game::map::Universe& univ, afl::io::Stream& file, int count, int player, int turnNumber);

        /** Load ion storms from KOREx.DAT.
            \param univ Target universe
            \param file File to read from
            \param count Number of ion storms to read */
        void loadKoreIonStorms(game::map::Universe& univ, afl::io::Stream& file, int count);

        /** Load explosions from KOREx.DAT.
            \param univ Target universe
            \param file File to read from
            \param count Number of explosions to read */
        void loadKoreExplosions(game::map::Universe& univ, afl::io::Stream& file, int count);

        /** Load inbox.
            Load MDATAx.DAT, or appropriate section from RST or VPA.DB. */
        void loadInbox(game::msg::Inbox& inbox, afl::io::Stream& file, int turn);

        /** Load battles.
            \param turn Target turn
            \param file File to read from */
        void loadBattles(game::Turn& turn, afl::io::Stream& file, const game::config::HostConfiguration& config);

        void loadUfos(game::map::Universe& univ, afl::io::Stream& file, int firstId, int count);

     private:
        afl::charset::Charset& m_charset;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
    };

} }

#endif
