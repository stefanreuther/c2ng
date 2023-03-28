/**
  *  \file game/interface/enginecontext.hpp
  *  \brief Class game::interface::EngineContext
  */
#ifndef C2NG_GAME_INTERFACE_ENGINECONTEXT_HPP
#define C2NG_GAME_INTERFACE_ENGINECONTEXT_HPP

#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Engine context.
        Implements the result of the Engine() function.
        To create, usually use EngineContext::create().

        @see EngineFunction */
    class EngineContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        /** Constructor.
            @param nr       Engine number
            @param shipList Ship list */
        EngineContext(int nr, afl::base::Ref<game::spec::ShipList> shipList);

        /** Destructor. */
        ~EngineContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual EngineContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Create EngineContext.
            @param nr      Engine number
            @param session Session
            @return newly-allocated EngineContext; null if preconditions are missing */
        static EngineContext* create(int nr, Session& session);

     private:
        int m_number;
        afl::base::Ref<game::spec::ShipList> m_shipList;
    };

} }

#endif
