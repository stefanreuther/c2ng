/**
  *  \file u/helper/contextverifier.cpp
  *  \brief Context verifier
  */

#include <map>
#include <cxxtest/TestSuite.h>
#include "u/helper/contextverifier.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/values.hpp"

namespace {
    typedef std::map<String_t, interpreter::TypeHint> Map_t;

    class PropertyCollector : public interpreter::PropertyAcceptor {
     public:
        PropertyCollector(Map_t& map)
            : m_data(map)
            { }

        virtual void addProperty(const String_t& name, interpreter::TypeHint th)
            {
                Map_t::iterator it = m_data.find(name);
                TSM_ASSERT_EQUALS(name, it, m_data.end());
                m_data.insert(std::make_pair(name, th));
            }

     private:
        Map_t& m_data;
    };
}

void
verifyTypes(interpreter::Context& ctx)
{
    // Collect all properties
    Map_t map;
    PropertyCollector collector(map);
    ctx.enumProperties(collector);

    // Iterate through properties.
    // Each must successfully look up and resolve to the correct type.
    int numNonNullProperties = 0;
    for (Map_t::iterator it = map.begin(); it != map.end(); ++it) {
        // Look up
        interpreter::Context::PropertyIndex_t index;
        interpreter::Context* foundContext = ctx.lookup(it->first, index);
        TSM_ASSERT(it->first, foundContext != 0);

        // Get. If it's non-null, it must be valid.
        std::auto_ptr<afl::data::Value> value(foundContext->get(index));
        if (value.get() != 0) {
            ++numNonNullProperties;
            switch (it->second) {
             case interpreter::thNone:
                // No constraints
                break;

             case interpreter::thBool:
                TSM_ASSERT(it->first, dynamic_cast<afl::data::BooleanValue*>(value.get()) != 0);
                break;
                
             case interpreter::thInt:
                TSM_ASSERT(it->first, dynamic_cast<afl::data::IntegerValue*>(value.get()) != 0);
                break;

             case interpreter::thFloat:
                TSM_ASSERT(it->first, dynamic_cast<afl::data::FloatValue*>(value.get()) != 0);
                break;

             case interpreter::thString:
                TSM_ASSERT(it->first, dynamic_cast<afl::data::StringValue*>(value.get()) != 0);
                break;
             case interpreter::thProcedure: {
                interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(value.get());
                TSM_ASSERT(it->first, cv != 0);
                TSM_ASSERT(it->first, cv->isProcedureCall());
                break;
             }
             case interpreter::thFunction: {
                interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(value.get());
                TSM_ASSERT(it->first, cv != 0);
                TSM_ASSERT(it->first, !cv->isProcedureCall());
                break;
             }
             case interpreter::thArray:
                TSM_ASSERT(it->first, dynamic_cast<interpreter::IndexableValue*>(value.get()) != 0);
                break;
            }

            // Clone it. Both must have same stringification (otherwise, it's not a clone, right?)
            std::auto_ptr<afl::data::Value> clone(value->clone());
            TS_ASSERT_EQUALS(interpreter::toString(value.get(), false), interpreter::toString(clone.get(), false));
            TS_ASSERT_EQUALS(interpreter::toString(value.get(), true), interpreter::toString(clone.get(), true));
        }
    }

    // Must have a nonzero number of non-null properties to sort out bogus implementations that only return null
    TS_ASSERT(numNonNullProperties > 0);
}

void
verifyInteger(interpreter::Context& ctx, const char* name, int value)
{
    // Look up
    interpreter::Context::PropertyIndex_t index;
    interpreter::Context* foundContext = ctx.lookup(name, index);
    TSM_ASSERT(name, foundContext != 0);

    // Get it
    std::auto_ptr<afl::data::Value> result(foundContext->get(index));
    TSM_ASSERT(name, result.get() != 0);

    afl::data::IntegerValue* iv = dynamic_cast<afl::data::IntegerValue*>(result.get());
    TSM_ASSERT(name, iv != 0);
    TSM_ASSERT_EQUALS(name, iv->getValue(), value);
}

void
verifyBoolean(interpreter::Context& ctx, const char* name, bool value)
{
    // Look up
    interpreter::Context::PropertyIndex_t index;
    interpreter::Context* foundContext = ctx.lookup(name, index);
    TSM_ASSERT(name, foundContext != 0);

    // Get it
    std::auto_ptr<afl::data::Value> result(foundContext->get(index));
    TSM_ASSERT(name, result.get() != 0);

    afl::data::BooleanValue* bv = dynamic_cast<afl::data::BooleanValue*>(result.get());
    TSM_ASSERT(name, bv != 0);
    TSM_ASSERT_EQUALS(name, bv->getValue(), value);
}

void
verifyString(interpreter::Context& ctx, const char* name, const char* value)
{
    // Look up
    interpreter::Context::PropertyIndex_t index;
    interpreter::Context* foundContext = ctx.lookup(name, index);
    TSM_ASSERT(name, foundContext != 0);

    // Get it
    std::auto_ptr<afl::data::Value> result(foundContext->get(index));
    TSM_ASSERT(name, result.get() != 0);

    afl::data::StringValue* sv = dynamic_cast<afl::data::StringValue*>(result.get());
    TSM_ASSERT(name, sv != 0);
    TSM_ASSERT_EQUALS(name, sv->getValue(), value);
}

void
verifyNull(interpreter::Context& ctx, const char* name)
{
    // Look up
    interpreter::Context::PropertyIndex_t index;
    interpreter::Context* foundContext = ctx.lookup(name, index);
    TSM_ASSERT(name, foundContext != 0);

    // Get it
    std::auto_ptr<afl::data::Value> result(foundContext->get(index));
    TSM_ASSERT(name, result.get() == 0);
}
