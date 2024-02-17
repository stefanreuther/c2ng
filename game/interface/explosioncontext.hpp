/**
  *  \file game/interface/explosioncontext.hpp
  *  \brief Class game::interface::ExplosionContext
  */
#ifndef C2NG_GAME_INTERFACE_EXPLOSIONCONTEXT_HPP
#define C2NG_GAME_INTERFACE_EXPLOSIONCONTEXT_HPP

#include "afl/string/translator.hpp"
#include "game/map/explosion.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Explosion context.
        Implements the result of enumerating the "Explosion" function.
        To create, usually use ExplosionContext::create().

        @see ExplosionFunction */
    class ExplosionContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            @param id       Id (index into ExplosionType)
            @param turn     Turn
            @param tx       Translator */
        ExplosionContext(Id_t id, const afl::base::Ref<Turn>& turn, afl::string::Translator& tx);

        /** Destructor. */
        ~ExplosionContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual ExplosionContext* clone() const;
        virtual game::map::Explosion* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Create ExplosionContext.
            @param id      Id (index into ExplosionType)
            @param session Session
            @param t       Turn
            @return newly-allocated ExplosionContext; null if preconditions are missing */
        static ExplosionContext* create(Id_t id, Session& session, const afl::base::Ref<Turn>& t);

     private:
        Id_t m_id;
        afl::base::Ref<Turn> m_turn;
        afl::string::Translator& m_translator;
    };

} }

#endif
