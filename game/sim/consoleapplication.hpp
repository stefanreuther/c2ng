/**
  *  \file game/sim/consoleapplication.hpp
  *  \brief Class game::sim::ConsoleApplication
  */
#ifndef C2NG_GAME_SIM_CONSOLEAPPLICATION_HPP
#define C2NG_GAME_SIM_CONSOLEAPPLICATION_HPP

#include "afl/charset/charset.hpp"
#include "util/application.hpp"

namespace game { namespace sim {

    class Setup;
    class ResultList;

    /** Simulator console application.
        Provides a command-line interface to the battle simulator.
        In particular, it replaces the "mergeccb" utility. */
    class ConsoleApplication : public util::Application {
     public:
        /** Constructor.
            \param env Environment
            \param fs  File system */
        ConsoleApplication(afl::sys::Environment& env, afl::io::FileSystem& fs);

        // Application:
        void appMain();

     private:
        bool m_verbose;

        struct Parameters;
        struct Session;

        void parseCommandLine(Parameters& p);
        void help();

        void loadSetup(Setup& setup, afl::charset::Charset& charset, const std::vector<String_t>& loadFileNames);
        void saveSetup(const Setup& setup, afl::charset::Charset& charset, const String_t& saveFileName);
        void loadSession(Session& session, const Parameters& params, afl::charset::Charset& charset);
        void verifySetup(const Setup& setup, const Session& session);
        void showSetup(const Setup& setup, const Session& session);
        void runSimulation(const Setup& setup, const Session& session, const Parameters& params);
        void showClassResults(const Setup& setup, const Session& session, const ResultList& resultList);
        void showUnitResults(const Setup& setup, const Session& session, const ResultList& resultList);
    };

} }

#endif
