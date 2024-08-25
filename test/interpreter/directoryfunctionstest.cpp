/**
  *  \file test/interpreter/directoryfunctionstest.cpp
  *  \brief Test for interpreter::DirectoryFunctions
  */

#include "interpreter/directoryfunctions.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/assert.hpp"
#include "afl/test/testrunner.hpp"
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

    interpreter::IndexableValue& lookupFunction(afl::test::Assert a, Environment& env, String_t name)
    {
        afl::data::Value* v = env.world.getGlobalValue(name.c_str());
        interpreter::IndexableValue* iv = dynamic_cast<interpreter::IndexableValue*>(v);
        a(name).checkNonNull("iv", iv);

        interpreter::test::ValueVerifier verif(*iv, a);
        verif.verifyBasics();
        verif.verifyNotSerializable();
        return *iv;
    }
}

/** Test normal operation.
    A: create directory with content. Call 'DirectoryEntry("/dir")'. Examine result by simulating 'ForEach'.
    E: all directory entries returned with correct content. */
AFL_TEST("interpreter.DirectoryFunctions:DirectoryEntry:normal", a)
{
    Environment env;
    env.fs.createDirectory("/dir");
    env.fs.createDirectory("/dir/a");
    env.fs.openFile("/dir/b", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("xyz"));

    interpreter::IndexableValue& iv = lookupFunction(a, env, "DIRECTORYENTRY");
    a.check("01. isProcedureCall", !iv.isProcedureCall());

    // Invoke it, producing a Callable
    afl::data::Segment seg;
    seg.pushBackString("/dir");
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<afl::data::Value> result(iv.get(args));
    interpreter::CallableValue* calla = dynamic_cast<interpreter::CallableValue*>(result.get());
    a.checkNonNull("11. CallableValue", calla);

    interpreter::test::ValueVerifier callaVerif(*calla, a("DirectoryEntry(/dir)"));
    callaVerif.verifyBasics();
    callaVerif.verifyNotSerializable();
    a.checkEqual("21. getDimension", calla->getDimension(0), 0U);
    a.check("22. isProcedureCall", !calla->isProcedureCall());

    // Verify content
    std::auto_ptr<interpreter::Context> ctx(calla->makeFirstContext());
    a.checkNonNull("31. ctx", ctx.get());

    interpreter::test::ContextVerifier verif(*ctx, a("DirectoryEntry(/dir) context"));
    verif.verifyTypes();
    verif.verifyBasics();
    verif.verifyNotSerializable();
    a.checkNull("41. getObject", ctx->getObject());

    verif.verifyString("NAME", "a");
    verif.verifyString("PATH", "/dir/a");
    verif.verifyNull("SIZE");
    verif.verifyString("TYPE", "d");
    a.check("51. next", ctx->next());

    verif.verifyString("NAME", "b");
    verif.verifyString("PATH", "/dir/b");
    verif.verifyInteger("SIZE", 3);
    verif.verifyString("TYPE", "f");
    a.check("61. next", !ctx->next());
}

/** Test operation on empty directory.
    A: create directory without content. Call 'DirectoryEntry("/dir")'. Examine result.
    E: empty result (null context) returned. */
AFL_TEST("interpreter.DirectoryFunctions:DirectoryEntry:empty", a)
{
    Environment env;
    env.fs.createDirectory("/dir");

    interpreter::IndexableValue& iv = lookupFunction(a, env, "DIRECTORYENTRY");

    // Invoke it, producing a Callable
    afl::data::Segment seg;
    seg.pushBackString("/dir");
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<afl::data::Value> result(iv.get(args));
    interpreter::CallableValue* calla = dynamic_cast<interpreter::CallableValue*>(result.get());
    a.checkNonNull("01. CallableValue", calla);

    // Verify content
    std::auto_ptr<interpreter::Context> ctx(calla->makeFirstContext());
    a.checkNull("11. ctx", ctx.get());
}

/** Test invocation with null directory name.
    A: Call 'DirectoryEntry(Z(0))'.
    E: null context returned */
AFL_TEST("interpreter.DirectoryFunctions:DirectoryEntry:null", a)
{
    Environment env;
    env.fs.createDirectory("/dir");

    interpreter::IndexableValue& iv = lookupFunction(a, env, "DIRECTORYENTRY");

    // Invoke it with null, producing null
    afl::data::Segment seg;
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<afl::data::Value> result(iv.get(args));
    a.checkNull("01. get", result.get());
}

/** Test invocation with nonexistant directory.
    A: Call 'DirectoryEntry("/dir")' for a nonexistant directory.
    E: Iteration fails eventually. */
AFL_TEST("interpreter.DirectoryFunctions:DirectoryEntry:error:nonexistant", a)
{
    Environment env;
    interpreter::IndexableValue& iv = lookupFunction(a, env, "DIRECTORYENTRY");

    // Invoke it. It is unspecified whether it fails on "get()", or on "makeFirstContext()".
    afl::data::Segment seg;
    seg.pushBackString("/dir");
    interpreter::Arguments args(seg, 0, 1);
    try {
        std::auto_ptr<afl::data::Value> result(iv.get(args));
        interpreter::CallableValue* calla = dynamic_cast<interpreter::CallableValue*>(result.get());
        a.checkNonNull("01. CallableValue", calla);
        AFL_CHECK_THROWS(a("02. makeFirstContext"), calla->makeFirstContext(), afl::except::FileProblemException);
    }
    catch (afl::except::FileProblemException&)
    { }
}

/** Test invocation with wrong arity.
    A: Call 'DirectoryEntry()'.
    E: Error. */
AFL_TEST("interpreter.DirectoryFunctions:DirectoryEntry:error:arity", a)
{
    Environment env;
    env.fs.createDirectory("/dir");

    interpreter::IndexableValue& iv = lookupFunction(a, env, "DIRECTORYENTRY");

    // Invoke it, wrong arity
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, iv.get(args), interpreter::Error);
}

/** Test bad invocation as procedure.
    A: Call the result of 'DirectoryEntry()' as a procedure.
    E: Error. */
AFL_TEST("interpreter.DirectoryFunctions:DirectoryEntry:error:call-as-proc", a)
{
    Environment env;
    env.fs.createDirectory("/dir");
    env.fs.createDirectory("/dir/a");
    env.fs.openFile("/dir/b", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("xyz"));

    interpreter::IndexableValue& iv = lookupFunction(a, env, "DIRECTORYENTRY");
    a.check("01. isProcedureCall", !iv.isProcedureCall());

    // Invoke it, producing a Callable
    afl::data::Segment seg;
    seg.pushBackString("/dir");
    interpreter::Arguments args(seg, 0, 1);
    std::auto_ptr<afl::data::Value> result(iv.get(args));
    interpreter::CallableValue* calla = dynamic_cast<interpreter::CallableValue*>(result.get());
    a.checkNonNull("11. CallableValue", calla);

    // Try to call it
    interpreter::Process proc(env.world, "testDirectoryEntryCallResult", 777);
    afl::data::Segment callArgs;
    AFL_CHECK_THROWS(a("21. call"), calla->call(proc, callArgs, false), interpreter::Error);
}
