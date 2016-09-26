/**
  *  \file game/interface/simplefunction.hpp
  */
#ifndef C2NG_GAME_INTERFACE_SIMPLEFUNCTION_HPP
#define C2NG_GAME_INTERFACE_SIMPLEFUNCTION_HPP

#include "interpreter/indexablevalue.hpp"
#include "afl/data/value.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    class SimpleFunction : public interpreter::IndexableValue {
     public:
        typedef afl::data::Value* (*Function_t)(Session&, interpreter::Arguments&);

        SimpleFunction(Session& session, Function_t func);
        ~SimpleFunction();

        // IndexableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which);
        virtual interpreter::Context* makeFirstContext();
        virtual SimpleFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext* ctx) const;

     private:
        Session& m_session;
        Function_t m_function;
    };

} }

#endif
