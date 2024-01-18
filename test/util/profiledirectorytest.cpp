/**
  *  \file test/util/profiledirectorytest.cpp
  *  \brief Test for util::ProfileDirectory
  */

#include "util/profiledirectory.hpp"

#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    struct Environment {
        afl::sys::InternalEnvironment env;
        afl::io::InternalFileSystem fs;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        Environment()
            : env(), fs(), tx(), log()
            {
                env.setSettingsDirectoryName("/home/user/*");
            }
    };
}

/** Test opening files. */
AFL_TEST("util.ProfileDirectory:open:file", a)
{
    // Environment
    Environment env;
    util::ProfileDirectory testee(env.env, env.fs, env.tx, env.log);

    // Cannot open file
    a.checkNull("01", testee.openFileNT("pcc2.ini").get());

    // Create the file
    testee.createFile("pcc2.ini")->fullWrite(afl::string::toBytes("content"));

    // File can now be opened
    a.checkEqual("11", testee.openFileNT("pcc2.ini")->getSize(), 7U);
    a.checkEqual("12", env.fs.openFile("/home/user/PCC2/pcc2.ini", afl::io::FileSystem::OpenRead)->getSize(), 7U);
}

/** Test opening through open(). */
AFL_TEST("util.ProfileDirectory:open:dir", a)
{
    // Environment
    Environment env;
    util::ProfileDirectory testee(env.env, env.fs, env.tx, env.log);

    // Open directory and create the file
    testee.open()->openFile("pcc2.ini", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("content"));

    // File can now be opened
    a.checkEqual("01", testee.openFileNT("pcc2.ini")->getSize(), 7U);
    a.checkEqual("02", env.fs.openFile("/home/user/PCC2/pcc2.ini", afl::io::FileSystem::OpenRead)->getSize(), 7U);
}
