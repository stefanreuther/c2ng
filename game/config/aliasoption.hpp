/**
  *  \file game/config/aliasoption.hpp
  *  \brief Class game::config::CostArrayOption
  */
#ifndef C2NG_GAME_CONFIG_ALIASOPTION_HPP
#define C2NG_GAME_CONFIG_ALIASOPTION_HPP

#include "game/config/configurationoption.hpp"
#include "game/config/configuration.hpp"
#include "afl/string/string.hpp"

namespace game { namespace config {

    /** Alias option.
        This simply forwards all requests to another option. */
    class AliasOption : public ConfigurationOption {
     public:
        AliasOption(Configuration& container, String_t forwardedOptionName);

        virtual ~AliasOption();
        virtual void set(String_t value);
        virtual String_t toString() const;

        ConfigurationOption* getForwardedOption() const;

     private:
        Configuration& m_container;
        String_t m_forwardedOptionName;
    };

    struct AliasOptionDescriptor {
        const char* m_name;
        const char* m_forwardedOptionName;

        // Meta-information
        typedef AliasOption OptionType_t;
        OptionType_t* create(Configuration& config) const
            { return new OptionType_t(config, m_forwardedOptionName); }
    };
    
} }

#endif
