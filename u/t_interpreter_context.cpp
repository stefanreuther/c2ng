/**
  *  \file u/t_interpreter_context.cpp
  *  \brief Test for interpreter::Context
  */

#include "interpreter/context.hpp"

#include "t_interpreter.hpp"

/** Interface test. */
void
TestInterpreterContext::testIt()
{
    class Tester : public interpreter::Context {
     public:
        virtual Tester* lookup(const afl::data::NameQuery& /*name*/, PropertyIndex_t& /*result*/)
            { return 0; }
        virtual void set(PropertyIndex_t /*index*/, afl::data::Value* /*value*/)
            { }
        virtual afl::data::Value* get(PropertyIndex_t /*index*/)
            { return 0; }
        virtual bool next()
            { return false; }
        virtual Context* clone() const
            { return 0; }
        virtual game::map::Object* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/)
            { }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            { }
    };
    Tester t;
}

