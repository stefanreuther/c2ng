/**
  *  \file u/t_interpreter_procedurevalue.cpp
  *  \brief Test for interpreter::ProcedureValue
  */

#include <memory>
#include "interpreter/procedurevalue.hpp"

#include "t_interpreter.hpp"
#include "interpreter/error.hpp"
#include "afl/io/internalsink.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/sys/log.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "interpreter/world.hpp"
#include "interpreter/process.hpp"

/** Interface test. */
void
TestInterpreterProcedureValue::testIt()
{
    class Tester : public interpreter::ProcedureValue {
     public:
        Tester(size_t n)
            : m_n(n)
            { }
        virtual void call(interpreter::Process& /*proc*/, interpreter::Arguments& args)
            {
                TS_ASSERT_EQUALS(args.getNumArgs(), m_n);
            }
        virtual Tester* clone() const
            { return new Tester(m_n); }
     private:
        size_t m_n;
    };
    Tester t(3);

    // Test normal methods
    // - toString: because it has no readable form, both are identical
    TS_ASSERT_EQUALS(t.toString(true), t.toString(false));
    TS_ASSERT_EQUALS(t.toString(true).substr(0, 2), "#<");

    // - isProcedureCall
    TS_ASSERT(t.isProcedureCall());

    // - getDimension
    TS_ASSERT_EQUALS(t.getDimension(0), 0);
    TS_ASSERT_EQUALS(t.getDimension(1), 0);

    // - makeFirstContext
    TS_ASSERT_THROWS(t.makeFirstContext(), interpreter::Error);

    // - store
    {
        interpreter::TagNode node;
        afl::io::InternalSink sink;
        afl::charset::Utf8Charset cs;
        interpreter::vmio::NullSaveContext ctx;
        TS_ASSERT_THROWS(t.store(node, sink, cs, ctx), interpreter::Error);
    }

    // - clone
    interpreter::ProcedureValue& pv = t;
    interpreter::CallableValue& cv = t;
    std::auto_ptr<interpreter::ProcedureValue> clone(pv.clone());
    TS_ASSERT(clone.get() != 0);
    TS_ASSERT(clone.get() != &pv);
    TS_ASSERT(dynamic_cast<Tester*>(clone.get()) != 0);

    // Test invocation
    {
        afl::sys::Log log;
        afl::io::NullFileSystem fs;
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackNew(0);
        seg.pushBackNew(0);

        interpreter::World world(log, fs);
        interpreter::Process proc(world, "TestInterpreterProcedureValue::testIt", 999);

        TS_ASSERT_EQUALS(proc.getStackSize(), 0U);
        cv.call(proc, seg, false);
        TS_ASSERT_EQUALS(proc.getStackSize(), 0U);
        cv.call(proc, seg, true);
        TS_ASSERT_EQUALS(proc.getStackSize(), 1U);
        TS_ASSERT(proc.getResult() == 0);
    }
}
