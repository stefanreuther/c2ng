/**
  *  \file interpreter/simplefunction.cpp
  *  \brief Template class interpreter::SimpleFunction
  */

#include "interpreter/simplefunction.hpp"

afl::data::Value*
interpreter::SimpleFunction<void>::get(Arguments& args)
{
    return m_get != 0
        ? m_get(args)
        : 0;
}

interpreter::FunctionValue*
interpreter::SimpleFunction<void>::clone() const
{
    return new SimpleFunction(m_get);
}
