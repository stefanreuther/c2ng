/**
  *  \file u/t_util_helpindex.cpp
  *  \brief Test for util::HelpIndex
  */

#include "util/helpindex.hpp"

#include "t_util.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/sys/log.hpp"

/** Test scenarios. */
void
TestUtilHelpIndex::testMulti()
{
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::sys::Log log;

    // Create files
    const char FILE1[] = "<help><page id=\"toc\"></page><page id=\"f1\"></page></help>";
    const char FILE2[] = "<help priority=\"99\"><page id=\"toc\"></page><page id=\"f2\"></page></help>";
    const char FILE3[] = "<help priority=\"99\"><page id=\"toc\"></page><page id=\"f2\"></page></help>";
    afl::base::Ref<afl::io::Directory> dir = fs.openDirectory(fs.getWorkingDirectoryName());
    dir->openFile("__tmp1", fs.Create)->fullWrite(afl::string::toBytes(FILE1));
    dir->openFile("__tmp2", fs.Create)->fullWrite(afl::string::toBytes(FILE2));
    dir->openFile("__tmp3", fs.Create)->fullWrite(afl::string::toBytes(FILE3));

    // Create help index, starting with two files
    util::HelpIndex t;
    t.addFile("__tmp1", "o1");
    t.addFile("__tmp2", "o2");

    // Not found
    {
        util::HelpIndex::NodeVector_t out;
        t.find("qq", out, fs, log);
        TS_ASSERT_EQUALS(out.size(), 0U);
    }

    // Verify toc
    {
        util::HelpIndex::NodeVector_t out;
        t.find("toc", out, fs, log);
        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT(out[0] != 0);
        TS_ASSERT_EQUALS(out[0]->file.name, "__tmp2");
        TS_ASSERT(out[1] != 0);
        TS_ASSERT_EQUALS(out[1]->file.name, "__tmp1");
    }

    // Verify f2
    {
        util::HelpIndex::NodeVector_t out;
        t.find("f2", out, fs, log);
        TS_ASSERT_EQUALS(out.size(), 1U);
        TS_ASSERT(out[0] != 0);
        TS_ASSERT_EQUALS(out[0]->file.name, "__tmp2");
    }

    // Add another file and verify again
    t.addFile("__tmp3", "o3");

    // Verify toc
    {
        util::HelpIndex::NodeVector_t out;
        t.find("toc", out, fs, log);
        TS_ASSERT_EQUALS(out.size(), 3U);
        TS_ASSERT(out[0] != 0);
        TS_ASSERT_EQUALS(out[0]->file.name, "__tmp3");
        TS_ASSERT(out[1] != 0);
        TS_ASSERT_EQUALS(out[1]->file.name, "__tmp2");
        TS_ASSERT(out[2] != 0);
        TS_ASSERT_EQUALS(out[2]->file.name, "__tmp1");
    }

    // Verify f2
    {
        util::HelpIndex::NodeVector_t out;
        t.find("f2", out, fs, log);
        TS_ASSERT_EQUALS(out.size(), 1U);
        TS_ASSERT(out[0] != 0);
        TS_ASSERT_EQUALS(out[0]->file.name, "__tmp3");
    }

    // Remove o2
    t.removeFilesByOrigin("o2");

    // Verify toc
    {
        util::HelpIndex::NodeVector_t out;
        t.find("toc", out, fs, log);
        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT(out[0] != 0);
        TS_ASSERT_EQUALS(out[0]->file.name, "__tmp3");
        TS_ASSERT(out[1] != 0);
        TS_ASSERT_EQUALS(out[1]->file.name, "__tmp1");
    }

    // Remove files
    dir->erase("__tmp1");
    dir->erase("__tmp2");
    dir->erase("__tmp3");
}

/** Test adding a missing file. Must not throw. */
void
TestUtilHelpIndex::testMissing()
{
    afl::io::NullFileSystem fs;
    afl::sys::Log log;

    util::HelpIndex t;
    TS_ASSERT_THROWS_NOTHING(t.addFile("__q2poiwknskdflahuw0e298x", "o1"));

    util::HelpIndex::NodeVector_t out;
    TS_ASSERT_THROWS_NOTHING(t.find("p", out, fs, log));
    TS_ASSERT(out.empty());
}

