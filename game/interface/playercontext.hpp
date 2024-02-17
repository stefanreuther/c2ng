/**
  *  \file game/interface/playercontext.hpp
  *  \brief Class game::interface::PlayerContext
  */
#ifndef C2NG_GAME_INTERFACE_PLAYERCONTEXT_HPP
#define C2NG_GAME_INTERFACE_PLAYERCONTEXT_HPP

#include "afl/string/translator.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Player context.
        Implements the result of the Player() function.
        To create, usually use PlayerContext::create().

        @see PlayerFunction */
    class PlayerContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            @param nr    Player number
            @param game  Game (for teams, scores)
            @param root  Root (for host configuration, players)
            @param tx    Translator */
        PlayerContext(int nr, const afl::base::Ref<Game>& game, const afl::base::Ref<Root>& root, afl::string::Translator& tx);

        /** Destructor. */
        ~PlayerContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual PlayerContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Create PlayerContext.
            @param nr      Player number
            @param session Session
            @return newly-allocated PlayerContext; null if preconditions are missing */
        static PlayerContext* create(int nr, Session& session);

     private:
        int m_number;
        afl::base::Ref<Game> m_game;
        afl::base::Ref<Root> m_root;
        afl::string::Translator& m_translator;
    };

} }

#endif
