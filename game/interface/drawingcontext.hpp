/**
  *  \file game/interface/drawingcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_DRAWINGCONTEXT_HPP
#define C2NG_GAME_INTERFACE_DRAWINGCONTEXT_HPP

#include "interpreter/context.hpp"
#include "game/turn.hpp"
#include "afl/base/ref.hpp"
#include "game/map/drawingcontainer.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    class DrawingContext : public interpreter::Context {
     public:
        DrawingContext(afl::base::Ref<Turn> turn, game::map::DrawingContainer::Iterator_t it);
        ~DrawingContext();

        // Context:
        virtual DrawingContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual DrawingContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

        static DrawingContext* create(Session& session);

     private:
        // Turn, to keep the turn object alive
        afl::base::Ref<Turn> m_turn;

        game::map::DrawingContainer::Iterator_t m_iterator;
    };

} }

#endif
