/**
  *  \file main/c2reshack.cpp
  */

#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/string/proxytranslator.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/dialog.hpp"
#include "afl/sys/environment.hpp"
#include "afl/test/translator.hpp"
#include "ui/reshack/application.hpp"
#include "util/translator.hpp"

int main(int, char** argv)
{
    // Capture environment
    afl::sys::Dialog& dialog = afl::sys::Dialog::getSystemInstance();
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();

    // Infrastructure (FIXME).
    util::Translator tx;
    tx.loadDefaultTranslation(fs, env);
    afl::string::Translator::setSystemInstance(std::auto_ptr<afl::string::Translator>(new afl::string::ProxyTranslator(tx)));

    // Application
    return ui::reshack::Application(dialog, tx, env, fs).run();
}
