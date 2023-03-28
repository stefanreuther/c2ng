/**
  *  \file game/interface/ufocontext.hpp
  *  \brief Class game::interface::UfoContext
  */
#ifndef C2NG_GAME_INTERFACE_UFOCONTEXT_HPP
#define C2NG_GAME_INTERFACE_UFOCONTEXT_HPP

#include "game/map/ufo.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Ufo context.
        Publishes properties and methods of an Ufo.
        Implements the result of the "Ufo()" function.

        @see UfoFunction */
    class UfoContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        /** Constructor.
            @param slot     Slot; see UfoType::getObjectByIndex()
            @param turn     Turn
            @param session  Session (for translator, InterpreterInterface) */
        UfoContext(Id_t slot, afl::base::Ref<Turn> turn, Session& session);

        /** Destructor. */
        ~UfoContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual UfoContext* clone() const;
        virtual game::map::Ufo* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Id_t m_slot;
        afl::base::Ref<Turn> m_turn;
        Session& m_session;
    };

} }

#endif
