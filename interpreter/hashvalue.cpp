/**
  *  \file interpreter/hashvalue.cpp
  */

#include "interpreter/hashvalue.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"
#include "interpreter/error.hpp"
#include "interpreter/context.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/typehint.hpp"

namespace {
    /** Context for iterating a hash. */
    class HashContext : public interpreter::Context {
     public:
        HashContext(afl::base::Ptr<interpreter::HashData> data)
            : m_data(data),
              m_slot(0)
            { }
        ~HashContext()
            { }

        // Context:
        virtual bool lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                // ex IntHashContext::lookup
                if (name.match("KEY")) {
                    /* @q Key:Str (Hash Element Property)
                       The key of this hash element. */
                    result = 0;
                    return true;
                } else if (name.match("VALUE")) {
                    /* @q Value:Any (Hash Element Property)
                       The value of this hash element.
                       @assignable */
                    result = 1;
                    return true;
                } else {
                    return false;
                }
            }

        virtual void set(PropertyIndex_t index, afl::data::Value* value)
            {
                // ex IntHashContext::set
                if (index == 1) {
                    m_data->setNew(m_slot, afl::data::Value::cloneOf(value));
                } else {
                    throw interpreter::Error::notAssignable();
                }
            }

        virtual afl::data::Value* get(PropertyIndex_t index)
            {
                // ex IntHashContext::get
                if (index == 0) {
                    return interpreter::makeStringValue(m_data->getName(m_slot));
                } else {
                    return afl::data::Value::cloneOf(m_data->get(m_slot));
                }
            }

        virtual bool next()
            {
                // ex IntHashContext::next
                if (m_slot+1 < m_data->getNumNames()) {
                    ++m_slot;
                    return true;
                } else {
                    return false;
                }
            }

        virtual game::map::Object* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor)
            {
                acceptor.addProperty("KEY", interpreter::thString);
                acceptor.addProperty("VALUE", interpreter::thNone);
            }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "#<hashIterator>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext* /*ctx*/) const
            { throw interpreter::Error::notSerializable(); }

        // Value:
        HashContext* clone() const
            { return new HashContext(*this); }

     private:
        afl::base::Ptr<interpreter::HashData> m_data;
        afl::data::NameMap::Index_t m_slot;
    };
}

interpreter::HashValue::HashValue(afl::base::Ptr<HashData> data)
    : m_data(data)
{ }

interpreter::HashValue::~HashValue()
{ }

afl::base::Ptr<interpreter::HashData>
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
interpreter::HashValue::set(Arguments& args, afl::data::Value* value)
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
int32_t
interpreter::HashValue::getDimension(int32_t /*which*/)
{
    // ex IntHash::getDimension
    return 0;
}

interpreter::Context*
interpreter::HashValue::makeFirstContext()
{
    // ex IntHash::makeFirstContext
    if (m_data->getNumNames() == 0) {
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
interpreter::HashValue::store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext* ctx) const
{
    // ex IntHash::store
    // FIXME: port this (store)
    // IntVMSaveContext* vsc = IntVMSaveContext::getCurrentInstance();
    // if (vsc != 0) {
    //     tag.tag   = IntTagNode::Tag_Hash;
    //     tag.value = vsc->addHash(*data);
    // } else {
    (void) out;
    (void) aux;
    (void) cs;
    (void) ctx;
    throw Error::notSerializable();
    // }
}

// Value:
interpreter::HashValue*
interpreter::HashValue::clone() const
{
    return new HashValue(m_data);
}

