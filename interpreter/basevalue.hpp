/**
  *  \file interpreter/basevalue.hpp
  *  \brief Base class interpreter::BaseValue
  */
#ifndef C2NG_INTERPRETER_BASEVALUE_HPP
#define C2NG_INTERPRETER_BASEVALUE_HPP

#include "afl/data/value.hpp"
#include "afl/string/string.hpp"
#include "interpreter/tagnode.hpp"
#include "afl/io/datasink.hpp"

namespace interpreter {

    class SaveContext;

    /** Base interpreter value.
        While we use afl::data elements for regular values (integer, float, etc.),
        all our own values derive from this one to add some useful methods.

        To pass around values, we still use afl::data::Value pointers (and afl::data::Segment, etc.), never BaseValue.
        dynamic_cast to BaseValue is used to determine whether a value has our added methods.
        Other types are identified regularily using afl::data::Visitor. */
    class BaseValue : public afl::data::Value {
     public:
        /** Convert to string.
            \param readable true to (try to) produce a readable/parseable representation, false to make it user-readable
            \return result */
        virtual String_t toString(bool readable) const = 0;

        /** Store data for serialization.
            \param out [out] Tag node
            \param aux [out] Auxiliary data
            \param ctx [in,optional] Context to save structured data.
                   Serialization may fail if this object represents structured data and no context is given.
            \throw Error if object cannot be serialized */
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const = 0;

        // afl::data::Value:
        virtual void visit(afl::data::Visitor& visitor) const;
    };

}

#endif
