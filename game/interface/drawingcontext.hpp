/**
  *  \file game/interface/drawingcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_DRAWINGCONTEXT_HPP
#define C2NG_GAME_INTERFACE_DRAWINGCONTEXT_HPP

#include "interpreter/simplecontext.hpp"
#include "game/turn.hpp"
#include "afl/base/ref.hpp"
#include "game/map/drawingcontainer.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    class DrawingContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        DrawingContext(afl::base::Ref<Turn> turn, afl::base::Ref<Root> root, game::map::DrawingContainer::Iterator_t it);
        ~DrawingContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual DrawingContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        static DrawingContext* create(Session& session);

     private:
        // Turn, to keep the turn object alive
        afl::base::Ref<Turn> m_turn;
        afl::base::Ref<Root> m_root;

        game::map::DrawingContainer::Iterator_t m_iterator;
    };

} }

#endif
