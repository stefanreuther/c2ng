/**
  *  \file interpreter/hashvalue.cpp
  *  \brief Class interpreter::HashValue
  */

#include "interpreter/hashvalue.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/savecontext.hpp"
#include "interpreter/simplecontext.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/values.hpp"

namespace {
    /** Context for iterating a hash. */
    class HashContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        HashContext(afl::data::Hash::Ref_t data)
            : m_data(data),
              m_slot(0)
            { }
        ~HashContext()
            { }

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                // ex IntHashContext::lookup
                if (name.match("KEY")) {
                    /* @q Key:Str (Hash Element Property)
                       The key of this hash element. */
                    result = 0;
                    return this;
                } else if (name.match("VALUE")) {
                    /* @q Value:Any (Hash Element Property)
                       The value of this hash element.
                       @assignable */
                    result = 1;
                    return this;
                } else {
                    return 0;
                }
            }

        virtual void set(PropertyIndex_t index, const afl::data::Value* value)
            {
                // ex IntHashContext::set
                if (index == 1) {
                    m_data->setValueByIndex(m_slot, value);
                } else {
                    throw interpreter::Error::notAssignable();
                }
            }

        virtual afl::data::Value* get(PropertyIndex_t index)
            {
                // ex IntHashContext::get
                if (index == 0) {
                    return interpreter::makeStringValue(m_data->getKeys().getNameByIndex(m_slot));
                } else {
                    return afl::data::Value::cloneOf(m_data->getValueByIndex(m_slot));
                }
            }

        virtual bool next()
            {
                // ex IntHashContext::next
                if (m_slot+1 < m_data->getKeys().getNumNames()) {
                    ++m_slot;
                    return true;
                } else {
                    return false;
                }
            }

        virtual afl::base::Deletable* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const
            {
                acceptor.addProperty("KEY", interpreter::thString);
                acceptor.addProperty("VALUE", interpreter::thNone);
            }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "#<hashIterator>"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }

        // Value:
        HashContext* clone() const
            { return new HashContext(*this); }

     private:
        afl::data::Hash::Ref_t m_data;
        afl::data::NameMap::Index_t m_slot;
    };
}

// Constructor.
interpreter::HashValue::HashValue(afl::data::Hash::Ref_t data)
    : m_data(data)
{ }

// Destructor.
interpreter::HashValue::~HashValue()
{ }

// Access underlying actual hash.
afl::data::Hash::Ref_t
interpreter::HashValue::getData()
{
    // ex IntHash::getData
    return m_data;
}

// IndexableValue:
afl::data::Value*
interpreter::HashValue::get(Arguments& args)
{
    // ex IntHash::get
    String_t key;
    args.checkArgumentCount(1);
    if (!checkStringArg(key, args.getNext())) {
        return 0;
    }
    return afl::data::Value::cloneOf(m_data->get(key));
}

void
interpreter::HashValue::set(Arguments& args, const afl::data::Value* value)
{
    // ex IntHash::set
    String_t key;
    args.checkArgumentCount(1);
    if (!checkStringArg(key, args.getNext())) {
        throw Error::notAssignable();
    }
    m_data->setNew(key, afl::data::Value::cloneOf(value));
}

// CallableValue:
size_t
interpreter::HashValue::getDimension(size_t /*which*/) const
{
    // ex IntHash::getDimension
    return 0;
}

interpreter::Context*
interpreter::HashValue::makeFirstContext()
{
    // ex IntHash::makeFirstContext
    if (m_data->getKeys().getNumNames() == 0) {
        return 0;
    } else {
        return new HashContext(m_data);
    }
}


// BaseValue:
String_t
interpreter::HashValue::toString(bool /*readable*/) const
{
    return "#<hash>";
}

void
interpreter::HashValue::store(TagNode& out, afl::io::DataSink& /*aux*/, SaveContext& ctx) const
{
    // ex IntHash::store
    out.tag = TagNode::Tag_Hash;
    out.value = ctx.addHash(*m_data);
}

// Value:
interpreter::HashValue*
interpreter::HashValue::clone() const
{
    return new HashValue(m_data);
}

