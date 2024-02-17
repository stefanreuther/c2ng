/**
  *  \file game/interface/planetcontext.hpp
  *  \brief Class game::interface::PlanetContext
  */
#ifndef C2NG_GAME_INTERFACE_PLANETCONTEXT_HPP
#define C2NG_GAME_INTERFACE_PLANETCONTEXT_HPP

#include "afl/base/ref.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Planet context.
        Implements the result of the Planet() function.
        To create, usually use PlanetContext::create().

        @see PlanetFunction */
    class PlanetContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        /** Constructor.
            @param id       Planet Id
            @param session  Session (for translator, ship list)
            @param root     Root; mutable to attach listeners for actions
            @param game     Game
            @param turn     Turn */
        PlanetContext(Id_t id, Session& session, const afl::base::Ref<Root>& root, const afl::base::Ref<Game>& game, const afl::base::Ref<Turn>& turn);

        /** Destructor. */
        ~PlanetContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual PlanetContext* clone() const;
        virtual game::map::Planet* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Create PlanetContext for planet in viewpoint turn.
            @param id       Planet Id
            @param session  Session (for translator, ship list)
            @param g        Game
            @param t        Turn (part of game)
            @return newly-allocated PlanetContext; null if preconditions not satisfied */
        static PlanetContext* create(Id_t id, Session& session, const afl::base::Ref<Game>& g, const afl::base::Ref<Turn>& t);

     private:
        Id_t m_id;
        Session& m_session;
        afl::base::Ref<Root> m_root;
        afl::base::Ref<Game> m_game;
        afl::base::Ref<Turn> m_turn;
    };

} }

#endif
