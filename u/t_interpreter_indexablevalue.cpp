/**
  *  \file u/t_interpreter_indexablevalue.cpp
  *  \brief Test for interpreter::IndexableValue
  */

#include "interpreter/indexablevalue.hpp"

#include "t_interpreter.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

/** Interface test. */
void
TestInterpreterIndexableValue::testIt()
{
    class Tester : public interpreter::IndexableValue {
     public:
        virtual afl::data::Value* get(interpreter::Arguments& args)
            { return interpreter::makeIntegerValue(int(args.getNumArgs())); }
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value)
            { rejectSet(args, value); }

        virtual int32_t getDimension(int32_t /*which*/) const
            { return 0; }
        virtual interpreter::Context* makeFirstContext()
            { return 0; }
        virtual CallableValue* clone() const
            { return new Tester(); }
        virtual String_t toString(bool /*readable*/) const
            { TS_FAIL("toString unexpected"); return ""; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
    };
    Tester t;

    // Verify:
    // - isProcedureCall
    TS_ASSERT_EQUALS(t.isProcedureCall(), false);

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
    TS_ASSERT(iv != 0);
    TS_ASSERT_EQUALS(iv->getValue(), 3);

    // - set/rejectSet()
    afl::data::Segment setSeg;
    callSeg.pushBackInteger(7);
    callSeg.pushBackInteger(8);
    callSeg.pushBackInteger(9);
    interpreter::Arguments setArgs(setSeg, 0, 3);
    afl::data::IntegerValue setValue(42);

    TS_ASSERT_THROWS(t.set(setArgs, &setValue), interpreter::Error);
}

