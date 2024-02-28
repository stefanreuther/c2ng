/**
  *  \file test/util/helpindextest.cpp
  *  \brief Test for util::HelpIndex
  */

#include "util/helpindex.hpp"

#include "afl/io/directory.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"

/** Test scenarios. */
AFL_TEST("util.HelpIndex:basics", a)
{
    afl::io::InternalFileSystem fs;
    afl::sys::Log log;
    afl::string::NullTranslator tx;

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
        t.find("qq", out, fs, log, tx);
        a.checkEqual("01. size", out.size(), 0U);
    }

    // Verify toc
    {
        util::HelpIndex::NodeVector_t out;
        t.find("toc", out, fs, log, tx);
        a.checkEqual  ("11. size", out.size(), 2U);
        a.checkNonNull("12. node", out[0]);
        a.checkEqual  ("13. name", out[0]->file.name, "__tmp2");
        a.checkNonNull("14. node", out[1]);
        a.checkEqual  ("15. name", out[1]->file.name, "__tmp1");
    }

    // Verify f2
    {
        util::HelpIndex::NodeVector_t out;
        t.find("f2", out, fs, log, tx);
        a.checkEqual  ("21. size", out.size(), 1U);
        a.checkNonNull("22. node", out[0]);
        a.checkEqual  ("23. name", out[0]->file.name, "__tmp2");
    }

    // Add another file and verify again
    t.addFile("__tmp3", "o3");

    // Verify toc
    {
        util::HelpIndex::NodeVector_t out;
        t.find("toc", out, fs, log, tx);
        a.checkEqual  ("31. size", out.size(), 3U);
        a.checkNonNull("32. node", out[0]);
        a.checkEqual  ("33. name", out[0]->file.name, "__tmp3");
        a.checkNonNull("34. node", out[1]);
        a.checkEqual  ("35. name", out[1]->file.name, "__tmp2");
        a.checkNonNull("36. node", out[2]);
        a.checkEqual  ("37. name", out[2]->file.name, "__tmp1");
    }

    // Verify f2
    {
        util::HelpIndex::NodeVector_t out;
        t.find("f2", out, fs, log, tx);
        a.checkEqual  ("41. size", out.size(), 1U);
        a.checkNonNull("42. node", out[0]);
        a.checkEqual  ("43. name", out[0]->file.name, "__tmp3");
    }

    // Remove o2
    t.removeFilesByOrigin("o2");

    // Verify toc
    {
        util::HelpIndex::NodeVector_t out;
        t.find("toc", out, fs, log, tx);
        a.checkEqual  ("51. size", out.size(), 2U);
        a.checkNonNull("52. node", out[0]);
        a.checkEqual  ("53. name", out[0]->file.name, "__tmp3");
        a.checkNonNull("54. node", out[1]);
        a.checkEqual  ("55. name", out[1]->file.name, "__tmp1");
    }

    // Remove files
    dir->erase("__tmp1");
    dir->erase("__tmp2");
    dir->erase("__tmp3");
}

/** Test adding a missing file. Must not throw. */
AFL_TEST("util.HelpIndex:addFile:missing", a)
{
    afl::io::NullFileSystem fs;
    afl::sys::Log log;
    afl::string::NullTranslator tx;

    util::HelpIndex t;
    AFL_CHECK_SUCCEEDS(a("01. addFile"), t.addFile("__q2poiwknskdflahuw0e298x", "o1"));

    util::HelpIndex::NodeVector_t out;
    AFL_CHECK_SUCCEEDS(a("11. find"), t.find("p", out, fs, log, tx));
    a.check("12. empty", out.empty());
}
