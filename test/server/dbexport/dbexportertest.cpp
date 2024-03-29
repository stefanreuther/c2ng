/**
  *  \file test/server/dbexport/dbexportertest.cpp
  *  \brief Test for server::dbexport::DBExporter
  */

#include "server/dbexport/dbexporter.hpp"

#include <stdexcept>
#include "afl/data/segment.hpp"
#include "afl/io/internaltextwriter.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/commandlineparser.hpp"
#include "afl/test/testrunner.hpp"

using afl::data::Segment;

namespace {
    class CommandLineParserMock : public afl::sys::CommandLineParser {
     public:
        CommandLineParserMock(afl::base::Memory<const char*const> args)
            : m_args(args)
            { }
        virtual bool getNext(bool& option, String_t& text)
            {
                if (const char*const* p = m_args.eat()) {
                    text = *p;
                    option = false;
                    return true;
                } else {
                    return false;
                }
            }
        virtual bool getParameter(String_t& /*value*/)
            {
                // should not be called
                throw std::runtime_error("getParameter unexpected");
            }
        virtual Flags_t getFlags()
            { return Flags_t(); }
     private:
        afl::base::Memory<const char*const> m_args;
    };


    const char*const DEFAULT_ARGS[] = {
        "*"
    };
}

/** Simple test. This is just a litmus test, for coverage and for testing basic layout.
    It is also tested in c2systest/dbexporter/01_types. */
AFL_TEST("server.dbexport.DBExporter:types", a)
{
    afl::string::NullTranslator tx;
    afl::net::redis::InternalDatabase db;
    db.callVoid(Segment().pushBackString("set").pushBackString("a").pushBackInteger(1));
    db.callVoid(Segment().pushBackString("set").pushBackString("b").pushBackString("word"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("c").pushBackString("k").pushBackString("hash"));
    db.callVoid(Segment().pushBackString("sadd").pushBackString("d").pushBackString("set"));
    db.callVoid(Segment().pushBackString("rpush").pushBackString("e").pushBackString("x"));

    afl::io::InternalTextWriter t;
    CommandLineParserMock c(DEFAULT_ARGS);

    server::dbexport::exportDatabase(t, db, c, tx);

    a.checkEqual("result", afl::string::fromMemory(t.getContent()),
                 "silent redis set   a                              1\n"
                 "silent redis set   b                              word\n"
                 "silent redis hset  c                              k hash\n"
                 "silent redis sadd  d                              set\n"
                 "silent redis rpush e                              x\n");
}

/** String test. Tests stringification. */
AFL_TEST("server.dbexport.DBExporter:strings", a)
{
    afl::string::NullTranslator tx;
    afl::net::redis::InternalDatabase db;
    db.callVoid(Segment().pushBackString("set").pushBackString("a").pushBackString("a'b"));
    db.callVoid(Segment().pushBackString("set").pushBackString("b").pushBackString("a$b"));
    db.callVoid(Segment().pushBackString("set").pushBackString("c").pushBackString("a\nb"));
    db.callVoid(Segment().pushBackString("set").pushBackString("d").pushBackString("a\n\r\tb"));
    db.callVoid(Segment().pushBackString("set").pushBackString("e").pushBackString("a'\"b"));
    db.callVoid(Segment().pushBackString("set").pushBackString("f").pushBackString("a\033b"));

    afl::io::InternalTextWriter t;
    CommandLineParserMock c(DEFAULT_ARGS);

    server::dbexport::exportDatabase(t, db, c, tx);

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
AFL_TEST("server.dbexport.DBExporter:largs-list", a)
{
    // A list with 1000 elements
    afl::string::NullTranslator tx;
    afl::net::redis::InternalDatabase db;
    for (int i = 0; i < 1000; ++i) {
        db.callVoid(Segment().pushBackString("rpush").pushBackString("a").pushBackInteger(i));
    }

    afl::io::InternalTextWriter t;
    CommandLineParserMock c(DEFAULT_ARGS);
    server::dbexport::exportDatabase(t, db, c, tx);
    a.checkGreaterThan("result size", t.getContent().size(), 50000U);
}

/** Test export of many lists. */
AFL_TEST("server.dbexport.DBExporter:many-lists", a)
{
    // 1000 lists of 1 element each
    afl::string::NullTranslator tx;
    afl::net::redis::InternalDatabase db;
    for (int i = 0; i < 1000; ++i) {
        db.callVoid(Segment().pushBackString("rpush").pushBackInteger(i).pushBackString("a"));
    }

    afl::io::InternalTextWriter t;
    CommandLineParserMock c(DEFAULT_ARGS);
    server::dbexport::exportDatabase(t, db, c, tx);
    a.checkGreaterThan("result size", t.getContent().size(), 50000U);
}

/** Test export of large set. */
AFL_TEST("server.dbexport.DBExporter:large-set", a)
{
    // Set with 1000 elements.
    afl::string::NullTranslator tx;
    afl::net::redis::InternalDatabase db;
    for (int i = 0; i < 1000; ++i) {
        db.callVoid(Segment().pushBackString("sadd").pushBackString("a").pushBackInteger(i));
    }

    afl::io::InternalTextWriter t;
    CommandLineParserMock c(DEFAULT_ARGS);
    server::dbexport::exportDatabase(t, db, c, tx);
    a.checkGreaterThan("result size", t.getContent().size(), 50000U);
}

/** Test export of many sets. */
AFL_TEST("server.dbexport.DBExporter:many-sets", a)
{
    // 1000 sets with 1 element each
    afl::string::NullTranslator tx;
    afl::net::redis::InternalDatabase db;
    for (int i = 0; i < 1000; ++i) {
        db.callVoid(Segment().pushBackString("sadd").pushBackInteger(i).pushBackString("a"));
    }

    afl::io::InternalTextWriter t;
    CommandLineParserMock c(DEFAULT_ARGS);
    server::dbexport::exportDatabase(t, db, c, tx);
    a.checkGreaterThan("result size", t.getContent().size(), 50000U);
}

/** Test export of large hash. */
AFL_TEST("server.dbexport.DBExporter:large-hash", a)
{
    // Hash with 1000 keys.
    afl::string::NullTranslator tx;
    afl::net::redis::InternalDatabase db;
    for (int i = 0; i < 1000; ++i) {
        db.callVoid(Segment().pushBackString("hset").pushBackString("a").pushBackInteger(i).pushBackString("x"));
    }

    afl::io::InternalTextWriter t;
    CommandLineParserMock c(DEFAULT_ARGS);
    server::dbexport::exportDatabase(t, db, c, tx);
    a.checkGreaterThan("result size", t.getContent().size(), 50000U);
}

/** Test export of many hashes. */
AFL_TEST("server.dbexport.DBExporter:many-hashes", a)
{
    // 1000 hashes with 1 key.
    afl::string::NullTranslator tx;
    afl::net::redis::InternalDatabase db;
    for (int i = 0; i < 1000; ++i) {
        db.callVoid(Segment().pushBackString("hset").pushBackInteger(i).pushBackString("a").pushBackString("x"));
    }

    afl::io::InternalTextWriter t;
    CommandLineParserMock c(DEFAULT_ARGS);
    server::dbexport::exportDatabase(t, db, c, tx);
    a.checkGreaterThan("result size", t.getContent().size(), 50000U);
}
