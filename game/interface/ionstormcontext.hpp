/**
  *  \file game/interface/ionstormcontext.hpp
  *  \brief Class game::interface::IonStormContext
  */
#ifndef C2NG_GAME_INTERFACE_IONSTORMCONTEXT_HPP
#define C2NG_GAME_INTERFACE_IONSTORMCONTEXT_HPP

#include "game/map/ionstorm.hpp"
#include "game/session.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Ion Storm context.
        Implements the result of enumerating the "IonStorm" function.
        To create, usually use IonStormContext::create().

        @see IonStormFunction */
    class IonStormContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        /** Constructor.
            @param id      Id
            @param session Session
            @param game    Game */
        IonStormContext(int id, Session& session, afl::base::Ref<Game> game);
        ~IonStormContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual IonStormContext* clone() const;
        virtual game::map::IonStorm* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Create IonStormContext.
            @param id      Id
            @param session Session
            @return newly-allocated IonStormContext; null if preconditions are missing (e.g. storm does not exist) */
        static IonStormContext* create(int id, Session& session);

     private:
        int m_id;
        Session& m_session;
        afl::base::Ref<Game> m_game;
    };

} }

#endif
