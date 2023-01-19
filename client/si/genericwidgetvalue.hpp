/**
  *  \file client/si/genericwidgetvalue.hpp
  */
#ifndef C2NG_CLIENT_SI_GENERICWIDGETVALUE_HPP
#define C2NG_CLIENT_SI_GENERICWIDGETVALUE_HPP

#include "client/si/widgetvalue.hpp"
#include "interpreter/nametable.hpp"
#include "game/session.hpp"
#include "afl/base/weaklink.hpp"

namespace client { namespace si {

    class ScriptSide;

    enum GenericWidgetDomain {
        WidgetFunctionDomain,
        WidgetCommandDomain,
        WidgetPropertyDomain
    };

    class GenericWidgetValue : public WidgetValue, public interpreter::Context::PropertyAccessor {
     public:
        GenericWidgetValue(afl::base::Memory<const interpreter::NameTable> names, game::Session& session, ScriptSide* ss, const WidgetReference& ref);

        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual GenericWidgetValue* clone() const;
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        afl::base::Memory<const interpreter::NameTable> getNames() const
            { return m_names; }

     private:
        const afl::base::Memory<const interpreter::NameTable> m_names;
        game::Session& m_session;
        const afl::base::WeakLink<ScriptSide> m_pScriptSide;
    };

} }

#endif
