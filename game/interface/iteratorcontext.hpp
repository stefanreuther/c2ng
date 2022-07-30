/**
  *  \file game/interface/iteratorcontext.hpp
  *  \brief Class game::interface::IteratorContext
  */
#ifndef C2NG_GAME_INTERFACE_ITERATORCONTEXT_HPP
#define C2NG_GAME_INTERFACE_ITERATORCONTEXT_HPP

#include "afl/base/ref.hpp"
#include "game/game.hpp"
#include "game/interface/iteratorprovider.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/context.hpp"
#include "interpreter/singlecontext.hpp"

namespace game {
    class Session;
}

namespace game { namespace interface {

    /** Iterator context.
        Implements the "Iterator()" and "UI.Iterator" objects. */
    class IteratorContext : public interpreter::SingleContext, public interpreter::Context::PropertyAccessor {
     public:
        /** Constructor.
            @param provider IteratorProvider */
        explicit IteratorContext(afl::base::Ref<IteratorProvider> provider);

        /** Destructor. */
        ~IteratorContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual IteratorContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        const afl::base::Ref<IteratorProvider> m_provider;
    };


    /** Implementation of the "Iterator" function.
        @param session Session
        @param args    Parameters */
    afl::data::Value* IFIterator(game::Session& session, interpreter::Arguments& args);

    /** Make iterator for a screen number.
        Essentially, this is the implementation behind the Iterator() function.
        @param session Session
        @param nr      Screen number; see game::map::Cursors.
        @return newly-allocated Context object; null if @c nr is out of range or preconditions missing */
    interpreter::Context* makeIteratorValue(Session& session, int nr);

    /** Create object context, given an object.
        @param obj Object
        @param session Session
        @return newly-allocated Context object; null if object is unknown or preconditions not satisfied */
    interpreter::Context* createObjectContext(game::map::Object* obj, Session& session);

} }

#endif
