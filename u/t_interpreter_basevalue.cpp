/**
  *  \file u/t_interpreter_basevalue.cpp
  *  \brief Test for interpreter::BaseValue
  */

#include "interpreter/basevalue.hpp"

#include "t_interpreter.hpp"
#include "afl/data/visitor.hpp"
#include "afl/io/nullstream.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"
#include "interpreter/error.hpp"

/** Simple test.
    This is mostly an interface test. */
void
TestInterpreterBaseValue::testIt()
{
    // Tester:
    class Tester : public interpreter::BaseValue {
     public:
        virtual String_t toString(bool /*readable*/) const
            { return "Tester"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
        virtual Tester* clone() const
            { return new Tester(); }
    };
    Tester t;

    // Verify visit():
    class Visitor : public afl::data::Visitor {
     public:
        virtual void visitString(const String_t& /*str*/)
            { TS_FAIL("visitString unexpected"); }
        virtual void visitInteger(int32_t /*iv*/)
            { TS_FAIL("visitInteger unexpected"); }
        virtual void visitFloat(double /*fv*/)
            { TS_FAIL("visitFloat unexpected"); }
        virtual void visitBoolean(bool /*bv*/)
            { TS_FAIL("visitBoolean unexpected"); }
        virtual void visitHash(const afl::data::Hash& /*hv*/)
            { TS_FAIL("visitHash unexpected"); }
        virtual void visitVector(const afl::data::Vector& /*vv*/)
            { TS_FAIL("visitVector unexpected"); }
        virtual void visitOther(const afl::data::Value& /*other*/)
            { }
        virtual void visitError(const String_t& /*source*/, const String_t& /*str*/)
            { TS_FAIL("visitError unexpected"); }
        virtual void visitNull()
            { TS_FAIL("visitNull unexpected"); }
    };
    Visitor v;
    v.visit(&t);

    // Verify store()/rejectStore():
    interpreter::TagNode out;
    afl::io::NullStream aux;
    interpreter::vmio::NullSaveContext ctx;
    TS_ASSERT_THROWS(t.store(out, aux, ctx), interpreter::Error);
}
