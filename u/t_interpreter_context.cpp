/**
  *  \file u/t_interpreter_context.cpp
  *  \brief Test for interpreter::Context
  */

#include "interpreter/context.hpp"

#include "t_interpreter.hpp"
#include "interpreter/error.hpp"

/** Interface test: Context. */
void
TestInterpreterContext::testIt()
{
    class Tester : public interpreter::Context {
     public:
        virtual PropertyAccessor* lookup(const afl::data::NameQuery& /*name*/, PropertyIndex_t& /*result*/)
            { return 0; }
        virtual bool next()
            { return false; }
        virtual Context* clone() const
            { return 0; }
        virtual game::map::Object* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/)
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
void
TestInterpreterContext::testPropertyAccessor()
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
void
TestInterpreterContext::testReadOnlyAccessor()
{
    class Tester : public interpreter::Context::ReadOnlyAccessor {
     public:
        virtual afl::data::Value* get(interpreter::Context::PropertyIndex_t /*index*/)
            { return 0; }
    };
    Tester t;
    TS_ASSERT_THROWS(t.set(0, 0), interpreter::Error);
}

