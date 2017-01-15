/**
  *  \file game/interface/simpleprocedure.hpp
  */
#ifndef C2NG_GAME_INTERFACE_SIMPLEPROCEDURE_HPP
#define C2NG_GAME_INTERFACE_SIMPLEPROCEDURE_HPP

#include "interpreter/procedurevalue.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    class SimpleProcedure : public interpreter::ProcedureValue {
     public:
        typedef void (*Function_t)(interpreter::Process&, Session&, interpreter::Arguments&);

        SimpleProcedure(Session& session, Function_t func);
        ~SimpleProcedure();

        // ProcedureValue:
        virtual void call(interpreter::Process& proc, interpreter::Arguments& args);
        virtual SimpleProcedure* clone() const;

     private:
        Session& m_session;
        Function_t m_function;
    };

} }

#endif
