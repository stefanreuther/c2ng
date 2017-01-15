/**
  *  \file game/interface/plugincontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_PLUGINCONTEXT_HPP
#define C2NG_GAME_INTERFACE_PLUGINCONTEXT_HPP

#include "interpreter/context.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    class PluginContext : public interpreter::Context {
     public:
        PluginContext(String_t name, Session& session);
        ~PluginContext();

        // Context:
        virtual PluginContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual PluginContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

        static PluginContext* create(String_t name, Session& session);

     private:
        String_t m_name;
        Session& m_session;
    };

} }

#endif
