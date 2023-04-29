/**
  *  \file u/t_interpreter_directoryfunctions.cpp
  *  \brief Test for interpreter::DirectoryFunctions
  */

#include "interpreter/directoryfunctions.hpp"

#include "t_interpreter.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/assert.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/world.hpp"

namespace {
    struct Environment {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::InternalFileSystem fs;
        interpreter::World world;

        Environment()
            : log(), tx(), fs(), world(log, tx, fs)
            { }
    };

    interpreter::IndexableValue& lookupFunction(Environment& env, String_t name)
    {
        afl::test::Assert a(name);
        afl::data::Value* v = env.world.getGlobalValue(name.c_str());
        interpreter::IndexableValue* iv = dynamic_cast<interpreter::IndexableValue*>(v);
        a.check("iv != 0", iv != 0);

        interpreter::test::ValueVerifier verif(*iv, a);
        verif.verifyBasics();
        verif.verifyNotSerializable();
        return *iv;
    }
}

/** Test normal operation.
    A: create directory with content. Call 'DirectoryEntry("/dir")'. Examine result by simulating 'ForEach'.
    E: all directory entries returned with correct content. */
void
TestInterpreterDirectoryFunctions::testDirectoryEntryNormal()
{
    Environment env;
    env.fs.createDirectory("/dir");
    env.fs.createDirectory("/dir/a");
    env.fs.openFile("/dir/b", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("xyz"));

    interpreter::IndexableValue& iv = lookupFunction(env, "DIRECTORYENTRY");
    TS_ASSERT(!iv.isProcedureCall());

    // Invoke it, producing a Callable
    afl::data::Segment seg;
    seg.pushBackString("/dir");
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<afl::data::Value> result(iv.get(args));
    interpreter::CallableValue* calla = dynamic_cast<interpreter::CallableValue*>(result.get());
    TS_ASSERT(calla != 0);

    interpreter::test::ValueVerifier callaVerif(*calla, "DirectoryEntry(/dir)");
    callaVerif.verifyBasics();
    callaVerif.verifyNotSerializable();
    TS_ASSERT_EQUALS(calla->getDimension(0), 0);
    TS_ASSERT(!calla->isProcedureCall());

    // Verify content
    std::auto_ptr<interpreter::Context> ctx(calla->makeFirstContext());
    TS_ASSERT(ctx.get() != 0);

    interpreter::test::ContextVerifier verif(*ctx, "DirectoryEntry(/dir) context");
    verif.verifyTypes();
    verif.verifyBasics();
    verif.verifyNotSerializable();
    TS_ASSERT(ctx->getObject() == 0);

    verif.verifyString("NAME", "a");
    verif.verifyString("PATH", "/dir/a");
    verif.verifyNull("SIZE");
    verif.verifyString("TYPE", "d");
    TS_ASSERT(ctx->next());

    verif.verifyString("NAME", "b");
    verif.verifyString("PATH", "/dir/b");
    verif.verifyInteger("SIZE", 3);
    verif.verifyString("TYPE", "f");
    TS_ASSERT(!ctx->next());
}

/** Test operation on emptry directory.
    A: create directory without content. Call 'DirectoryEntry("/dir")'. Examine result.
    E: empty result (null context) returned. */
void
TestInterpreterDirectoryFunctions::testDirectoryEntryEmpty()
{
    Environment env;
    env.fs.createDirectory("/dir");

    interpreter::IndexableValue& iv = lookupFunction(env, "DIRECTORYENTRY");

    // Invoke it, producing a Callable
    afl::data::Segment seg;
    seg.pushBackString("/dir");
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<afl::data::Value> result(iv.get(args));
    interpreter::CallableValue* calla = dynamic_cast<interpreter::CallableValue*>(result.get());
    TS_ASSERT(calla != 0);

    // Verify content
    std::auto_ptr<interpreter::Context> ctx(calla->makeFirstContext());
    TS_ASSERT(ctx.get() == 0);
}

/** Test invocation with null directory name.
    A: Call 'DirectoryEntry(Z(0))'.
    E: null context returned */
void
TestInterpreterDirectoryFunctions::testDirectoryEntryNull()
{
    Environment env;
    env.fs.createDirectory("/dir");

    interpreter::IndexableValue& iv = lookupFunction(env, "DIRECTORYENTRY");

    // Invoke it with null, producing null
    afl::data::Segment seg;
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<afl::data::Value> result(iv.get(args));
    TS_ASSERT(result.get() == 0);
}

/** Test invocation with nonexistant directory.
    A: Call 'DirectoryEntry("/dir")' for a nonexistant directory.
    E: Iteration fails eventually. */
void
TestInterpreterDirectoryFunctions::testDirectoryEntryNonExistant()
{
    Environment env;
    interpreter::IndexableValue& iv = lookupFunction(env, "DIRECTORYENTRY");

    // Invoke it. It is unspecified whether it fails on "get()", or on "makeFirstContext()".
    afl::data::Segment seg;
    seg.pushBackString("/dir");
    interpreter::Arguments args(seg, 0, 1);
    try {
        std::auto_ptr<afl::data::Value> result(iv.get(args));
        interpreter::CallableValue* calla = dynamic_cast<interpreter::CallableValue*>(result.get());
        TS_ASSERT(calla != 0);
        TS_ASSERT_THROWS(calla->makeFirstContext(), afl::except::FileProblemException);
    }
    catch (afl::except::FileProblemException&)
    { }
}

/** Test invocation with wrong arity.
    A: Call 'DirectoryEntry()'.
    E: Error. */
void
TestInterpreterDirectoryFunctions::testDirectoryEntryArityError()
{
    Environment env;
    env.fs.createDirectory("/dir");

    interpreter::IndexableValue& iv = lookupFunction(env, "DIRECTORYENTRY");

    // Invoke it, wrong arity
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    TS_ASSERT_THROWS(iv.get(args), interpreter::Error);
}

/** Test bad invocation as procedure.
    A: Call the result of 'DirectoryEntry()' as a procedure.
    E: Error. */
void
TestInterpreterDirectoryFunctions::testDirectoryEntryCallResult()
{
    Environment env;
    env.fs.createDirectory("/dir");
    env.fs.createDirectory("/dir/a");
    env.fs.openFile("/dir/b", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("xyz"));

    interpreter::IndexableValue& iv = lookupFunction(env, "DIRECTORYENTRY");
    TS_ASSERT(!iv.isProcedureCall());

    // Invoke it, producing a Callable
    afl::data::Segment seg;
    seg.pushBackString("/dir");
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<afl::data::Value> result(iv.get(args));
    interpreter::CallableValue* calla = dynamic_cast<interpreter::CallableValue*>(result.get());
    TS_ASSERT(calla != 0);

    // Try to call it
    interpreter::Process proc(env.world, "testDirectoryEntryCallResult", 777);
    afl::data::Segment callArgs;
    TS_ASSERT_THROWS(calla->call(proc, callArgs, false), interpreter::Error);
}

