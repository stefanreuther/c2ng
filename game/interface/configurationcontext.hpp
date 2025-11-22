/**
  *  \file game/interface/configurationcontext.hpp
  *  \brief Class game::interface::ConfigurationContext
  */
#ifndef C2NG_GAME_INTERFACE_CONFIGURATIONCONTEXT_HPP
#define C2NG_GAME_INTERFACE_CONFIGURATIONCONTEXT_HPP

#include "game/config/configuration.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/singlecontext.hpp"

namespace game { namespace interface {

    /** Context to access a Configuration object.
        This implements the Configuration() function and the System.Cfg, System.Pref properties. */
    class ConfigurationContext : public interpreter::SingleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** State data. */
        struct Data {
            Session& session;
            const afl::base::Ref<game::config::Configuration> config;

            Data(Session& session, const afl::base::Ref<game::config::Configuration>& config)
                : session(session), config(config)
                { }
        };

        /** Constructor.
            @param session   Session (for live viewpoint-player)
            @param config    Configuration object */
        ConfigurationContext(Session& session, const afl::base::Ref<game::config::Configuration>& config);

        /** Destructor. */
        ~ConfigurationContext();

        /** Access contained Configuration object.
            @return Configuration object */
        game::config::Configuration& config();

        // ReadOnlyAccessor:
        virtual afl::data::Value* get(PropertyIndex_t index);

        // Context:
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual ConfigurationContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Check parameter type.
            @param value Value received from user
            @return ConfigurationContext or null
            @throw interpreter::Error if value is not a ConfigurationContext */
        static ConfigurationContext* check(afl::data::Value* value);

     private:
        class KeyContext;
        class EntryFunction;

        Data m_data;
    };

    // Configuration() factory function
    afl::data::Value* IFConfiguration(Session& session, interpreter::Arguments& args);

    // Commands to invoke on a ConfigurationContext
    void IFConfiguration_Add(const ConfigurationContext::Data& state, interpreter::Process& proc, interpreter::Arguments& args);
    void IFConfiguration_Create(const ConfigurationContext::Data& state, interpreter::Process& proc, interpreter::Arguments& args);
    void IFConfiguration_Load(const ConfigurationContext::Data& state, interpreter::Process& proc, interpreter::Arguments& args);
    void IFConfiguration_Merge(const ConfigurationContext::Data& state, interpreter::Process& proc, interpreter::Arguments& args);
    void IFConfiguration_Subtract(const ConfigurationContext::Data& state, interpreter::Process& proc, interpreter::Arguments& args);

    // Functions to invoke on a ConfigurationContext
    afl::data::Value* IFConfiguration_Get(const ConfigurationContext::Data& state, interpreter::Arguments& args);

} }

#endif
