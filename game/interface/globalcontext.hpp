/**
  *  \file game/interface/globalcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_GLOBALCONTEXT_HPP
#define C2NG_GAME_INTERFACE_GLOBALCONTEXT_HPP

#include "interpreter/context.hpp"
#include "game/session.hpp"
#include "interpreter/singlecontext.hpp"

namespace game { namespace interface {

    class GlobalContext : public interpreter::SingleContext {
     public:
        explicit GlobalContext(Session& session);
        ~GlobalContext();

        // Context:
        virtual GlobalContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual GlobalContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
