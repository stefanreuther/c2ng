/**
  *  \file interpreter/staticcontext.hpp
  *  \brief Interface interpreter::StaticContext
  */
#ifndef C2NG_INTERPRETER_STATICCONTEXT_HPP
#define C2NG_INTERPRETER_STATICCONTEXT_HPP

#include "afl/data/namequery.hpp"
#include "afl/base/deletable.hpp"
#include "interpreter/context.hpp"

namespace interpreter {

    /** Static context.
        This is a minimized version of the interface of Context that produces a static compilation context to StatementCompilationContext.
        It has a smaller interface than Context itself. */
    class StaticContext : public afl::base::Deletable {
     public:
        /** Look up a symbol by its name.
            \param [in]  q     Name query
            \param [out] index On success, property index
            \return non-null PropertyAccessor if found, null on failure. */
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& q, Context::PropertyIndex_t& index) const = 0;
    };

}

#endif
