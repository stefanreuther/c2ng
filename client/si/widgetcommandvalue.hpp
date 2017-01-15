/**
  *  \file client/si/widgetcommandvalue.hpp
  */
#ifndef C2NG_CLIENT_SI_WIDGETCOMMANDVALUE_HPP
#define C2NG_CLIENT_SI_WIDGETCOMMANDVALUE_HPP

#include "interpreter/procedurevalue.hpp"
#include "client/si/widgetcommand.hpp"
#include "client/si/widgetreference.hpp"
#include "afl/base/weaklink.hpp"

namespace client { namespace si {

    class ScriptSide;

    class WidgetCommandValue : public interpreter::ProcedureValue {
     public:
        WidgetCommandValue(WidgetCommand cmd, game::Session& session, ScriptSide* ss, const WidgetReference& ref);

        virtual void call(interpreter::Process& proc, interpreter::Arguments& args);
        virtual WidgetCommandValue* clone() const;

     private:
        const WidgetCommand m_command;
        game::Session& m_session;
        const afl::base::WeakLink<ScriptSide> m_pScriptSide;
        const WidgetReference m_ref;
    };


} }

#endif
