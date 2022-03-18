/**
  *  \file interpreter/contextprovider.hpp
  *  \brief Interface interpreter::ContextProvider
  */
#ifndef C2NG_INTERPRETER_CONTEXTPROVIDER_HPP
#define C2NG_INTERPRETER_CONTEXTPROVIDER_HPP

#include "afl/data/namequery.hpp"
#include "afl/base/deletable.hpp"
#include "interpreter/context.hpp"

namespace interpreter {

    /** Context provider.
        This is a minimized version of the interface of Context to reduce dependencies in StatementCompilationContext. */
    class ContextProvider : public afl::base::Deletable {
     public:
        /** Look up a symbol by its name.
            \param [in]  q     Name query
            \param [out] index On success, property index
            \return non-null PropertyAccessor if found, null on failure. */
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& q, Context::PropertyIndex_t& index) = 0;
    };

}

#endif
