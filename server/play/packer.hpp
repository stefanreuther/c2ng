/**
  *  \file server/play/packer.hpp
  *  \brief Interface server::play::Packer
  */
#ifndef C2NG_SERVER_PLAY_PACKER_HPP
#define C2NG_SERVER_PLAY_PACKER_HPP

#include "afl/base/deletable.hpp"
#include "afl/data/hash.hpp"
#include "afl/string/string.hpp"
#include "interpreter/context.hpp"
#include "server/types.hpp"

namespace server { namespace play {

    /** Interface for an object to transfer.
        Queries and commands produce a list of results, represented as a Packer descendant. */
    class Packer : public afl::base::Deletable {
        // ex ServerObjectWriter, ServerQueryWriter
     public:
        /** Build value.
            Called after all actions have been performed; should build the result from the now-current game status.
            \return newly-allocated value */
        virtual Value_t* buildValue() const = 0;

        /** Get name.
            Used as the hash key in the result sent to the client, and also for duplicate removal.

            Naming conventions:
            - "objN" (e.g. "ship10") if this is an object with Id
            - "obj" (e.g. "beam") if this is an array indexed by Id
            - "zobj" (e.g. "zmine") if this is an array NOT indexed by Id

            \return name */
        virtual String_t getName() const = 0;

        /** Fetch value from Context and add to Hash.
            This is a utility function to build the result.
            Use it for creating a c2play binding from a script binding.

            \param hv         [in/out] Hash to update
            \param ctx        [in] Context object
            \param scriptName [in] Name of property in ctx to look up. It is an error if this property does not exist.
            \param jsonName   [in] Name of property in hv to create */
        static void addValue(afl::data::Hash& hv, interpreter::Context& ctx, const char* scriptName, const char* jsonName);

        /** Add new value to Hash.
            This is a utility function to build the result.

            We do not send null values, so this is just a wrapper around Hash::setNew that filters nulls.

            \param hv         [in/out] Hash to update
            \param value      [in] Newly-allocated value
            \param jsonName   [in] Name of property in hv to create */
        static void addValueNew(afl::data::Hash& hv, Value_t* value, const char* jsonName);

        /** Flatten a value for serialisation as JSON.
            JSON cannot publish most of our structured values.
            This converts those that are used in the built-in interface to types that can
            (afl::data::HashValue, afl::data::VectorValue).

            Takes ownership of the parameter, and returns newly-allocated value.
            If value can be used as-is, takes the shortcut of returning it as-is.
            Otherwise, allocates new value and frees old one.

            \param value Value
            \return Result */
        static Value_t* flattenNew(Value_t* value);
    };

} }

#endif
