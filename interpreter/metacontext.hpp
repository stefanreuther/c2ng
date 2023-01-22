/**
  *  \file interpreter/metacontext.hpp
  *  \brief Class interpreter::MetaContext
  */
#ifndef C2NG_INTERPRETER_METACONTEXT_HPP
#define C2NG_INTERPRETER_METACONTEXT_HPP

#include "afl/base/ref.hpp"
#include "afl/base/refcounted.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/simplecontext.hpp"

namespace interpreter {

    /** Meta-context that provides information about the property names of another Context.
        For each property, provides the attributes:
        - ID (running Id starting from 0)
        - NAME
        - TYPE (int, bool, etc.) */
    class MetaContext : public SimpleContext, public Context::ReadOnlyAccessor {
     public:
        /** Create MetaContext.
            @param parent Context to get information about
            @return newly-allocated MetaContext. Null if the given Context has no properties. */
        static MetaContext* create(const Context& parent);

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual MetaContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const;

     private:
        /** Shared bulk data. */
        struct Data : public afl::base::RefCounted, public PropertyAcceptor {
            std::vector<String_t> m_names;
            std::vector<uint8_t> m_types;

            virtual void addProperty(const String_t& name, TypeHint th);
        };

        MetaContext(const afl::base::Ref<Data>& data, size_t pos);

        afl::base::Ref<Data> m_data;
        std::size_t m_position;
    };

}

#endif
