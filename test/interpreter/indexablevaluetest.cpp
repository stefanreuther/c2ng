/**
  *  \file test/interpreter/indexablevaluetest.cpp
  *  \brief Test for interpreter::IndexableValue
  */

#include "interpreter/indexablevalue.hpp"

#include <stdexcept>
#include "afl/data/integervalue.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

/** Interface test. */
AFL_TEST("interpreter.IndexableValue", a)
{
    class Tester : public interpreter::IndexableValue {
     public:
        virtual afl::data::Value* get(interpreter::Arguments& args)
            { return interpreter::makeIntegerValue(int(args.getNumArgs())); }
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value)
            { rejectSet(args, value); }

        virtual size_t getDimension(size_t /*which*/) const
            { return 0; }
        virtual interpreter::Context* makeFirstContext()
            { return 0; }
        virtual CallableValue* clone() const
            { return new Tester(); }
        virtual String_t toString(bool /*readable*/) const
            { throw std::runtime_error("toString unexpected"); }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
    };
    Tester t;

    // Verify:
    // - isProcedureCall
    a.checkEqual("01. isProcedureCall", t.isProcedureCall(), false);

    // - call
    afl::data::Segment callSeg;
    callSeg.pushBackInteger(7);
    callSeg.pushBackInteger(8);
    callSeg.pushBackInteger(9);

    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::Process proc(world, "testIt", 99);

    t.call(proc, callSeg, true);

    const afl::data::IntegerValue* iv = dynamic_cast<const afl::data::IntegerValue*>(proc.getResult());
    a.checkNonNull("11. IntegerValue", iv);
    a.checkEqual("12. getValue", iv->getValue(), 3);

    // - set/rejectSet()
    afl::data::Segment setSeg;
    callSeg.pushBackInteger(7);
    callSeg.pushBackInteger(8);
    callSeg.pushBackInteger(9);
    interpreter::Arguments setArgs(setSeg, 0, 3);
    afl::data::IntegerValue setValue(42);

    AFL_CHECK_THROWS(a("21. set"), t.set(setArgs, &setValue), interpreter::Error);
}
