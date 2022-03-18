/**
  *  \file game/v3/check/checker.hpp
  */
#ifndef C2NG_GAME_V3_CHECK_CHECKER_HPP
#define C2NG_GAME_V3_CHECK_CHECKER_HPP

#include "game/v3/structures.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/base/memory.hpp"
#include "game/v3/check/configuration.hpp"
#include "game/v3/genfile.hpp"

namespace game { namespace v3 { namespace check {

    // Very rough port based on
    //   'Check v0.5 - A VGA Planets Turn Checker - (c) 2005-2012 Stefan Reuther'
    // FIXME: this needs a lot of love:
    // - split into loader (RST, TRN) and core
    // - factor out output generation
    // - convert to dynamic array allocation, get rid of the pascal-style for (1; <= N) loops
    // - reclassify messages to make is usable as host turn checker

    // FIXME: new checks
    // - validate positions of transport target ships
    // - validate positions of shipyard target ships
    // - validate parameters of intercept/tow mission
    // - use config (StarbaseCost, BaseTechCost, MaximumFightersOnBase, MaximumDefenseOnBase)
    // - check object count < 0 [done-ng]
    // - fail on SYNTAX error in gen.dat [done-ng]
    // - report invalid timestamp in turn [done-ng]
    // - validate base owner before accessing truehull [done-ng]
    // - check overloaded ships

    class Checker {
     public:
        typedef game::v3::structures::Ship Ship_t;
        typedef game::v3::structures::Planet Planet_t;
        typedef game::v3::structures::Base Base_t;

        static const int NUM_SHIPS = game::v3::structures::NUM_SHIPS;
        static const int NUM_PLANETS = game::v3::structures::NUM_PLANETS;
        static const int NUM_TORPEDO_TYPES = game::v3::structures::NUM_TORPEDO_TYPES;
        static const int NUM_BEAM_TYPES = game::v3::structures::NUM_BEAM_TYPES;
        static const int NUM_ENGINE_TYPES = game::v3::structures::NUM_ENGINE_TYPES;
        static const int NUM_HULL_TYPES = 105;

        static const size_t SIGNATURE_SIZE = 10;

        Checker(afl::io::Directory& gamedir,
                afl::io::Directory& rootdir,
                int player,
                afl::io::TextWriter& log,
                afl::io::TextWriter& output,
                afl::io::TextWriter& error);

        ~Checker();

        // Main
        void run();

        Configuration& config()
            { return m_config; }
        const Configuration& config() const
            { return m_config; }

        bool hadAnyError() const
            { return had_any_error; }
        bool hadChecksumError() const
            { return had_ck_error; }

     private:
        struct PlanetEntry {
            // ex check.pas:CPlanet
            int x, y;
            Planet_t *pdat, *pdis;
            Base_t *bdat, *bdis;
        };
        struct ShipEntry {
            // ex check.pas:CShip
            bool seen;         // whether the check already saw this ship, NOT whether we know it!
            Ship_t *dat, *dis;
        };

        afl::io::Directory& gamedir;
        afl::io::Directory& rootdir;
        int m_player;
        afl::io::TextWriter& log;
        afl::io::TextWriter& output;
        afl::io::TextWriter& error;
        Configuration m_config;

        struct DirStuff {
            game::v3::GenFile gen;
        };

        bool had_ck_error;

        // Log File Output
        void logAbort(const String_t& s);
        void die(const String_t& s);
        void syntax(const String_t& s);

        bool had_divi;
        enum { hRaw, hStartSec, hInSec } html_fmt;

        void startSec(const String_t& color);
        static String_t escape(const String_t& s);
        void logStr(const String_t& s);
        void logStrB(const String_t& s);
        void logDivi();
        void logItem(const String_t& pre, const String_t& suf);
        void logBlock(const String_t& s);
        void logCheck(const String_t& title);

        // Checksums
        static uint32_t checksum(afl::base::ConstBytes_t bytes);
        static bool memEq(afl::base::ConstBytes_t a, afl::base::ConstBytes_t b);
        void checkChecksum(const String_t& title, uint32_t soll, uint32_t ist);
        void checkSigs(const String_t& name, afl::io::Stream& dat, afl::io::Stream& dis, const DirStuff& stuff);
        bool checkTimestamp(uint8_t (&ts)[18]);

        // Data
        ShipEntry ships[NUM_SHIPS];
        PlanetEntry planets[NUM_SHIPS];
        game::v3::structures::Torpedo torps[NUM_TORPEDO_TYPES];
        game::v3::structures::Beam beams[NUM_BEAM_TYPES];
        game::v3::structures::Hull hulls[NUM_HULL_TYPES];
        game::v3::structures::Engine engines[NUM_ENGINE_TYPES];
        game::v3::structures::Int16_t truehull[11][20];

        // Loading things
        afl::base::Ref<afl::io::Stream> openGameFile(const String_t& name) const;
        afl::base::Ref<afl::io::Stream> openSpecFile(const String_t& name) const;

        void loadXYPlan();
        int planetAt(int x, int y) const;
        void loadGen(DirStuff& stuff);
        void loadShips(const DirStuff& stuff);
        void loadPlanets(const DirStuff& stuff);
        void loadChecksums(const DirStuff& stuff);
        void loadResult(uint8_t (&rst_timestamp)[18]);
        void loadTurn(const uint8_t (&rst_timestamp)[18]);
        void loadSpecs();
        bool isActive(int pl) const;

        // Checks
        bool had_any_error;
        bool had_error;
        String_t ctx;

        void rangeCheckSingleValue(const String_t& s, int32_t val, int32_t min, int32_t max);
        void rangeCheckCost(const game::v3::structures::Cost& cost);
        void rangeCheckSpecs();
        void checkEditable(const String_t& s, int32_t dat, int32_t dis, int32_t min, int32_t max);
        void checkInvariant(const String_t& s, int32_t dat, int32_t dis, int32_t min, int32_t max);
        void checkTransfer(const String_t& name, const game::v3::structures::ShipTransfer& dat, const game::v3::structures::ShipTransfer& dis);
        void checkTransferTarget(const String_t& name, const game::v3::structures::ShipTransfer& dat, int i);
        void rangeCheckShips();
        void checkComponent(const String_t& what, int want, int have, int max);
        void rangeCheckPlanets();

        // Flow checks
        struct ResourceSummary {
            int32_t n, t, d, m;
            int32_t mc, sup;
            int32_t clans;
            int32_t torps[NUM_TORPEDO_TYPES];
            int32_t fighters;
        };
        void addTransfer(ResourceSummary& rs, const game::v3::structures::ShipTransfer& t);
        void addShip(ResourceSummary& rs, const Ship_t& s);
        void addPlanet(ResourceSummary& rs, const Planet_t& p);
        void addBase(ResourceSummary& rs, const Base_t& b);
        void tryBuy(ResourceSummary& corr, const String_t& what, int cur, int ori, int t, int d, int m, int mc, int sup, int need_tech, int have_tech);
        void tryBuyTech(ResourceSummary& corr, const String_t& what, int cur, int ori);
        void checkBalance(bool& ok, const String_t& what, int32_t cur, int32_t old, int32_t corr);
        void validateStructures(const String_t& what, int cur, int old, int32_t col, int cutoff);
        void flowCheckOrbits();
        void checkBalanceSpace(bool& ok, const String_t& what, int32_t cur, int32_t old);
        void flowCheckFreeSpace();
    };

} } }

#endif
