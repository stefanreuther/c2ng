/**
  *  \file game/test/simpleenvironment.hpp
  *  \brief Class game::test::SimpleEnvironment
  */
#ifndef C2NG_GAME_TEST_SIMPLEENVIRONMENT_HPP
#define C2NG_GAME_TEST_SIMPLEENVIRONMENT_HPP

#include "afl/sys/environment.hpp"

namespace game { namespace test {

    /** Simple environment.
        Reports default/null values for everything and does not allow attaching a stream. */
    class SimpleEnvironment : public afl::sys::Environment {
     public:
        virtual afl::base::Ref<CommandLine_t> getCommandLine();
        virtual String_t getInvocationName();
        virtual String_t getEnvironmentVariable(const String_t& name);
        virtual String_t getSettingsDirectoryName(const String_t& appName);
        virtual String_t getInstallationDirectoryName();
        virtual afl::string::LanguageCode getUserLanguage();
        virtual afl::base::Ref<afl::io::TextWriter> attachTextWriter(Channel ch);
        virtual afl::base::Ref<afl::io::TextReader> attachTextReader(Channel ch);
        virtual afl::base::Ref<afl::io::Stream> attachStream(Channel ch);
    };

} }

#endif
