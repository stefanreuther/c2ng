/**
  *  \file client/si/widgetfunctionvalue.hpp
  */
#ifndef C2NG_CLIENT_SI_WIDGETFUNCTIONVALUE_HPP
#define C2NG_CLIENT_SI_WIDGETFUNCTIONVALUE_HPP

#include "interpreter/indexablevalue.hpp"
#include "client/si/widgetfunction.hpp"
#include "afl/base/weaklink.hpp"
#include "afl/base/ref.hpp"
#include "game/session.hpp"
#include "client/si/widgetreference.hpp"

namespace client { namespace si {

    class ScriptSide;

    class WidgetFunctionValue : public interpreter::IndexableValue {
     public:
        WidgetFunctionValue(WidgetFunction func, game::Session& session, ScriptSide* ss, const WidgetReference& ref);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

        // CallableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual WidgetFunctionValue* clone() const;

     private:
        const WidgetFunction m_function;
        game::Session& m_session;
        const afl::base::WeakLink<ScriptSide> m_pScriptSide;
        const WidgetReference m_ref;
    };


} }

#endif
