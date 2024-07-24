/**
  *  \file test/util/translatortest.cpp
  *  \brief Test for util::Translator
  */

#include "util/translator.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/languagecode.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/testrunner.hpp"

using afl::except::FileProblemException;
using afl::io::FileSystem;
using afl::io::InternalFileSystem;
using afl::string::LanguageCode;
using afl::sys::InternalEnvironment;

namespace {
    void checkError(afl::test::Assert a, afl::base::ConstBytes_t data)
    {
        util::Translator testee;
        afl::io::ConstMemoryStream mem(data);
        AFL_CHECK_THROWS(a, testee.loadFile(mem), FileProblemException);
    }

    // Created from:
    //   msgid "a"
    //   msgstr "xyz"
    static const uint8_t GOOD_FILE[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        // magic                                        num                     inPtr
        0x2c, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        // outPtr               inText                  inSize                  outText
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // outSize              inPtr#1                 inLen#1                 outPtr#1
        0x04, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x79, 0x7a, 0x00
        // outLen#1             inText      outText
    };
}

/* Empty translator */
AFL_TEST("util.Translator:empty", a)
{
    util::Translator testee;

    // Normal 1:1 translation
    a.checkEqual("01", testee("x"), "x");
    a.checkEqual("02", testee("x:"), "x:");
    a.checkEqual("03", testee("x{tag}y"), "x{tag}y");

    // Special handling for tags; special cases/malformed
    a.checkEqual("11", testee("{tag}y"), "y");
    a.checkEqual("12", testee("{tag}"), "");
    a.checkEqual("13", testee("{tag"), "{tag");
}

/* Populated translator */
AFL_TEST("util.Translator:normal", a)
{
    util::Translator testee;
    testee.addTranslation("a", "b");
    testee.addTranslation("{tag}x", "y");

    // Normal (non)translation
    a.checkEqual("01", testee("a"), "b");
    a.checkEqual("02", testee("x"), "x");
    a.checkEqual("03", testee("{tag}x"), "y");

    // Suffix handling
    a.checkEqual("11", testee("a: "), "b: ");
    a.checkEqual("12", testee("a\n"), "b\n");
}

/* Loading a file */
AFL_TEST("util.Translator:load:success", a)
{
    util::Translator testee;
    afl::io::ConstMemoryStream mem(GOOD_FILE);
    testee.loadFile(mem);

    // Verify
    a.checkEqual("01", testee("a"), "xyz");
    a.checkEqual("02", testee("x"), "x");
}

/* Loading a file: error cases */
AFL_TEST("util.Translator:load:error:too-short", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x34,
    };
    checkError(a, DATA);
}

AFL_TEST("util.Translator:load:error:bad-magic", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x31, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        //                                  ^^^^
        0x2c, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x79, 0x7a, 0x00
    };
    checkError(a, DATA);
}

AFL_TEST("util.Translator:load:error:bad-num", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x99, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        //                                              ^^^^
        0x2c, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x79, 0x7a, 0x00
    };
    checkError(a, DATA);
}

AFL_TEST("util.Translator:load:error:bad-inptr", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x99, 0x00, 0x00, 0x00,
        //                                                                      ^^^^
        0x2c, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x79, 0x7a, 0x00
    };
    checkError(a, DATA);
}

AFL_TEST("util.Translator:load:error:bad-outptr", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        0x99, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        // ^^^
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x79, 0x7a, 0x00
    };
    checkError(a, DATA);
}


AFL_TEST("util.Translator:load:error:bad-intext", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x99, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        //                      ^^^^
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x79, 0x7a, 0x00
    };
    checkError(a, DATA);
}

AFL_TEST("util.Translator:load:error:bad-insize", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x99, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        //                                              ^^^^
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x79, 0x7a, 0x00
    };
    checkError(a, DATA);
}

AFL_TEST("util.Translator:load:error:bad-outtext", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x99, 0x00, 0x00, 0x00,
        //                                                                      ^^^^
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x79, 0x7a, 0x00
    };
    checkError(a, DATA);
}

AFL_TEST("util.Translator:load:error:bad-outsize", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // ^^
        0x04, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x79, 0x7a, 0x00
        // outLen#1             inText      outText
    };
    checkError(a, DATA);
}

AFL_TEST("util.Translator:load:error:bad-inptr-1", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //                      ^^^^
        0x04, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x79, 0x7a, 0x00
    };
    checkError(a, DATA);
}

AFL_TEST("util.Translator:load:error:bad-inlen-1", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        //                                              ^^^^
        0x04, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x79, 0x7a, 0x00
    };
    checkError(a, DATA);
}

AFL_TEST("util.Translator:load:error:bad-outptr-1", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        //                                                                      ^^^^
        0x04, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x79, 0x7a, 0x00
    };
    checkError(a, DATA);
}

AFL_TEST("util.Translator:load:error:bad-outlen-1", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x61, 0x00, 0x78, 0x79, 0x7a, 0x00
        // ^^^
    };
    checkError(a, DATA);
}

/* Unspecified variant: no null terminator.
   Most importantly, must not crash.
   For the c2ng implementation, the null terminator is optional (in PCC2, it's mandatory). */
AFL_TEST("util.Translator:load:unspec", a)
{
    static const uint8_t DATA[] = {
        0x43, 0x43, 0x6c, 0x61, 0x6e, 0x67, 0x30, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x61, 0x62, 0x78, 0x79, 0x7a, 0x78
        //                      inText      outText
    };
    util::Translator testee;
    afl::io::ConstMemoryStream mem(DATA);
    testee.loadFile(mem);

    // Verify
    a.checkEqual("01", testee("ab"), "xyzx");
    a.checkEqual("02", testee("a"), "a");
}

/* Test loadTranslation */
AFL_TEST("util.Translator:loadTranslation", a)
{
    InternalFileSystem fs;
    fs.createDirectory("/install");
    fs.createDirectory("/install/share");
    fs.createDirectory("/install/share/resource");
    fs.openFile("/install/share/resource/tr-de.lang", FileSystem::Create)
        ->fullWrite(GOOD_FILE);

    InternalEnvironment env;
    env.setInstallationDirectoryName("/install");
    env.setUserLanguage(LanguageCode("de_DE"));

    util::Translator testee;
    testee.loadTranslation(fs, env, LanguageCode("tr_DE.UTF-8@euro"));

    a.checkEqual("01", testee("a"), "xyz");
}

/* Test loadTranslation, error case */
AFL_TEST("util.Translator:loadTranslation:error", a)
{
    InternalFileSystem fs;
    fs.createDirectory("/install");
    fs.createDirectory("/install/share");
    fs.createDirectory("/install/share/resource");
    fs.openFile("/install/share/resource/tr-de.lang", FileSystem::Create); // Empty file

    InternalEnvironment env;
    env.setInstallationDirectoryName("/install");
    env.setUserLanguage(LanguageCode("de_DE"));

    util::Translator testee;
    testee.loadTranslation(fs, env, LanguageCode("tr_DE.UTF-8@euro"));
    // Succeeds, error is not given to caller!

    a.checkEqual("01", testee("a"), "a");
}

/* Test loadDefaultTranslation */
AFL_TEST("util.Translator:loadDefaultTranslation", a)
{
    InternalFileSystem fs;
    fs.createDirectory("/install");
    fs.createDirectory("/install/share");
    fs.createDirectory("/install/share/resource");
    fs.openFile("/install/share/resource/de-de.lang", FileSystem::Create)
        ->fullWrite(GOOD_FILE);

    InternalEnvironment env;
    env.setInstallationDirectoryName("/install");
    env.setUserLanguage(LanguageCode("de_DE"));

    util::Translator testee;
    testee.loadDefaultTranslation(fs, env);

    a.checkEqual("01", testee("a"), "xyz");
}
