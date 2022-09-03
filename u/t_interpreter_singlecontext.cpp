/**
  *  \file u/t_interpreter_singlecontext.cpp
  *  \brief Test for interpreter::SingleContext
  */

#include "interpreter/singlecontext.hpp"

#include "t_interpreter.hpp"

/** Interface test: SingleContext. */
void
TestInterpreterSingleContext::testInterface()
{
    class Tester : public interpreter::SingleContext {
     public:
        // Context
        virtual PropertyAccessor* lookup(const afl::data::NameQuery& /*name*/, PropertyIndex_t& /*result*/)
            { return 0; }
        virtual Context* clone() const
            { return 0; }
        virtual game::map::Object* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/)
            { }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { }
    };
    Tester t;

    TS_ASSERT_EQUALS(t.next(), false);
}
