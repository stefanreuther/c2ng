/**
  *  \file test/util/plugin/dialogapplicationtest.cpp
  *  \brief Test for util::plugin::DialogApplication
  */

#include "util/plugin/dialogapplication.hpp"

#include "afl/io/internalfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/dialog.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    class MockDialog : public afl::sys::Dialog, public afl::test::CallReceiver {
     public:
        MockDialog(afl::test::Assert a)
            : Dialog(), CallReceiver(a)
            { }
        virtual void showInfo(String_t info, String_t title)
            { checkCall(afl::string::Format("showInfo(%s,%s)", info, title)); }
        virtual void showError(String_t info, String_t title)
            { checkCall(afl::string::Format("showError(%s,%s)", info, title)); }
        virtual bool askYesNo(String_t info, String_t title)
            {
                checkCall(afl::string::Format("askYesNo(%s,%s)", info, title));
                return consumeReturnValue<bool>();
            }
    };
}

AFL_TEST("util.plugin.DialogApplication", a)
{
    // Environment
    // - file system
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/home");
    fs.createDirectory("/home/PCC2");
    fs.openFile("/q.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("name = cute plugin\n"));

    // - environment
    afl::sys::InternalEnvironment env;
    afl::data::StringList_t args;
    args.push_back("/q.c2p");
    env.setCommandLine(args);
    env.setSettingsDirectoryName("/home/*");

    // - dialog
    MockDialog dlg("testIt");
    dlg.expectCall("askYesNo(Do you want to install plugin \"cute plugin\" (Q)?),PCC2 Plugin Installer)");
    dlg.provideReturnValue(true);
    dlg.expectCall("showInfo(Plugin 'cute plugin' has been installed.,PCC2 Plugin Installer)");

    // Test it
    util::plugin::DialogApplication testee(env, fs, dlg);
    int result = testee.run();
    a.checkEqual("result", result, 0);
}
