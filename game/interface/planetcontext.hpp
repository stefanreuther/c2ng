/**
  *  \file game/interface/planetcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_PLANETCONTEXT_HPP
#define C2NG_GAME_INTERFACE_PLANETCONTEXT_HPP

#include "interpreter/context.hpp"
#include "afl/base/ref.hpp"
#include "game/root.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/map/planet.hpp"

namespace game { namespace interface {

    class PlanetContext : public interpreter::Context, public interpreter::Context::PropertyAccessor {
     public:
        PlanetContext(int id,
                      Session& session,
                      afl::base::Ref<Root> root,
                      afl::base::Ref<Game> game);
        ~PlanetContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual PlanetContext* clone() const;
        virtual game::map::Planet* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

        static PlanetContext* create(int id, Session& session);

     private:
        Id_t m_id;
        Session& m_session;
        afl::base::Ref<Root> m_root;
        afl::base::Ref<Game> m_game;
    };

} }

#endif
