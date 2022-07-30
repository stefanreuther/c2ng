/**
  *  \file game/interface/iteratorprovider.hpp
  *  \brief Interface game::interface::IteratorProvider
  */
#ifndef C2NG_GAME_INTERFACE_ITERATORPROVIDER_HPP
#define C2NG_GAME_INTERFACE_ITERATORPROVIDER_HPP

#include "interpreter/tagnode.hpp"
#include "afl/string/string.hpp"
#include "afl/base/deletable.hpp"
#include "afl/base/refcounted.hpp"

namespace game {
    class Session;
}

namespace game { namespace map {
    class ObjectCursor;
    class ObjectType;
} }

namespace game { namespace interface {

    /** Adaptor for IteratorContext.
        Provides information to implement an Iterator object. */
    class IteratorProvider : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        /** Get underlying ObjectCursor.
            The cursor provides a value to the iterator's "Current" property.
            @return Cursor. If return value is null, "Current" will be empty and not assignable */
        virtual game::map::ObjectCursor* getCursor() = 0;

        /** Get underlying ObjectType.
            The object type provides functionality for "Next()", "Previous()", etc.
            @return Object type. If return value is null, the functions will return empty. */
        virtual game::map::ObjectType* getType() = 0;

        /** Get cursor number.
            This function provides a value to the iterator's "Screen" property.
            @return Cursor number. If return value is zero, "Screen" will be empty */
        virtual int getCursorNumber() = 0;

        /** Get session.
            The session is required for creating related objects.
            @return Session */
        virtual game::Session& getSession() = 0;

        /** Implementation of interpreter::BaseValue::store().
            @param [out] out Serialized value
            @throw interpreter::Error if not serializable */
        virtual void store(interpreter::TagNode& out) = 0;

        /** Implementation of interpreter::BaseValue::toString().
            @return stringified value */
        virtual String_t toString() = 0;
    };

} }

#endif
