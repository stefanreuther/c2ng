/**
  *  \file interpreter/closure.cpp
  */

#include "interpreter/closure.hpp"
#include "interpreter/error.hpp"

interpreter::Closure::Closure()
    : CallableValue(),
      m_function(),
      m_fixedArgs(new afl::data::Segment())
{ }

interpreter::Closure::Closure(const Closure& other)
    : CallableValue(*this),
      m_function(other.m_function),
      m_fixedArgs(other.m_fixedArgs)
{ }
      
interpreter::Closure::~Closure()
{ }

// Closure:
void
interpreter::Closure::setNewFunction(CallableValue* function)
{
    m_function = function;
}

void
interpreter::Closure::addNewArgument(afl::data::Value* value)
{
    m_fixedArgs->pushBackNew(value);
}

void
interpreter::Closure::addNewArgumentsFrom(afl::data::Segment& seg, size_t nargs)
{
    seg.transferLastTo(nargs, *m_fixedArgs);
}

// CallableValue:
void
interpreter::Closure::call(Process& proc, afl::data::Segment& args, bool want_result)
{
    // ex IntClosure::call
    afl::data::Segment combinedArgs;

    // We must not modify fixed_args
    for (size_t i = 0, n = m_fixedArgs->size(); i < n; ++i) {
        combinedArgs.pushBack((*m_fixedArgs)[i]);
    }

    // We can loot args
    args.transferLastTo(args.size(), combinedArgs);

    // Call function
    m_function->call(proc, combinedArgs, want_result);
}

bool
interpreter::Closure::isProcedureCall() const
{
    // ex IntClosure::isProcedureCall
    return m_function->isProcedureCall();
}

int32_t
interpreter::Closure::getDimension(int32_t which) const
{
    // ex IntClosure::getDimension
    int32_t totalDimensions = m_function->getDimension(0);
    int32_t fixedDimensions = m_fixedArgs->size();
    if (fixedDimensions >= totalDimensions) {
        // All arguments fixed; no way this has any dimensions
        return 0;
    } else {
        // Some dimensions still free
        if (which == 0) {
            // Number of dimensions
            return totalDimensions - fixedDimensions;
        } else if (which + fixedDimensions <= totalDimensions) {
            // Asking for size of an existing dimension
            return m_function->getDimension(which + fixedDimensions);
        } else {
            // Asking for size of a nonexistant dimension
            return 0;
        }
    }
}

interpreter::Context*
interpreter::Closure::makeFirstContext()
{
    // ex IntClosure::makeFirstContext
    // Since we are providing a "slice" of the array, we cannot make a first context.
    // It would have to represent that slice. Therefore, pretend to be not iterable.
    throw Error::typeError(Error::ExpectIterable);
}

// BaseValue:
String_t
interpreter::Closure::toString(bool /*readable*/) const
{
    // ex IntClosure::toString
    return "#<closure>";
}

void
interpreter::Closure::store(TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, SaveContext& /*ctx*/) const
{
    // ex IntClosure::store
    throw Error::notSerializable();
}

// Value:
interpreter::Closure*
interpreter::Closure::clone() const
{
    // ex IntClosure::clone
    return new Closure(*this);
}
