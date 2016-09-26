/**
  *  \file game/interface/simpleprocedure.hpp
  */
#ifndef C2NG_GAME_INTERFACE_SIMPLEPROCEDURE_HPP
#define C2NG_GAME_INTERFACE_SIMPLEPROCEDURE_HPP

#include "interpreter/callablevalue.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    class SimpleProcedure : public interpreter::CallableValue {
     public:
        typedef void (*Function_t)(interpreter::Process&, Session&, interpreter::Arguments&);

        SimpleProcedure(Session& session, Function_t func);
        ~SimpleProcedure();

        // CallableValue:
        virtual void call(interpreter::Process& proc, afl::data::Segment& args, bool want_result);
        virtual bool isProcedureCall();
        virtual int32_t getDimension(int32_t which);
        virtual interpreter::Context* makeFirstContext();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext* ctx) const;

        // Value:
        virtual SimpleProcedure* clone() const;

     private:
        Session& m_session;
        Function_t m_function;
    };

} }

#endif
