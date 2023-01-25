/**
  *  \file client/applicationparameters.hpp
  *  \brief Class client::ApplicationParameters
  */
#ifndef C2NG_CLIENT_APPLICATIONPARAMETERS_HPP
#define C2NG_CLIENT_APPLICATIONPARAMETERS_HPP

#include <vector>
#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/dialog.hpp"
#include "afl/sys/environment.hpp"
#include "gfx/windowparameters.hpp"
#include "gfx/application.hpp"

namespace client {

    /** PCC2 Client Application parameter parser.

        General mode:
        - if no directory is given, the root browser is opened; check getGameDirectory();
        - if a directory is given, mode is decided by getDirectoryMode(). */
    class ApplicationParameters {
     public:
        /** Directory mode.
            This mode is only relevant if a directory is given. */
        enum DirectoryMode {
            /** Open game (getPlayerNumber() or default). */
            OpenGame,

            /** Open browser ("-dir" option). */
            OpenBrowser
        };

        /** Constructor.
            @param app           Application
            @param programTitle  Program title */
        ApplicationParameters(gfx::Application& app, const String_t& programTitle);

        /** Parse command line.
            @param cmdl    Command line */
        void parse(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl);

        /** Get directory mode.
            @return mode */
        DirectoryMode getDirectoryMode() const;

        /** Get game directory.
            @return game directory if set */
        const afl::base::Optional<String_t>& getGameDirectory() const;

        /** Get requested command line resources.
            @return list of command line resources */
        const std::vector<String_t>& getCommandLineResources() const;

        /** Get window parameters.
            @return window parameters */
        const gfx::WindowParameters& getWindowParameters() const;

        /** Get proxy address.
            @return proxy address if set */
        const afl::base::Optional<String_t>& getProxyAddress() const;

        /** Get RST password.
            @return RST password if set */
        const afl::base::Optional<String_t>& getPassword() const;

        /** Get trace configuration.
            @return trace configuration; empty if none set */
        const String_t& getTraceConfiguration() const;

        /** Get request delay (--debug-request-delay).
            @return request delay; 0 if none set */
        int getRequestThreadDelay() const;

        /** Get player number.
            @return player number; 0 if none given */
        int getPlayerNumber() const;

     private:
        gfx::Application& m_app;
        String_t m_programTitle;
        gfx::WindowParameters m_windowParameters;
        String_t m_traceConfig;
        afl::base::Optional<String_t> m_gameDirectory;
        afl::base::Optional<String_t> m_proxyAddress;
        afl::base::Optional<String_t> m_password;
        std::vector<String_t> m_commandLineResources;
        int m_requestThreadDelay;
        int m_playerNumber;
        DirectoryMode m_directoryMode;

        void doHelp();
    };

}

inline client::ApplicationParameters::DirectoryMode
client::ApplicationParameters::getDirectoryMode() const
{
    return m_directoryMode;
}

inline const afl::base::Optional<String_t>&
client::ApplicationParameters::getGameDirectory() const
{
    return m_gameDirectory;
}

inline const std::vector<String_t>&
client::ApplicationParameters::getCommandLineResources() const
{
    return m_commandLineResources;
}

inline const gfx::WindowParameters&
client::ApplicationParameters::getWindowParameters() const
{
    return m_windowParameters;
}

inline const afl::base::Optional<String_t>&
client::ApplicationParameters::getProxyAddress() const
{
    return m_proxyAddress;
}

inline const afl::base::Optional<String_t>&
client::ApplicationParameters::getPassword() const
{
    return m_password;
}

inline const String_t&
client::ApplicationParameters::getTraceConfiguration() const
{
    return m_traceConfig;
}

inline int
client::ApplicationParameters::getRequestThreadDelay() const
{
    return m_requestThreadDelay;
}

inline int
client::ApplicationParameters::getPlayerNumber() const
{
    return m_playerNumber;
}

#endif
