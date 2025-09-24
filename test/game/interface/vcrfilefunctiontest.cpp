/**
  *  \file test/game/interface/vcrfilefunctiontest.cpp
  *  \brief Test for game::interface::VcrFileFunction
  */

#include "game/interface/vcrfilefunction.hpp"

#include "afl/data/segment.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/vcr/test/database.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

using afl::base::Ref;
using afl::data::Segment;
using afl::io::FileSystem;
using afl::io::Stream;
using game::interface::VcrFileFunction;
using game::vcr::test::Database;
using interpreter::Arguments;
using interpreter::Context;
using interpreter::Error;
using interpreter::IndexableValue;
using interpreter::test::ContextVerifier;
using interpreter::test::ValueVerifier;

namespace {
    // VCR file (from game.vcr.classic.Database:load:phost4)
    static const uint8_t VCR_FILE[] = {
        0x02, 0x00, 0x6c, 0x8b, 0x83, 0x33, 0x03, 0x80, 0x01, 0x00, 0xec, 0x01, 0x64, 0x00, 0x56, 0x69,
        0x72, 0x75, 0x73, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x00, 0x00, 0xc7, 0x01, 0x2b, 0x00, 0x08, 0x00, 0x72, 0x48, 0x08, 0x00, 0x08, 0x01,
        0x08, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x5a, 0x69, 0x6d, 0x70, 0x68, 0x66, 0x69, 0x72,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00,
        0xd8, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x64, 0x00, 0x64, 0x00, 0x7c, 0xab, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x02,
        0x88, 0x01, 0x53, 0x65, 0x72, 0x69, 0x61, 0x6c, 0x20, 0x23, 0x54, 0x47, 0x44, 0x42, 0x41, 0x43,
        0x59, 0x56, 0x53, 0x4f, 0x53, 0x51, 0x00, 0x00, 0x2a, 0x03, 0x0b, 0x00, 0x04, 0x00, 0x41, 0x26,
        0x0a, 0x00, 0x0d, 0x01, 0x00, 0x00, 0x0a, 0x00, 0x2f, 0x00, 0x09, 0x00, 0x44, 0x72, 0x61, 0x67,
        0x73, 0x74, 0x65, 0x72, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x00, 0x00, 0x14, 0x01, 0x5a, 0x00, 0x08, 0x00, 0x69, 0x45, 0x09, 0x00, 0x04, 0x01, 0x06, 0x00,
        0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x64, 0x00, 0x64, 0x00,
    };

    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::InternalFileSystem fs;
        game::Session session;

        Environment()
            : tx(), fs(), session(tx, fs)
            { }
    };

    void addRoot(Environment& env)
    {
        env.session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    }

    void addShipList(Environment& env)
    {
        env.session.setShipList(new game::spec::ShipList());
    }
}


/* Simple test for VcrFileFunction.
   A: create a VcrFileFunction object with a dummy database. Inspect its properties/call it.
   E: calls behave as expected */
AFL_TEST("game.interface.VcrFileFunction:create:ok", a)
{
    Environment env;
    addRoot(env);
    addShipList(env);

    Ref<Database> db = *new Database();
    db->addBattle().setAlgorithmName("first");
    db->addBattle().setAlgorithmName("second");
    db->addBattle().setAlgorithmName("third");

    std::auto_ptr<VcrFileFunction> testee(VcrFileFunction::create(env.session, db));

    // Test basic properties
    ValueVerifier verif(*testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    a.checkEqual("01. battles", &*testee->battles(), &*db);
    a.checkEqual("02. dim 0", testee->getDimension(0), 1U);
    a.checkEqual("03. dim 1", testee->getDimension(1), 4U);

    // Test successful invocation: attribute VcrFile()(2).ALGORITHM must have expected value
    {
        Segment seg;
        seg.pushBackInteger(2);
        Arguments args(seg, 0, 1);
        std::auto_ptr<Context> result(testee->get(args));
        a.checkNonNull("11. get", result.get());
        ContextVerifier(*result, a("12. get")).verifyString("ALGORITHM", "second");
    }

    // Test failing invocation
    {
        // arity error: VcrFile()() fails
        Segment seg;
        Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("21. arity error"), testee->get(args), Error);
    }
    {
        // type error: VcrFile()("X") fails
        Segment seg;
        seg.pushBackString("X");
        Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("22. type error"), testee->get(args), Error);
    }
    {
        // range error: VcrFile()(0) fails (minimum is 1)
        Segment seg;
        seg.pushBackInteger(0);
        Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("23. range error"), testee->get(args), Error);
    }
    {
        // range error: VcrFile()(4) fails (maximum is 3)
        Segment seg;
        seg.pushBackInteger(4);
        Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("24. range error"), testee->get(args), Error);
    }

    // Test invocation with null: VcrFile()(null) must be null
    {
        Segment seg;
        seg.pushBackNew(0);
        Arguments args(seg, 0, 1);
        std::auto_ptr<Context> result(testee->get(args));
        a.checkNull("31. null", result.get());
    }

    // Test iteration (ForEach VcrFile()...)
    {
        std::auto_ptr<Context> result(testee->makeFirstContext());
        a.checkNonNull("41. makeFirstContext", result.get());
        ContextVerifier(*result, a("42. makeFirstContext")).verifyString("ALGORITHM", "first");
    }

    // Test set: VcrFile() cannot be assigned to
    {
        Segment seg;
        seg.pushBackInteger(3);
        Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("51. set"), testee->set(args, 0), Error);
    }
}

/* Test creation from empty database.
   A: Attempt to create a VcrFileFunction from an empty (but non-null) VCR database
   E: result is null */
AFL_TEST("game.interface.VcrFileFunction:create:empty", a)
{
    Environment env;
    addRoot(env);
    addShipList(env);

    Ref<Database> db = *new Database();
    std::auto_ptr<VcrFileFunction> testee(VcrFileFunction::create(env.session, db));
    a.checkNull("01. create", testee.get());
}

/* Test interface function for normal VCR file.
   A: Create file with classic VCR content, open it, call VcrFile(#fd).
   E: Content successfully loaded */
AFL_TEST("game.interface.VcrFileFunction:IFVcrFile:normal", a)
{
    // Environment
    Environment env;
    addRoot(env);
    addShipList(env);
    env.fs.openFile("/test.dat", FileSystem::Create)
        ->fullWrite(VCR_FILE);

    // Open the file
    env.session.world().fileTable().openFile(7, env.fs.openFile("/test.dat", FileSystem::OpenRead));

    // Do it
    Segment seg0;
    seg0.pushBackInteger(7);
    Arguments args0(seg0, 0, 1);
    std::auto_ptr<afl::data::Value> result0(game::interface::IFVcrFile(env.session, args0));
    IndexableValue* iv = dynamic_cast<IndexableValue*>(result0.get());

    // Verify
    a.checkNonNull("01. result", result0.get());
    a.checkNonNull("02. indexable", iv);
    a.checkEqual("03. dim", iv->getDimension(1), 3U);

    // Verify content
    {
        Segment seg;
        seg.pushBackInteger(2);
        Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> result(iv->get(args));
        a.checkNonNull("11. get", dynamic_cast<Context*>(result.get()));
        ContextVerifier(*dynamic_cast<Context*>(result.get()), a("12. get")).verifyString("ALGORITHM", "PHost 4");
    }
}

/* Test interface function for FLAK file.
   A: Create file with FLAK content, open it, call VcrFile(#fd).
   E: Content successfully loaded */
AFL_TEST("game.interface.VcrFileFunction:IFVcrFile:flak", a)
{
    // FLAK file (created using simulator)
    static const uint8_t FILE[] = {
        0x46, 0x4c, 0x41, 0x4b, 0x56, 0x43, 0x52, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x30, 0x30, 0x2d, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3a, 0x30, 0x30, 0x3a,
        0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0xec, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xda, 0x97,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x18, 0x00,
        0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x68, 0x00,
        0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xe4, 0x00, 0x00, 0x00, 0x0c, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x60, 0x6d,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x01, 0x00, 0x64, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xa0, 0x92, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x53, 0x68,
        0x69, 0x70, 0x20, 0x31, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd2, 0x00, 0x64, 0x00,
        0x01, 0x00, 0xdc, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x53, 0x68, 0x69, 0x70,
        0x20, 0x32, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x00, 0x00, 0xe0, 0x00, 0x02, 0x00, 0x0b, 0x00, 0x59, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0a, 0x00,
        0x03, 0x00, 0x3c, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x01, 0x64, 0x00, 0x01, 0x00,
        0x4f, 0x01, 0x00, 0x00, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x20, 0x00, 0x00, 0x00,
        0x27, 0x00
    };

    // Environment
    Environment env;
    addRoot(env);
    addShipList(env);
    env.fs.openFile("/test.dat", FileSystem::Create)
        ->fullWrite(FILE);

    // Open the file
    env.session.world().fileTable().openFile(7, env.fs.openFile("/test.dat", FileSystem::OpenRead));

    // Do it
    Segment seg0;
    seg0.pushBackInteger(7);
    Arguments args0(seg0, 0, 1);
    std::auto_ptr<afl::data::Value> result0(game::interface::IFVcrFile(env.session, args0));
    IndexableValue* iv = dynamic_cast<IndexableValue*>(result0.get());

    // Verify
    a.checkNonNull("01. result", result0.get());
    a.checkNonNull("02. indexable", iv);
    a.checkEqual("03. dim", iv->getDimension(1), 2U);

    // Verify content
    {
        Segment seg;
        seg.pushBackInteger(1);
        Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> result(iv->get(args));
        a.checkNonNull("11. get", dynamic_cast<Context*>(result.get()));
        ContextVerifier(*dynamic_cast<Context*>(result.get()), a("12. get")).verifyString("ALGORITHM", "FLAK");
    }
}

/* Test interface function for normal VCR file stored at offset.
   A: Create file with some padding followed by classic VCR content, open it, call VcrFile(#fd).
   E: Content successfully loaded. File pointer matches expectation. */
AFL_TEST("game.interface.VcrFileFunction:IFVcrFile:offset", a)
{
    static const uint8_t PREFIX[] = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };

    // Environment
    Environment env;
    addRoot(env);
    addShipList(env);
    {
        Ref<Stream> f = env.fs.openFile("/test.dat", FileSystem::Create);
        f->fullWrite(PREFIX);
        f->fullWrite(VCR_FILE);
        f->fullWrite(PREFIX);
    }

    // Open the file
    env.session.world().fileTable().openFile(7, env.fs.openFile("/test.dat", FileSystem::OpenRead));
    env.session.world().fileTable().getFile(7)->setPos(sizeof(PREFIX));

    // Do it
    Segment seg0;
    seg0.pushBackInteger(7);
    Arguments args0(seg0, 0, 1);
    std::auto_ptr<afl::data::Value> result0(game::interface::IFVcrFile(env.session, args0));
    IndexableValue* iv = dynamic_cast<IndexableValue*>(result0.get());

    // Verify
    a.checkNonNull("01. result", result0.get());
    a.checkNonNull("02. indexable", iv);
    a.checkEqual("03. dim", iv->getDimension(1), 3U);
    a.checkEqual("04. fpos", env.session.world().fileTable().getFile(7)->getPos(), sizeof(PREFIX) + sizeof(VCR_FILE));

    // Verify content
    {
        Segment seg;
        seg.pushBackInteger(2);
        Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> result(iv->get(args));
        a.checkNonNull("11. get", dynamic_cast<Context*>(result.get()));
        ContextVerifier(*dynamic_cast<Context*>(result.get()), a("12. get")).verifyString("ALGORITHM", "PHost 4");
    }
}

/* Test interface function for empty VCR file.
   A: Create file containing just a zero, open it, call VcrFile(#fd).
   E: Call succeeds and returns null */
AFL_TEST("game.interface.VcrFileFunction:IFVcrFile:normal:empty", a)
{
    // Empty VCR file (from game.vcr.classic.Database:load:phost4)
    static const uint8_t FILE[] = {
        0x00, 0x00
    };

    // Environment
    Environment env;
    addRoot(env);
    addShipList(env);
    env.fs.openFile("/test.dat", FileSystem::Create)
        ->fullWrite(FILE);

    // Open the file
    env.session.world().fileTable().openFile(7, env.fs.openFile("/test.dat", FileSystem::OpenRead));

    // Do it
    Segment seg0;
    seg0.pushBackInteger(7);
    Arguments args0(seg0, 0, 1);
    std::auto_ptr<afl::data::Value> result0(game::interface::IFVcrFile(env.session, args0));

    // Verify
    a.checkNull("01. result", result0.get());
}

/* Test interface function for empty FLAK file.
   A: Create file containing a FLAK header and count=0, open it, call VcrFile(#fd).
   E: Call succeeds and returns null */
AFL_TEST("game.interface.VcrFileFunction:IFVcrFile:flak:empty", a)
{
    // Empty FLAK file
    static const uint8_t FILE[] = {
        0x46, 0x4c, 0x41, 0x4b, 0x56, 0x43, 0x52, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x30, 0x30, 0x2d, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3a, 0x30, 0x30, 0x3a,
        0x30, 0x30, 0x00, 0x00, 0x00, 0x00,
    };

    // Environment
    Environment env;
    addRoot(env);
    addShipList(env);
    env.fs.openFile("/test.dat", FileSystem::Create)
        ->fullWrite(FILE);

    // Open the file
    env.session.world().fileTable().openFile(7, env.fs.openFile("/test.dat", FileSystem::OpenRead));

    // Do it
    Segment seg0;
    seg0.pushBackInteger(7);
    Arguments args0(seg0, 0, 1);
    std::auto_ptr<afl::data::Value> result0(game::interface::IFVcrFile(env.session, args0));

    // Verify
    a.checkNull("01. result", result0.get());
}

/* Error case: null fd.
   A: Call VcrFile(null).
   E: Result is null. */
AFL_TEST("game.interface.VcrFileFunction:IFVcrFile:error:null", a)
{
    // Environment
    Environment env;
    addRoot(env);
    addShipList(env);

    // Do it
    Segment seg0;
    seg0.pushBackNew(0);
    Arguments args0(seg0, 0, 1);
    std::auto_ptr<afl::data::Value> result0(game::interface::IFVcrFile(env.session, args0));

    // Verify
    a.checkNull("01. result", result0.get());
}

/* Error case: truncated file.
   A: Create file containing an incomplete FLAK header, open it, call VcrFile(#fd).
   E: Call fails */
AFL_TEST("game.interface.VcrFileFunction:IFVcrFile:error:truncate", a)
{
    // Empty FLAK file
    static const uint8_t FILE[] = {
        0x46, 0x4c, 0x41, 0x4b, 0x56, 0x43,
    };

    // Environment
    Environment env;
    addRoot(env);
    addShipList(env);
    env.fs.openFile("/test.dat", FileSystem::Create)
        ->fullWrite(FILE);

    // Open the file
    env.session.world().fileTable().openFile(7, env.fs.openFile("/test.dat", FileSystem::OpenRead));

    // Do it
    Segment seg0;
    seg0.pushBackInteger(7);
    Arguments args0(seg0, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFVcrFile(env.session, args0), std::runtime_error);
}

/* Error case: no root.
   A: Create file with classic VCR content, open it, call VcrFile(#fd) without a Root in the session.
   E: Call fails */
AFL_TEST("game.interface.VcrFileFunction:IFVcrFile:error:no-root", a)
{
    // Environment
    Environment env;
    addShipList(env);
    env.fs.openFile("/test.dat", FileSystem::Create)
        ->fullWrite(VCR_FILE);

    // Open the file
    env.session.world().fileTable().openFile(7, env.fs.openFile("/test.dat", FileSystem::OpenRead));

    // Do it
    Segment seg0;
    seg0.pushBackInteger(7);
    Arguments args0(seg0, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFVcrFile(env.session, args0), std::exception);
}

/* Error case: file not open.
   A: Call VcrFile(#fd) with a non-open file handle.
   E: Call fails */
AFL_TEST("game.interface.VcrFileFunction:IFVcrFile:error:not-open", a)
{
    // Environment
    Environment env;
    addRoot(env);
    addShipList(env);

    // Do it
    Segment seg0;
    seg0.pushBackInteger(7);
    Arguments args0(seg0, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFVcrFile(env.session, args0), std::exception);
}

/* Error case: arity error.
   A: Call VcrFile(1,2,3).
   E: Call fails */
AFL_TEST("game.interface.VcrFileFunction:IFVcrFile:error:arity", a)
{
    // Environment
    Environment env;
    addRoot(env);
    addShipList(env);

    // Do it
    Segment seg0;
    seg0.pushBackInteger(1);
    seg0.pushBackInteger(2);
    seg0.pushBackInteger(3);
    Arguments args0(seg0, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFVcrFile(env.session, args0), std::exception);
}

/* Error case: type.
   A: Call VcrFile("X").
   E: Call fails */
AFL_TEST("game.interface.VcrFileFunction:IFVcrFile:error:type", a)
{
    // Environment
    Environment env;
    addRoot(env);
    addShipList(env);

    // Do it
    Segment seg0;
    seg0.pushBackString("X");
    Arguments args0(seg0, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFVcrFile(env.session, args0), std::exception);
}
