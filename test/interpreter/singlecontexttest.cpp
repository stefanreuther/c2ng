/**
  *  \file test/interpreter/singlecontexttest.cpp
  *  \brief Test for interpreter::SingleContext
  */

#include "interpreter/singlecontext.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test: SingleContext. */
AFL_TEST("interpreter.SingleContext", a)
{
    class Tester : public interpreter::SingleContext {
     public:
        // Context
        virtual PropertyAccessor* lookup(const afl::data::NameQuery& /*name*/, PropertyIndex_t& /*result*/)
            { return 0; }
        virtual Context* clone() const
            { return 0; }
        virtual afl::base::Deletable* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { }
    };
    Tester t;

    a.checkEqual("01. next", t.next(), false);
}
