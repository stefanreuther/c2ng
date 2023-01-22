/**
  *  \file interpreter/metacontext.cpp
  *  \brief Class interpreter::MetaContext
  */

#include "interpreter/metacontext.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/values.hpp"

namespace {
    static const interpreter::NameTable meta_mapping[] = {
        { "ID",   0, 0, interpreter::thInt },
        { "NAME", 1, 0, interpreter::thString },
        { "TYPE", 2, 0, interpreter::thString },
    };
}

interpreter::MetaContext*
interpreter::MetaContext::create(const Context& parent)
{
    afl::base::Ref<Data> p(*new Data());
    parent.enumProperties(*p);
    if (p->m_names.empty()) {
        return 0;
    } else {
        return new MetaContext(p, 0);
    }
}

inline
interpreter::MetaContext::MetaContext(const afl::base::Ref<Data>& data, size_t pos)
    : m_data(data),
      m_position(pos)
{ }

interpreter::Context::PropertyAccessor*
interpreter::MetaContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    return lookupName(name, meta_mapping, result) ? this : 0;
}

afl::data::Value*
interpreter::MetaContext::get(PropertyIndex_t index)
{
    if (m_position < m_data->m_names.size()) {
        switch (meta_mapping[index].index) {
         case 0:
            return makeSizeValue(m_position);
         case 1:
            return makeStringValue(m_data->m_names[m_position]);
         case 2:
            switch (m_data->m_types[m_position]) {
             case thNone:
                return makeStringValue("any");
             case thBool:
                return makeStringValue("bool");
             case thInt:
                return makeStringValue("int");
             case thFloat:
                return makeStringValue("float");
             case thString:
                return makeStringValue("string");
             case thProcedure:
                return makeStringValue("procedure");
             case thFunction:
                return makeStringValue("function");
             case thArray:
                return makeStringValue("array");
            }
            break;
        }
    }
    return 0;
}

bool
interpreter::MetaContext::next()
{
    if (m_position+1 < m_data->m_names.size()) {
        ++m_position;
        return true;
    } else {
        return false;
    }
}

interpreter::MetaContext*
interpreter::MetaContext::clone() const
{
    return new MetaContext(m_data, m_position);
}

afl::base::Deletable*
interpreter::MetaContext::getObject()
{
    return 0;
}

void
interpreter::MetaContext::enumProperties(PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(meta_mapping);
}

String_t
interpreter::MetaContext::toString(bool /*readable*/) const
{
    return "#<meta>";
}

void
interpreter::MetaContext::store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

void
interpreter::MetaContext::Data::addProperty(const String_t& name, TypeHint th)
{
    m_names.push_back(name);
    m_types.push_back(th);
}
