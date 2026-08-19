/**
  *  \file test/server/console/dbexportertest.cpp
  *  \brief Test for server::console::DbExporter
  */

#include "server/console/dbexporter.hpp"

#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internaltextwriter.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/console/dumbterminal.hpp"
#include "server/console/fileexporter.hpp"
#include "server/file/internalfileserver.hpp"

using afl::data::Segment;
using server::console::DbExporter;
using server::console::FileExporter;

/** Simple test. This is just a litmus test, for coverage and for testing basic layout.
    It is also tested in c2systest/dbexporter/01_types. */
AFL_TEST("server.console.DBExporter:types", a)
{
    afl::net::redis::InternalDatabase db;
    db.callVoid(Segment().pushBackString("set").pushBackString("a").pushBackInteger(1));
    db.callVoid(Segment().pushBackString("set").pushBackString("b").pushBackString("word"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("c").pushBackString("k").pushBackString("hash"));
    db.callVoid(Segment().pushBackString("sadd").pushBackString("d").pushBackString("set"));
    db.callVoid(Segment().pushBackString("rpush").pushBackString("e").pushBackString("x"));

    afl::io::InternalTextWriter t;
    server::console::DumbTerminal term(t, t);
    Segment arg;
    arg.pushBackString("*");
    interpreter::Arguments c(arg, 0, arg.size());

    server::console::exportDatabase(term, db, c);

    a.checkEqual("result", afl::string::fromMemory(t.getContent()),
                 "silent redis set   a                              1\n"
                 "silent redis set   b                              word\n"
                 "silent redis hset  c                              k hash\n"
                 "silent redis sadd  d                              set\n"
                 "silent redis rpush e                              x\n");
}

/** String test. Tests stringification. */
AFL_TEST("server.console.DBExporter:strings", a)
{
    afl::net::redis::InternalDatabase db;
    db.callVoid(Segment().pushBackString("set").pushBackString("a").pushBackString("a'b"));
    db.callVoid(Segment().pushBackString("set").pushBackString("b").pushBackString("a$b"));
    db.callVoid(Segment().pushBackString("set").pushBackString("c").pushBackString("a\nb"));
    db.callVoid(Segment().pushBackString("set").pushBackString("d").pushBackString("a\n\r\tb"));
    db.callVoid(Segment().pushBackString("set").pushBackString("e").pushBackString("a'\"b"));
    db.callVoid(Segment().pushBackString("set").pushBackString("f").pushBackString("a\033b"));

    afl::io::InternalTextWriter t;
    server::console::DumbTerminal term(t, t);
    Segment arg;
    arg.pushBackString("*");
    interpreter::Arguments c(arg, 0, arg.size());

    server::console::exportDatabase(term, db, c);

    a.checkEqual("result", afl::string::fromMemory(t.getContent()),
                 "silent redis set   a                              \"a'b\"\n"
                 "silent redis set   b                              'a$b'\n"
                 "silent redis set   c                              \"a\\nb\"\n"
                 "silent redis set   d                              \"a\\n\\r\\tb\"\n"
                 "silent redis set   e                              \"a'\\\"b\"\n"
                 "silent redis set   f                              \"a\\x1B""b\"\n");
}

/*
 *  The following test "few large" vs. "many small" elements.
 *  We had a typo here causing some combinations to crash.
 *  Acceptance criterion is therefore just that sensible output is produced.
 *  Since each line has at least 50 characters ("silent redis $CMD $KEY"),
 *  output for 1000 elements is at least 50k.
 */

/** Test export of large list. */
AFL_TEST("server.console.DBExporter:largs-list", a)
{
    // A list with 1000 elements
    afl::net::redis::InternalDatabase db;
    for (int i = 0; i < 1000; ++i) {
        db.callVoid(Segment().pushBackString("rpush").pushBackString("a").pushBackInteger(i));
    }

    afl::io::InternalTextWriter t;
    server::console::DumbTerminal term(t, t);
    Segment arg;
    arg.pushBackString("*");
    interpreter::Arguments c(arg, 0, arg.size());
    server::console::exportDatabase(term, db, c);
    a.checkGreaterThan("result size", t.getContent().size(), 50000U);
}

/** Test export of many lists. */
AFL_TEST("server.console.DBExporter:many-lists", a)
{
    // 1000 lists of 1 element each
    afl::net::redis::InternalDatabase db;
    for (int i = 0; i < 1000; ++i) {
        db.callVoid(Segment().pushBackString("rpush").pushBackInteger(i).pushBackString("a"));
    }

    afl::io::InternalTextWriter t;
    server::console::DumbTerminal term(t, t);
    Segment arg;
    arg.pushBackString("*");
    interpreter::Arguments c(arg, 0, arg.size());
    server::console::exportDatabase(term, db, c);
    a.checkGreaterThan("result size", t.getContent().size(), 50000U);
}

/** Test export of large set. */
AFL_TEST("server.console.DBExporter:large-set", a)
{
    // Set with 1000 elements.
    afl::net::redis::InternalDatabase db;
    for (int i = 0; i < 1000; ++i) {
        db.callVoid(Segment().pushBackString("sadd").pushBackString("a").pushBackInteger(i));
    }

    afl::io::InternalTextWriter t;
    server::console::DumbTerminal term(t, t);
    Segment arg;
    arg.pushBackString("*");
    interpreter::Arguments c(arg, 0, arg.size());
    server::console::exportDatabase(term, db, c);
    a.checkGreaterThan("result size", t.getContent().size(), 50000U);
}

/** Test export of many sets. */
AFL_TEST("server.console.DBExporter:many-sets", a)
{
    // 1000 sets with 1 element each
    afl::net::redis::InternalDatabase db;
    for (int i = 0; i < 1000; ++i) {
        db.callVoid(Segment().pushBackString("sadd").pushBackInteger(i).pushBackString("a"));
    }

    afl::io::InternalTextWriter t;
    server::console::DumbTerminal term(t, t);
    Segment arg;
    arg.pushBackString("*");
    interpreter::Arguments c(arg, 0, arg.size());
    server::console::exportDatabase(term, db, c);
    a.checkGreaterThan("result size", t.getContent().size(), 50000U);
}

/** Test export of large hash. */
AFL_TEST("server.console.DBExporter:large-hash", a)
{
    // Hash with 1000 keys.
    afl::net::redis::InternalDatabase db;
    for (int i = 0; i < 1000; ++i) {
        db.callVoid(Segment().pushBackString("hset").pushBackString("a").pushBackInteger(i).pushBackString("x"));
    }

    afl::io::InternalTextWriter t;
    server::console::DumbTerminal term(t, t);
    Segment arg;
    arg.pushBackString("*");
    interpreter::Arguments c(arg, 0, arg.size());
    server::console::exportDatabase(term, db, c);
    a.checkGreaterThan("result size", t.getContent().size(), 50000U);
}

/** Test export of many hashes. */
AFL_TEST("server.console.DBExporter:many-hashes", a)
{
    // 1000 hashes with 1 key.
    afl::net::redis::InternalDatabase db;
    for (int i = 0; i < 1000; ++i) {
        db.callVoid(Segment().pushBackString("hset").pushBackInteger(i).pushBackString("a").pushBackString("x"));
    }

    afl::io::InternalTextWriter t;
    server::console::DumbTerminal term(t, t);
    Segment arg;
    arg.pushBackString("*");
    interpreter::Arguments c(arg, 0, arg.size());
    server::console::exportDatabase(term, db, c);
    a.checkGreaterThan("result size", t.getContent().size(), 50000U);
}

/** Test DBExporter class, exporting database objects. */
AFL_TEST("server.console.DBExporter:db-objects", a)
{
    // Create two users
    afl::net::redis::InternalDatabase db;
    db.callVoid(Segment().pushBackString("set").pushBackString("user:1009:name").pushBackString("fred"));
    db.callVoid(Segment().pushBackString("set").pushBackString("uid:fred").pushBackString("1009"));

    db.callVoid(Segment().pushBackString("set").pushBackString("user:1020:name").pushBackString("barney"));
    db.callVoid(Segment().pushBackString("set").pushBackString("uid:barney").pushBackString("1020"));

    // Create a PM
    db.callVoid(Segment().pushBackString("sadd").pushBackString("default:folder:all").pushBackString("7"));
    db.callVoid(Segment().pushBackString("sadd").pushBackString("user:1009:pm:folder:7:messages").pushBackString("49"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("pm:49:header").pushBackString("author").pushBackString("1009"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("pm:49:header").pushBackString("flags/1009").pushBackString("1"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("pm:49:header").pushBackString("flags/1020").pushBackString("2"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("pm:49:header").pushBackString("to").pushBackString("u:1020"));
    db.callVoid(Segment().pushBackString("set").pushBackString("pm:49:text").pushBackString("text..."));

    // Create a message in a forum
    db.callVoid(Segment().pushBackString("sadd").pushBackString("user:1009:forum:posted").pushBackString("7"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("forum:3:header").pushBackString("name").pushBackString("Talk"));
    db.callVoid(Segment().pushBackString("sadd").pushBackString("forum:3:messages").pushBackString("7"));
    db.callVoid(Segment().pushBackString("sadd").pushBackString("forum:3:threads").pushBackString("5"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("thread:5:header").pushBackString("forum").pushBackString("3"));
    db.callVoid(Segment().pushBackString("sadd").pushBackString("thread:5:messages").pushBackString("7"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("msg:7:header").pushBackString("author").pushBackString("1009"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("msg:7:header").pushBackString("thread").pushBackString("5"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("msg:7:header").pushBackString("subject").pushBackString("sub..."));
    db.callVoid(Segment().pushBackString("set").pushBackString("msg:7:text").pushBackString("msgtext"));

    // Export a user
    {
        DbExporter testee;
        testee.add(DbExporter::User, "1009", DbExporter::Recursive);
        testee.complete(db);

        afl::io::InternalTextWriter t;
        server::console::DumbTerminal term(t, t);
        testee.generate(term, db);

        a.checkEqual("user result", afl::string::fromMemory(t.getContent()),
                     // User 1009 exported completely
                     "silent redis sadd  user:1009:forum:posted         7\n"
                     "silent redis set   user:1009:name                 fred\n"
                     "silent redis sadd  user:1009:pm:folder:7:messages 49\n"
                     "silent redis set   uid:fred                       1009\n"
                     // User 1020 referenced by the PM
                     "silent redis set   user:1020:name                 barney\n"
                     "silent redis set   uid:barney                     1020\n"
                     // PM (note flags only for 1009)
                     "silent redis set   pm:49:text                     text...\n"
                     "silent redis hset  pm:49:header                   author 1009\n"
                     "silent redis hset  pm:49:header                   flags/1009 1\n"
                     "silent redis hset  pm:49:header                   to u:1020\n"
                     // Thread header only
                     "silent redis hset  thread:5:header                forum 3\n"
                     // Forum post
                     "silent redis hset  msg:7:header                   author 1009\n"
                     "silent redis hset  msg:7:header                   subject sub...\n"
                     "silent redis hset  msg:7:header                   thread 5\n"
                     "silent redis set   msg:7:text                     msgtext\n");
    }

    // Export a thread
    {
        DbExporter testee;
        testee.add(DbExporter::Thread, "5", DbExporter::Recursive);
        testee.complete(db);

        afl::io::InternalTextWriter t;
        server::console::DumbTerminal term(t, t);
        testee.generate(term, db);

        a.checkEqual("thread result", afl::string::fromMemory(t.getContent()),
                     // Mentioned user
                     "silent redis set   user:1009:name                 fred\n"
                     "silent redis set   uid:fred                       1009\n"
                     // Forum header
                     "silent redis hset  forum:3:header                 name Talk\n"
                     "silent redis sadd  forum:3:messages               7\n"
                     "silent redis sadd  forum:3:messages               7\n"
                     // Thread
                     "silent redis hset  thread:5:header                forum 3\n"
                     "silent redis sadd  thread:5:messages              7\n"
                     // Message
                     "silent redis hset  msg:7:header                   author 1009\n"
                     "silent redis hset  msg:7:header                   subject sub...\n"
                     "silent redis hset  msg:7:header                   thread 5\n"
                     "silent redis set   msg:7:text                     msgtext\n");
    }

    // Export a PM
    {
        DbExporter testee;
        testee.add(DbExporter::PM, "49", DbExporter::Recursive);
        testee.complete(db);

        afl::io::InternalTextWriter t;
        server::console::DumbTerminal term(t, t);
        testee.generate(term, db);

        a.checkEqual("pm result", afl::string::fromMemory(t.getContent()),
                     // Exporting the PM does not un-redact the user flags!
                     "silent redis set   user:1009:name                 fred\n"
                     "silent redis set   uid:fred                       1009\n"
                     "silent redis set   user:1020:name                 barney\n"
                     "silent redis set   uid:barney                     1020\n"
                     "silent redis set   pm:49:text                     text...\n"
                     "silent redis hset  pm:49:header                   author 1009\n"
                     "silent redis hset  pm:49:header                   to u:1020\n");
    }

    // Export unknown object
    {
        DbExporter testee;
        testee.add(DbExporter::PM, "999", DbExporter::Recursive);
        testee.add(DbExporter::User, "999", DbExporter::Recursive);
        testee.complete(db);

        afl::io::InternalTextWriter t;
        server::console::DumbTerminal term(t, t);
        testee.generate(term, db);

        a.checkEqual("pm result", afl::string::fromMemory(t.getContent()),
                     // The exact wording is probably not contractual
                     "# warning: key pm:999:text got deleted during export\n");
    }
}

/** Test DBExporter class, exporting file objects. */
AFL_TEST("server.console.DBExporter:file-objects", a)
{
    // Create a user
    afl::net::redis::InternalDatabase db;
    db.callVoid(Segment().pushBackString("set").pushBackString("user:1009:name").pushBackString("fred"));
    db.callVoid(Segment().pushBackString("set").pushBackString("uid:fred").pushBackString("1009"));

    // Create some files for this user
    server::file::InternalFileServer fserv;
    fserv.callVoid(Segment().pushBackString("mkdir").pushBackString("u"));
    fserv.callVoid(Segment().pushBackString("mkdiras").pushBackString("u/fred").pushBackString("1009"));
    fserv.callVoid(Segment().pushBackString("mkdir").pushBackString("u/fred/sub"));
    fserv.callVoid(Segment().pushBackString("put").pushBackString("u/fred/sub/hello.txt").pushBackString("hi"));

    // Export
    {
        DbExporter testee;
        testee.add(DbExporter::User, "1009", DbExporter::Recursive);
        testee.complete(db);

        afl::io::InternalFileSystem fsys;
        std::auto_ptr<FileExporter> fx(FileExporter::create(FileExporter::InlineFiles, fsys, "x"));
        a.checkNonNull("file exporter", fx.get());

        afl::io::InternalTextWriter t;
        server::console::DumbTerminal term(t, t);
        testee.generate(term, db);
        testee.generateUserFiles(term, fserv, *fx);

        a.checkEqual("export result", afl::string::fromMemory(t.getContent()),
                     "silent redis set   user:1009:name                 fred\n"
                     "silent redis set   uid:fred                       1009\n"
                     "silent file     mkdirhier u/fred\n"
                     "silent file     mkdirhier u/fred/sub\n"
                     "silent file     put       u/fred/sub/hello.txt           hi\n");
    }
}
