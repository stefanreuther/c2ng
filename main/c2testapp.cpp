/**
  *  \file main/c2testapp.cpp
  *  \brief c2testapp - Test applets
  */

#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/environment.hpp"
#include "game/browser/testapplet.hpp"
#include "game/parser/testapplet.hpp"
#include "game/v3/scannerapplet.hpp"
#include "game/vcr/classic/testapplet.hpp"
#include "game/vcr/flak/testapplet.hpp"
#include "util/applet.hpp"
#include "util/directorybrowserapplet.hpp"
#include "util/processrunnerapplet.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();

    return util::Applet::Runner("PCC2 Test Applets", env, fs)
        .addNew("browser",    "Game browser test",       new game::browser::TestApplet(net))
        .addNew("dirbrowser", "Directory browser test",  new util::DirectoryBrowserApplet())
        .addNew("msgparse",   "Message parser test",     new game::parser::TestApplet())
        .addNew("overview",   "Directory overview test", new game::v3::ScannerApplet())
        .addNew("process",    "Process runner test",     new util::ProcessRunnerApplet())
        .addNew("testflak",   "FLAK test",               new game::vcr::flak::TestApplet())
        .addNew("testvcr",    "Classic VCR test",        new game::vcr::classic::TestApplet())
        .run();
}
