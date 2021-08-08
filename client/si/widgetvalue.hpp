/**
  *  \file client/si/widgetvalue.hpp
  */
#ifndef C2NG_CLIENT_SI_WIDGETVALUE_HPP
#define C2NG_CLIENT_SI_WIDGETVALUE_HPP

#include "interpreter/singlecontext.hpp"
#include "client/si/widgetreference.hpp"

namespace client { namespace si {

    class WidgetValue : public interpreter::SingleContext {
     public:
        explicit WidgetValue(const WidgetReference& ref);

        // Methods to be implemented by child classes:
        // virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result) = 0;
        // virtual Context* clone() const = 0;
        // virtual void enumProperties(PropertyAcceptor& acceptor) = 0;

        virtual game::map::Object* getObject();
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

        const WidgetReference& getValue() const;

     private:
        const WidgetReference m_ref;
    };

} }

#endif
