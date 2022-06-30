/**
  *  \file client/si/listboxfunction.hpp
  */
#ifndef C2NG_CLIENT_SI_LISTBOXFUNCTION_HPP
#define C2NG_CLIENT_SI_LISTBOXFUNCTION_HPP

#include "afl/base/weaklink.hpp"
#include "game/session.hpp"
#include "interpreter/functionvalue.hpp"

namespace client { namespace si {

    class ScriptSide;
    class WidgetHolder;

    class ListboxFunction : public interpreter::FunctionValue {
     public:
        ListboxFunction(game::Session& session, ScriptSide* pScriptSide);

        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual ListboxFunction* clone() const;

     private:
        game::Session& m_session;
        afl::base::WeakLink<ScriptSide> m_pScriptSide;
    };

} }

#endif
