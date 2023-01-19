/**
  *  \file interpreter/context.hpp
  *  \brief Class interpreter::Context
  */
#ifndef C2NG_INTERPRETER_CONTEXT_HPP
#define C2NG_INTERPRETER_CONTEXT_HPP

#include "afl/data/namequery.hpp"
#include "game/map/object.hpp"
#include "interpreter/basevalue.hpp"

namespace interpreter {

    class PropertyAcceptor;
    class Process;

    /** Context for name lookup.
        A context provides a means for looking up and dealing with local names, and possibly iteration through objects. */
    class Context : public BaseValue {
     public:
        /** Index for a property. */
        typedef size_t PropertyIndex_t;

        /** Property accessor.
            Used as return value from lookup().
            Not intended to control lifetime of objects. */
        class PropertyAccessor {
         public:
            /** Set value by its index.
                \param index Property index
                \param value New value. The parameter is owned by the caller; this function must copy it if needed. */
            virtual void set(PropertyIndex_t index, const afl::data::Value* value) = 0;

            /** Get value by its index.
                The returned value must be newly allocated, caller assumes responsibility. */
            virtual afl::data::Value* get(PropertyIndex_t index) = 0;
        };

        /** Read-only property accessor.
            Implements set() by refusing the call. */
        class ReadOnlyAccessor : public PropertyAccessor {
         public:
            virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        };


        /** Look up a symbol by its name.
            \param name [in] Name query
            \param result [out] On success, property index
            \return non-null PropertyAccessor if found, null on failure.

            BEWARE/LEGACY: do not implement this function using co-variant return types.
            This will cause g++-3.4 to miscompile this code (it fails to adjust null pointers). */
        virtual PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result) = 0;

        /** Advance to next object.
            Return true on success, false on failure. */
        virtual bool next() = 0;

        /** Clone this context. */
        virtual Context* clone() const = 0;

        /** Get associated game object.
            This is used for information purposes, and for type switches in various GUI function bindings.
            This may return null if this context is not associated with a game object. */
        virtual game::map::Object* getObject() = 0;

        /** Enumerate properties. Call acceptor.addProperty for every property.
            \param acceptor Acceptor object */
        virtual void enumProperties(PropertyAcceptor& acceptor) const = 0;

        /** Context has been entered on a process ("With" statement).
            \param proc Process
            \throw Error reject entering the context */
        virtual void onContextEntered(Process& proc) = 0;

        /** Context has been left on a process ("EndWith" statement). */
        virtual void onContextLeft() = 0;
    };

}

#endif
