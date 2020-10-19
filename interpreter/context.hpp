/**
  *  \file interpreter/context.hpp
  */
#ifndef C2NG_INTERPRETER_CONTEXT_HPP
#define C2NG_INTERPRETER_CONTEXT_HPP

#include "game/map/object.hpp"
#include "afl/data/namequery.hpp"
#include "interpreter/basevalue.hpp"

namespace interpreter {

    class PropertyAcceptor;

    /** Context for name lookup.
        A context provides a means for looking up and dealing with local names, and possibly iteration through objects.

        FIXME: it makes sense to split lookup() and set()/get() into different classes now */
    class Context : public BaseValue {
     public:
        /** Index for a property. */
        typedef size_t PropertyIndex_t;

        /** Look up a symbol by its name.
            \param name [in] Name query
            \param result [out] On success, property index
            \return non-null context if found (can be a different one than this one), null on failure. */
        virtual Context* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result) = 0;

        /** Set value by its index.
            \param index Property index
            \param value New value. The parameter is owned by the caller; this function must copy it if needed. */
        virtual void set(PropertyIndex_t index, afl::data::Value* value) = 0;

        /** Get value by its index.
            The returned value must be newly allocated, caller assumes responsibility. */
        virtual afl::data::Value* get(PropertyIndex_t index) = 0;

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
        virtual void enumProperties(PropertyAcceptor& acceptor) = 0;
    };

}

#endif
