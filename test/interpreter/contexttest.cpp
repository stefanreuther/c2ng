/**
  *  \file test/interpreter/contexttest.cpp
  *  \brief Test for interpreter::Context
  */

#include "interpreter/context.hpp"

#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"

/** Interface test: Context. */
AFL_TEST_NOARG("interpreter.Context:interface")
{
    class Tester : public interpreter::Context {
     public:
        virtual PropertyAccessor* lookup(const afl::data::NameQuery& /*name*/, PropertyIndex_t& /*result*/)
            { return 0; }
        virtual bool next()
            { return false; }
        virtual Context* clone() const
            { return 0; }
        virtual afl::base::Deletable* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { }
        virtual void onContextEntered(interpreter::Process& /*proc*/)
            { }
        virtual void onContextLeft()
            { }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { }
    };
    Tester t;
}

/** Interface test: Context::PropertyAccessor. */
AFL_TEST_NOARG("interpreter.Context:PropertyAccessor")
{
    class Tester : public interpreter::Context::PropertyAccessor {
     public:
        virtual void set(interpreter::Context::PropertyIndex_t /*index*/, const afl::data::Value* /*value*/)
            { }
        virtual afl::data::Value* get(interpreter::Context::PropertyIndex_t /*index*/)
            { return 0; }
    };
    Tester t;
}

/** Interface test: Context::ReadOnlyAccessor. */
AFL_TEST("interpreter.Context:ReadOnlyAccessor", a)
{
    class Tester : public interpreter::Context::ReadOnlyAccessor {
     public:
        virtual afl::data::Value* get(interpreter::Context::PropertyIndex_t /*index*/)
            { return 0; }
    };
    Tester t;
    AFL_CHECK_THROWS(a("01. set"), t.set(0, 0), interpreter::Error);
}
