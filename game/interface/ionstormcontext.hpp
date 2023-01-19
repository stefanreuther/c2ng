/**
  *  \file game/interface/ionstormcontext.hpp
  *  \brief Class game::interface::IonStormContext
  */
#ifndef C2NG_GAME_INTERFACE_IONSTORMCONTEXT_HPP
#define C2NG_GAME_INTERFACE_IONSTORMCONTEXT_HPP

#include "interpreter/simplecontext.hpp"
#include "game/session.hpp"
#include "game/map/ionstorm.hpp"

namespace game { namespace interface {

    class IonStormContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
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

        static IonStormContext* create(int id, Session& session);

     private:
        int m_id;
        Session& m_session;
        afl::base::Ref<Game> m_game;
    };

} }

#endif
