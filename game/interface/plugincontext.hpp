/**
  *  \file game/interface/plugincontext.hpp
  *  \brief Class game::interface::PluginContext
  */
#ifndef C2NG_GAME_INTERFACE_PLUGINCONTEXT_HPP
#define C2NG_GAME_INTERFACE_PLUGINCONTEXT_HPP

#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/singlecontext.hpp"

namespace game { namespace interface {

    /** Plugin context.
        Publishes properties of a plugin, given by name. */
    class PluginContext : public interpreter::SingleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        PluginContext(String_t name, Session& session);
        ~PluginContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual PluginContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        static PluginContext* create(String_t name, Session& session);

     private:
        String_t m_name;
        Session& m_session;
    };

    /** Implementation of System.Plugin().
        @param session Session
        @param args    Arguments */
    afl::data::Value* IFSystemPlugin(Session& session, interpreter::Arguments& args);

} }

#endif
