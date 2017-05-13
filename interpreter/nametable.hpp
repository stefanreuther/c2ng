/**
  *  \file interpreter/nametable.hpp
  *  \brief Structure interpreter::NameTable
  */
#ifndef C2NG_INTERPRETER_NAMETABLE_HPP
#define C2NG_INTERPRETER_NAMETABLE_HPP

#include "afl/base/types.hpp"
#include "afl/data/namequery.hpp"
#include "afl/base/memory.hpp"
#include "interpreter/context.hpp"

namespace interpreter {

    /** Name table entry.
        Tables of this type are used to define property name mappings in context implementations.
        A names is mapped to
        - an index into a domain
        - a domain
        - a type

        Index and domain are defined by the respective user of the table and have no predefined meaning.
        The typical use case is to have one or more domains
        (e.g. properties of a ship, properties of the ship's hull, properties of the ship's owner)
        and separate index series for each domain.

        The type hint is used for reflection, e.g. in export. */
    struct NameTable {
        const char* name;          /**< Name of property. */
        uint16_t    index;         /**< Index of property. */
        uint8_t     domain;        /**< Domain of property. */
        uint8_t     type;          // FIXME: change to TypeHint?
    };

    /** Look up name in table.
        \param name  [in] Name to find
        \param tab   [in] Table. Must be sorted lexically.
        \param index [out] Index into table (i.e. use tab[index] to access the result)
        \retval true Name was found, \c index was set
        \retval false Name not found */
    bool lookupName(const afl::data::NameQuery& name, afl::base::Memory<const NameTable> tab, interpreter::Context::PropertyIndex_t& index);
}

#endif
