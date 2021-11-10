/**
  *  \file client/si/dialogfunction.hpp
  */
#ifndef C2NG_CLIENT_SI_DIALOGFUNCTION_HPP
#define C2NG_CLIENT_SI_DIALOGFUNCTION_HPP

#include "interpreter/indexablevalue.hpp"
#include "game/session.hpp"
#include "afl/base/weaklink.hpp"

namespace client { namespace si {

    class ScriptSide;
    class WidgetHolder;

    class DialogFunction : public interpreter::IndexableValue {
     public:
        DialogFunction(game::Session& session, ScriptSide* pScriptSide);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        // IndexableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual DialogFunction* clone() const;

     private:
        game::Session& m_session;
        afl::base::WeakLink<ScriptSide> m_pScriptSide;
    };

} }

#endif
