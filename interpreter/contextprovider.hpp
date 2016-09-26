/**
  *  \file interpreter/contextprovider.hpp
  */
#ifndef C2NG_INTERPRETER_CONTEXTPROVIDER_HPP
#define C2NG_INTERPRETER_CONTEXTPROVIDER_HPP

#include "afl/data/namequery.hpp"
#include "afl/base/deletable.hpp"
#include "interpreter/context.hpp"

namespace interpreter {

    // base class for IntExecutionContext so that IntStatementCompilationContext need not refer to that.
    class ContextProvider : public afl::base::Deletable {
     public:
        virtual Context* lookup(const afl::data::NameQuery& q, Context::PropertyIndex_t& index) = 0;
    };

}

#endif
