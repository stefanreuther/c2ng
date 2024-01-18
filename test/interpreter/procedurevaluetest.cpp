/**
  *  \file test/interpreter/procedurevaluetest.cpp
  *  \brief Test for interpreter::ProcedureValue
  */

#include "interpreter/procedurevalue.hpp"

#include "afl/io/internalsink.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/world.hpp"
#include <memory>

/** Interface test. */
AFL_TEST("interpreter.ProcedureValue", a)
{
    class Tester : public interpreter::ProcedureValue {
     public:
        Tester(afl::test::Assert a, size_t n)
            : m_assert(a), m_n(n)
            { }
        virtual void call(interpreter::Process& /*proc*/, interpreter::Arguments& args)
            { m_assert.checkEqual("getNumArgs", args.getNumArgs(), m_n); }
        virtual Tester* clone() const
            { return new Tester(m_assert, m_n); }
     private:
        afl::test::Assert m_assert;
        size_t m_n;
    };
    Tester t(a("Tester"), 3);

    // Test normal methods
    // - toString: because it has no readable form, both are identical
    a.checkEqual(". toString11", t.toString(true), t.toString(false));
    a.checkEqual(". toString12", t.toString(true).substr(0, 2), "#<");

    // - isProcedureCall
    a.check("21. isProcedureCall", t.isProcedureCall());

    // - getDimension
    a.checkEqual("31. getDimension", t.getDimension(0), 0);
    a.checkEqual("32. getDimension", t.getDimension(1), 0);

    // - makeFirstContext
    AFL_CHECK_THROWS(a("41. makeFirstContext"), t.makeFirstContext(), interpreter::Error);

    // - store
    interpreter::test::ValueVerifier(t, a).verifyNotSerializable();

    // - clone
    interpreter::ProcedureValue& pv = t;
    interpreter::CallableValue& cv = t;
    std::auto_ptr<interpreter::ProcedureValue> clone(pv.clone());
    a.check("51. clone", clone.get() != 0);
    a.check("52. clone", clone.get() != &pv);
    a.check("53. clone", dynamic_cast<Tester*>(clone.get()) != 0);

    // Test invocation
    {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackNew(0);
        seg.pushBackNew(0);

        interpreter::World world(log, tx, fs);
        interpreter::Process proc(world, "TestInterpreterProcedureValue::testIt", 999);

        a.checkEqual("61. getStackSize", proc.getStackSize(), 0U);
        cv.call(proc, seg, false);
        a.checkEqual("62. getStackSize", proc.getStackSize(), 0U);
        cv.call(proc, seg, true);
        a.checkEqual("63. getStackSize", proc.getStackSize(), 1U);
        a.checkNull("64. getResult", proc.getResult());
    }
}
