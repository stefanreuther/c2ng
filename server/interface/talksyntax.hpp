/**
  *  \file server/interface/talksyntax.hpp
  *  \brief Interface server::interface::TalkSyntax
  */
#ifndef C2NG_SERVER_INTERFACE_TALKSYNTAX_HPP
#define C2NG_SERVER_INTERFACE_TALKSYNTAX_HPP

#include "afl/string/string.hpp"
#include "afl/base/memory.hpp"
#include "afl/data/vector.hpp"
#include "afl/base/ref.hpp"

namespace server { namespace interface {

    /** Interface for syntax-table inquiry (SYNTAX commands). */
    class TalkSyntax : public afl::base::Deletable {
     public:
        /** Get single syntax entry.
            \param key Key to query
            \return Value
            \throw std::exception if key not found */
        virtual String_t get(String_t key) = 0;

        /** Get multiple syntax entries.
            \param keys Keys to query
            \return Array of results. Each entry is either a string (if found) or null. */
        // FIXME: Can we find a better signature that does not deal with afl::data values?
        virtual afl::base::Ref<afl::data::Vector> mget(afl::base::Memory<const String_t> keys) = 0;
    };

} }

#endif
