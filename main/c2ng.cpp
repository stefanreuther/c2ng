/**
  *  \file main/c2ng.cpp
  *  \brief c2ng - Graphical Client - main function
  */

#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/string/proxytranslator.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/dialog.hpp"
#include "afl/sys/environment.hpp"
#include "afl/test/translator.hpp"
#include "client/application.hpp"

int main(int, char** argv)
{
    // Capture environment
    afl::sys::Dialog& dialog = afl::sys::Dialog::getSystemInstance();
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();

    // Infrastructure (FIXME).
#if 0
    afl::test::Translator tx("\xC2\xAB", "\xC2\xBB");
#else
    afl::string::NullTranslator tx;
#endif
    afl::string::Translator::setSystemInstance(std::auto_ptr<afl::string::Translator>(new afl::string::ProxyTranslator(tx)));

    // Application
    return client::Application(dialog, tx, env, fs, net).run();
}
