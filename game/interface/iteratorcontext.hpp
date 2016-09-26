/**
  *  \file game/interface/iteratorcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_ITERATORCONTEXT_HPP
#define C2NG_GAME_INTERFACE_ITERATORCONTEXT_HPP

#include "afl/base/ptr.hpp"
#include "game/game.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/context.hpp"
#include "interpreter/singlecontext.hpp"
#include "game/interface/iteratorprovider.hpp"

namespace game {
    class Session;
}

namespace game { namespace interface {

    class IteratorContext : public interpreter::SingleContext {
     public:
        IteratorContext(afl::base::Ptr<IteratorProvider> provider);
        ~IteratorContext();

        // Context:
        virtual bool lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual IteratorContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext* ctx) const;

     private:
        afl::base::Ptr<IteratorProvider> m_provider;
    };

    afl::data::Value* IFIterator(game::Session& session, interpreter::Arguments& args);

    afl::data::Value* makeIteratorValue(afl::base::Ptr<Game> game, int nr);

} }

#endif
