/**
  *  \file game/maint/dump/parsers.hpp
  *  \brief Class game::maint::dump::Parsers
  */
#ifndef C2NG_GAME_MAINT_DUMP_PARSERS_HPP
#define C2NG_GAME_MAINT_DUMP_PARSERS_HPP

namespace game { namespace maint { namespace dump {

    class Input;
    class Output;

    /** File parsers.
        Each parser function takes
        - an input (loaded file)
        - an output (output formatter)

        Each parser function shall decode as much as it can from its input.

        @change In PCC2, input is a Stream, and it's the job of the function to read the stream.
        Likewise, it's the job of the function to dump potential trailing garbage.
        In c2ng, those steps are done by the caller. */
    struct Parsers {
        static void parseShipFile(Input& in, Output& out);
        static void parsePDataFile(Input& in, Output& out);
        static void parseBDataFile(Input& in, Output& out);
        static void parseGenFile(Input& in, Output& out);
        static void parseVcrFile(Input& in, Output& out);
        static void parseTargetFile(Input& in, Output& out);

        static void parseHullSpec(Input& in, Output& out);
        static void parseTorpSpec(Input& in, Output& out);
        static void parseBeamSpec(Input& in, Output& out);
        static void parseEngSpec(Input& in, Output& out);
        static void parseTrueHull(Input& in, Output& out);
        static void parseNameList(Input& in, Output& out);
        static void parseXYPlan(Input& in, Output& out);
        static void parseRaceNames(Input& in, Output& out);

        static void parseTeams(Input& in, Output& out);
    };

} } }

#endif
