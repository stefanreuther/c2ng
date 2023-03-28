/**
  *  \file game/interface/shipcontext.hpp
  *  \brief Class game::interface::ShipContext
  */
#ifndef C2NG_GAME_INTERFACE_SHIPCONTEXT_HPP
#define C2NG_GAME_INTERFACE_SHIPCONTEXT_HPP

#include "afl/base/ref.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Ship context.
        Implements the result of the "Ship()" function.
        Publishes properties of a ship.
        To create, usually use ShipContext::create();

        @see ShipFunction */
    class ShipContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        /** Constructor.
            @param id        Ship Id
            @param session   Session (for translator)
            @param root      Root
            @param game      Game
            @param shipList  Ship list */
        ShipContext(Id_t id,
                    Session& session,
                    afl::base::Ref<Root> root,
                    afl::base::Ref<Game> game,
                    afl::base::Ref<const game::spec::ShipList> shipList);

        /** Destructor. */
        ~ShipContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual ShipContext* clone() const;
        virtual game::map::Ship* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Create ShipContext.
            @param id        Ship Id
            @param session   Session
            @return newly-allocated ShipContext; null if preconditions not satisfied */
        static ShipContext* create(Id_t id, Session& session);

     private:
        Id_t m_id;
        Session& m_session;
        afl::base::Ref<Root> m_root;
        afl::base::Ref<Game> m_game;
        afl::base::Ref<const game::spec::ShipList> m_shipList;
    };

} }

#endif
