/**
  *  \file client/si/widgetfunctionvalue.hpp
  */
#ifndef C2NG_CLIENT_SI_WIDGETFUNCTIONVALUE_HPP
#define C2NG_CLIENT_SI_WIDGETFUNCTIONVALUE_HPP

#include "afl/base/ref.hpp"
#include "afl/base/weaklink.hpp"
#include "client/si/widgetfunction.hpp"
#include "client/si/widgetreference.hpp"
#include "game/session.hpp"
#include "interpreter/functionvalue.hpp"

namespace client { namespace si {

    class ScriptSide;

    class WidgetFunctionValue : public interpreter::FunctionValue {
     public:
        WidgetFunctionValue(WidgetFunction func, game::Session& session, ScriptSide* ss, const WidgetReference& ref);

        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual WidgetFunctionValue* clone() const;

     private:
        const WidgetFunction m_function;
        game::Session& m_session;
        const afl::base::WeakLink<ScriptSide> m_pScriptSide;
        const WidgetReference m_ref;
    };


} }

#endif
