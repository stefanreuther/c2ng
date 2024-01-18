/**
  *  \file test/interpreter/basevaluetest.cpp
  *  \brief Test for interpreter::BaseValue
  */

#include "interpreter/basevalue.hpp"

#include <stdexcept>
#include "afl/data/visitor.hpp"
#include "afl/io/nullstream.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"

/** Simple test.
    This is mostly an interface test. */
AFL_TEST("interpreter.BaseValue", a)
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
            { throw std::runtime_error("visitString unexpected"); }
        virtual void visitInteger(int32_t /*iv*/)
            { throw std::runtime_error("visitInteger unexpected"); }
        virtual void visitFloat(double /*fv*/)
            { throw std::runtime_error("visitFloat unexpected"); }
        virtual void visitBoolean(bool /*bv*/)
            { throw std::runtime_error("visitBoolean unexpected"); }
        virtual void visitHash(const afl::data::Hash& /*hv*/)
            { throw std::runtime_error("visitHash unexpected"); }
        virtual void visitVector(const afl::data::Vector& /*vv*/)
            { throw std::runtime_error("visitVector unexpected"); }
        virtual void visitOther(const afl::data::Value& /*other*/)
            { }
        virtual void visitError(const String_t& /*source*/, const String_t& /*str*/)
            { throw std::runtime_error("visitError unexpected"); }
        virtual void visitNull()
            { throw std::runtime_error("visitNull unexpected"); }
    };
    Visitor v;
    v.visit(&t);

    // Verify store()/rejectStore():
    interpreter::TagNode out;
    afl::io::NullStream aux;
    interpreter::vmio::NullSaveContext ctx;
    AFL_CHECK_THROWS(a("01. store"), t.store(out, aux, ctx), interpreter::Error);
}
