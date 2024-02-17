/**
  *  \file game/interface/minefieldcontext.hpp
  *  \brief Class game::interface::MinefieldContext
  */
#ifndef C2NG_GAME_INTERFACE_MINEFIELDCONTEXT_HPP
#define C2NG_GAME_INTERFACE_MINEFIELDCONTEXT_HPP

#include "afl/string/translator.hpp"
#include "game/game.hpp"
#include "game/map/minefield.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Minefield context.
        Publishes content of a minefield.
        This implements the return value of the "Minefield()" function.
        Normally, use MinefieldContext::create() to create.

        @see MinefieldFunction */
    class MinefieldContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        /** Constructor.
            @param id   Id
            @param root Root (for player names)
            @param game Game (for player properties)
            @param turn Turn (for minefield)
            @param tx   Translator */
        MinefieldContext(Id_t id, const afl::base::Ref<const Root>& root, const afl::base::Ref<Game>& game, const afl::base::Ref<Turn>& turn, afl::string::Translator& tx);

        /** Destructor. */
        ~MinefieldContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual MinefieldContext* clone() const;
        virtual game::map::Minefield* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Create MinefieldContext.
            @param id      Id
            @param session Session
            @param force   true to force creation even if minefield does not currently exist
            @param g       Game
            @param t       Turn (part of game)
            @return newly-allocated MinefieldContext; null if preconditions not fulfilled */
        static MinefieldContext* create(int id, Session& session, const afl::base::Ref<Game>& g, const afl::base::Ref<Turn>& t, bool force);

     private:
        Id_t m_id;
        afl::base::Ref<const Root> m_root;
        afl::base::Ref<Game> m_game;
        afl::base::Ref<Turn> m_turn;
        afl::string::Translator& m_translator;
    };

} }

#endif
