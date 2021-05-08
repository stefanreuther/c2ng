/**
  *  \file game/test/simpleenvironment.cpp
  *  \brief Class game::test::SimpleEnvironment
  */

#include <stdexcept>
#include "game/test/simpleenvironment.hpp"
#include "afl/base/nullenumerator.hpp"

afl::base::Ref<afl::sys::Environment::CommandLine_t>
game::test::SimpleEnvironment::getCommandLine()
{
    return *new afl::base::NullEnumerator<String_t>();
}

String_t
game::test::SimpleEnvironment::getInvocationName()
{
    return "<getInvocationName>";
}

String_t
game::test::SimpleEnvironment::getEnvironmentVariable(const String_t& /*name*/)
{
    return String_t();
}

String_t
game::test::SimpleEnvironment::getSettingsDirectoryName(const String_t& /*appName*/)
{
    return "/";
}

String_t
game::test::SimpleEnvironment::getInstallationDirectoryName()
{
    return "/";
}

afl::string::LanguageCode
game::test::SimpleEnvironment::getUserLanguage()
{
    return afl::string::LanguageCode("en");
}

afl::base::Ref<afl::io::TextWriter>
game::test::SimpleEnvironment::attachTextWriter(Channel /*ch*/)
{
    throw std::runtime_error("not implemented: attachTextWriter");
}

afl::base::Ref<afl::io::TextReader>
game::test::SimpleEnvironment::attachTextReader(Channel /*ch*/)
{
    throw std::runtime_error("not implemented: attachTextReader");
}

afl::base::Ref<afl::io::Stream>
game::test::SimpleEnvironment::attachStream(Channel /*ch*/)
{
    throw std::runtime_error("not implemented: attachStream");
}
