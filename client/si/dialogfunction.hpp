/**
  *  \file client/si/dialogfunction.hpp
  */
#ifndef C2NG_CLIENT_SI_DIALOGFUNCTION_HPP
#define C2NG_CLIENT_SI_DIALOGFUNCTION_HPP

#include "afl/base/weaklink.hpp"
#include "game/session.hpp"
#include "interpreter/functionvalue.hpp"

namespace client { namespace si {

    class ScriptSide;
    class WidgetHolder;

    class DialogFunction : public interpreter::FunctionValue {
     public:
        DialogFunction(game::Session& session, ScriptSide* pScriptSide);

        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual DialogFunction* clone() const;

     private:
        game::Session& m_session;
        afl::base::WeakLink<ScriptSide> m_pScriptSide;
    };

} }

#endif
