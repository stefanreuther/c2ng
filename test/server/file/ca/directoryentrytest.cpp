/**
  *  \file test/server/file/ca/directoryentrytest.cpp
  *  \brief Test for server::file::ca::DirectoryEntry
  */

#include "server/file/ca/directoryentry.hpp"
#include "afl/test/testrunner.hpp"

/** Basic tests. */
AFL_TEST("server.file.ca.DirectoryEntry:basics", a)
{
    // Some data (a directory with two entries)
    static const uint8_t DATA[] = {
        // 40000 dir
        0x34, 0x30, 0x30, 0x30, 0x30, 0x20, 0x64, 0x69, 0x72, 0x00,
        0x39, 0x7b, 0xbf, 0x05, 0x97, 0x39, 0xcb, 0xfa, 0x73, 0xaa, 0xd2, 0xf8, 0xbf, 0x40, 0x4d, 0x04, 0xf4, 0x78, 0xb3, 0x8a,

        // 100644 file
        0x31, 0x30, 0x30, 0x36, 0x34, 0x34, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x00,
        0xa7, 0xf8, 0xd9, 0xe5, 0xdc, 0xf3, 0xa6, 0x8f, 0xdd, 0x2b, 0xfb, 0x72, 0x7c, 0xde, 0x12, 0x02, 0x98, 0x75, 0x26, 0x0b,
    };
    afl::base::ConstBytes_t in(DATA);
    afl::base::GrowableMemory<uint8_t> out;

    // Parse first entry
    server::file::ca::DirectoryEntry testee;
    bool ok = testee.parse(in);
    a.check("01. parse", ok);
    a.checkEqual("02. getName", testee.getName(), "dir");
    a.checkEqual("03. getType", testee.getType(), server::file::DirectoryHandler::IsDirectory);
    a.checkEqual("04. getId",   testee.getId().m_bytes[0], 0x39U);
    a.checkEqual("05. getId",   testee.getId().m_bytes[19], 0x8AU);
    testee.store(out);

    // Parse second entry
    ok = testee.parse(in);
    a.check("11. parse", ok);
    a.checkEqual("12. getName", testee.getName(), "file");
    a.checkEqual("13. getType", testee.getType(), server::file::DirectoryHandler::IsFile);
    a.checkEqual("14. getId",   testee.getId().m_bytes[0], 0xA7U);
    a.checkEqual("15. getId",   testee.getId().m_bytes[19], 0x0BU);
    testee.store(out);

    // Finish
    a.check("21. empty", in.empty());
    ok = testee.parse(in);
    a.check("22. ok", !ok);
    a.checkEqual("23. size", out.size(), sizeof(DATA));
    a.check("24. data", out.equalContent(DATA));
}

/** Test errors. */
// Truncated
AFL_TEST("server.file.ca.DirectoryEntry:error:truncated", a)
{
    static const uint8_t DATA[] = {
        // 40000 dir
        0x34, 0x30, 0x30, 0x30, 0x30, 0x20, 0x64, 0x69, 0x72, 0x00,
        0x39, 0x7b, 0xbf, 0x05, 0x97, 0x39, 0xcb, 0xfa, 0x73, 0xaa, 0xd2, 0xf8, 0xbf, 0x40, 0x4d, 0x04, 0xf4, 0x78, 0xb3, 0x8a,
    };
    for (size_t i = 0; i < sizeof(DATA); ++i) {
        afl::base::ConstBytes_t in(DATA);
        in.trim(i);
        a.check("parse", !server::file::ca::DirectoryEntry().parse(in));
    }
}

// Bad number
AFL_TEST("server.file.ca.DirectoryEntry:error:number", a)
{
    static const uint8_t DATA[] = {
        // 40009 dir
        0x34, 0x30, 0x30, 0x30, 0x39, 0x20, 0x64, 0x69, 0x72, 0x00,
        0x39, 0x7b, 0xbf, 0x05, 0x97, 0x39, 0xcb, 0xfa, 0x73, 0xaa, 0xd2, 0xf8, 0xbf, 0x40, 0x4d, 0x04, 0xf4, 0x78, 0xb3, 0x8a,
    };
    afl::base::ConstBytes_t in(DATA);
    a.check("parse", !server::file::ca::DirectoryEntry().parse(in));
}


/** Test parsing a directory containing an unsupported element. */
AFL_TEST("server.file.ca.DirectoryEntry:unsupported", a)
{
    static const uint8_t DATA[] = {
        // 120000 aa (a symlink)
        0x31, 0x32, 0x30, 0x30, 0x30, 0x30, 0x20, 0x61, 0x61, 0x00,
        0x2e, 0x65, 0xef, 0xe2, 0xa1, 0x45, 0xdd, 0xa7, 0xee, 0x51, 0xd1, 0x74, 0x12, 0x99, 0xf8, 0x48, 0xe5, 0xbf, 0x75, 0x2e
    };
    afl::base::ConstBytes_t in(DATA);
    afl::base::GrowableMemory<uint8_t> out;

    // Test parsing
    server::file::ca::DirectoryEntry testee;
    a.check("01. parse", testee.parse(in));
    a.checkEqual("02. getName", testee.getName(), "aa");
    a.checkEqual("03. getType", testee.getType(), server::file::DirectoryHandler::IsUnknown);

    // Test roundtrip
    testee.store(out);
    a.check("11. data", out.equalContent(DATA));
}

/** Test constructing. */
AFL_TEST("server.file.ca.DirectoryEntry:construct", a)
{
    static const server::file::ca::ObjectId id = {{0x2e,0x65,0xef,0xe2,0xa1,0x45,0xdd,0xa7,0xee,0x51,0xd1,0x74,0x12,0x99,0xf8,0x48,0xe5,0xbf,0x75,0x2e}};

    afl::base::GrowableMemory<uint8_t> out;
    server::file::ca::DirectoryEntry("Name", id, server::file::DirectoryHandler::IsFile).store(out);

    static const uint8_t DATA[] = {
        '1','0','0','6','4','4',' ','N','a','m','e',0,
        0x2e,0x65,0xef,0xe2,0xa1,0x45,0xdd,0xa7,0xee,0x51,0xd1,0x74,0x12,0x99,0xf8,0x48,0xe5,0xbf,0x75,0x2e
    };
    a.check("01. data", out.equalContent(DATA));
}

/** Test comparison. */
AFL_TEST("server.file.ca.DirectoryEntry:compare", a)
{
    using server::file::ca::DirectoryEntry;
    using server::file::ca::ObjectId;
    using server::file::DirectoryHandler;

    DirectoryEntry e1("a",     ObjectId::nil, DirectoryHandler::IsFile);
    DirectoryEntry e2("a",     ObjectId::nil, DirectoryHandler::IsDirectory);
    DirectoryEntry e3("a.txt", ObjectId::nil, DirectoryHandler::IsFile);
    DirectoryEntry e4("a0",    ObjectId::nil, DirectoryHandler::IsFile);

    // Comparison with self
    a.check("01", !e1.isBefore(e1));
    a.check("02", !e2.isBefore(e2));
    a.check("03", !e3.isBefore(e3));
    a.check("04", !e4.isBefore(e4));

    // Same name, different type. File goes first, but this is not a valid ordering question
    // because we cannot have a file and a directory of the same name in one directory!
    a.check("11", e1.isBefore(e2));
    a.check("12", !e2.isBefore(e1));

    // Ordering e1 < e3 < e2 < e4
    a.check("21", e1.isBefore(e3));
    a.check("22", e1.isBefore(e2));
    a.check("23", e1.isBefore(e4));

    a.check("31", e3.isBefore(e2));
    a.check("32", e3.isBefore(e4));

    a.check("41", e2.isBefore(e4));
}

/** More comparison. */
AFL_TEST("server.file.ca.DirectoryEntry:compare:2", a)
{
    using server::file::ca::DirectoryEntry;
    using server::file::ca::ObjectId;
    using server::file::DirectoryHandler;

    DirectoryEntry e1("plist-2.4-lite", ObjectId::nil, DirectoryHandler::IsDirectory);
    DirectoryEntry e2("plist-2.4",      ObjectId::nil, DirectoryHandler::IsDirectory);
    DirectoryEntry e3("plist-3.2",      ObjectId::nil, DirectoryHandler::IsDirectory);

    a.check("01", e1.isBefore(e2));
    a.check("02", e1.isBefore(e3));
    a.check("03", e2.isBefore(e3));
}
