/**
  *  \file game/interface/minefieldcontext.hpp
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

    class MinefieldContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        MinefieldContext(int id, afl::base::Ref<Root> root, afl::base::Ref<Game> game, afl::string::Translator& tx);
        ~MinefieldContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual MinefieldContext* clone() const;
        virtual game::map::Minefield* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        static MinefieldContext* create(int id, Session& session, bool force);

     private:
        int m_id;
        afl::base::Ref<Root> m_root;
        afl::base::Ref<Game> m_game;
        afl::string::Translator& m_translator;
    };

} }

#endif
