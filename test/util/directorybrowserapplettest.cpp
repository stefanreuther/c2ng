/**
  *  \file test/util/directorybrowserapplettest.cpp
  *  \brief Test for util::DirectoryBrowserApplet
  */

#include "util/directorybrowserapplet.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/testrunner.hpp"
#include "util/io.hpp"

using afl::io::ConstMemoryStream;
using afl::io::FileSystem;
using afl::io::InternalFileSystem;
using afl::io::InternalStream;
using afl::sys::Environment;
using afl::sys::InternalEnvironment;

namespace {
    String_t runSequence(FileSystem& fs, const char* seq)
    {
        InternalEnvironment env;
        afl::base::Ptr<InternalStream> out = new InternalStream();
        env.setChannelStream(Environment::Input, new ConstMemoryStream(afl::string::toBytes(seq)));
        env.setChannelStream(Environment::Output, out);
        env.setChannelStream(Environment::Error, out);

        afl::data::StringList_t args;
        args.push_back("app");
        env.setCommandLine(args);

        util::Applet::Runner("", env, fs)
            .addNew("app", "", new util::DirectoryBrowserApplet())
            .run();

        return util::normalizeLinefeeds(out->getContent());
    }
}

AFL_TEST("util.DirectoryBrowserApplet:open-ls-pwd", a)
{
    InternalFileSystem fs;
    fs.createDirectory("/dir");
    fs.createDirectory("/dir/sub");
    fs.openFile("/dir/file.txt", FileSystem::Create);

    String_t out = runSequence(fs,
                               "open /dir\n"
                               "ls\n"
                               "pwd\n");
    a.checkEqual("output", out,
                 "Root> "               // First prompt
                 "dir> "                // Second prompt
                 "  0. sub <DIR>\n"     // ls output
                 "dir> "                // Third prompt
                 "  0. Root\n"          // pwd output
                 "  1. dir\n"
                 "dir> ");
}

AFL_TEST("util.DirectoryBrowserApplet:add-open-ls", a)
{
    InternalFileSystem fs;
    fs.createDirectory("/dir");
    fs.createDirectory("/dir/sub");
    fs.openFile("/dir/file.txt", FileSystem::Create);

    String_t out = runSequence(fs,
                               "add *.txt\n"
                               "open /dir\n"
                               "ls\n");
    a.checkEqual("output", out,
                 "Root> "               // First prompt
                 "Root> "               // Second prompt
                 "dir> "                // Second prompt
                 "  0. sub <DIR>\n"     // ls output
                 "  0. file.txt <FILE>\n"
                 "dir> ");
}

AFL_TEST("util.DirectoryBrowserApplet:root-ls", a)
{
    InternalFileSystem fs;
    String_t out = runSequence(fs,
                               "root\n"
                               "load\n"
                               "ls\n");
    a.checkEqual("output", out,
                 "Root> "               // First prompt
                 "Root> "               // Second prompt
                 "Root> "               // Third prompt
                 "  0. Root Directory <DIR>\n"     // ls output
                 "Root> ");
}
