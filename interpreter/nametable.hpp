/**
  *  \file interpreter/nametable.hpp
  */
#ifndef C2NG_INTERPRETER_NAMETABLE_HPP
#define C2NG_INTERPRETER_NAMETABLE_HPP

#include "afl/base/types.hpp"
#include "afl/data/namequery.hpp"
#include "afl/base/memory.hpp"
#include "interpreter/context.hpp"

namespace interpreter {

    /** Name table entry. */
    struct NameTable {
        const char* name;          /**< Name of property. */
        uint16_t    index;         /**< Index of property. */
        uint8_t     domain;        /**< Domain of property. */
        uint8_t     type;          // FIXME: change to TypeHint?
    };

    bool lookupName(const afl::data::NameQuery& name, afl::base::Memory<const NameTable> tab, interpreter::Context::PropertyIndex_t& index);
}

#endif
