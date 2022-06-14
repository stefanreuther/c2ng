/**
  *  \file game/interface/iteratorprovider.hpp
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

    class IteratorProvider : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        virtual game::map::ObjectCursor* getCursor() = 0;
        virtual game::map::ObjectType* getType() = 0;
        virtual int getCursorNumber() = 0;
        virtual game::Session& getSession() = 0;
        virtual void store(interpreter::TagNode& out) = 0;
        virtual String_t toString() = 0;
    };

} }

#endif
