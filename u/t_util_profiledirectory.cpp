/**
  *  \file u/t_util_profiledirectory.cpp
  *  \brief Test for util::ProfileDirectory
  */

#include "util/profiledirectory.hpp"

#include "t_util.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"

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
void
TestUtilProfileDirectory::testOpen()
{
    // Environment
    Environment env;
    util::ProfileDirectory testee(env.env, env.fs, env.tx, env.log);

    // Cannot open file
    TS_ASSERT(testee.openFileNT("pcc2.ini").get() == 0);

    // Create the file
    testee.createFile("pcc2.ini")->fullWrite(afl::string::toBytes("content"));

    // File can now be opened
    TS_ASSERT_EQUALS(testee.openFileNT("pcc2.ini")->getSize(), 7U);
    TS_ASSERT_EQUALS(env.fs.openFile("/home/user/PCC2/pcc2.ini", afl::io::FileSystem::OpenRead)->getSize(), 7U);
}

/** Test opening through open(). */
void
TestUtilProfileDirectory::testOpenDir()
{
    // Environment
    Environment env;
    util::ProfileDirectory testee(env.env, env.fs, env.tx, env.log);

    // Open directory and create the file
    testee.open()->openFile("pcc2.ini", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("content"));

    // File can now be opened
    TS_ASSERT_EQUALS(testee.openFileNT("pcc2.ini")->getSize(), 7U);
    TS_ASSERT_EQUALS(env.fs.openFile("/home/user/PCC2/pcc2.ini", afl::io::FileSystem::OpenRead)->getSize(), 7U);
}

