/**
  *  \file game/config/aliasoption.hpp
  *  \brief Class game::config::AliasOption
  */
#ifndef C2NG_GAME_CONFIG_ALIASOPTION_HPP
#define C2NG_GAME_CONFIG_ALIASOPTION_HPP

#include "afl/string/string.hpp"
#include "game/config/configurationoption.hpp"

namespace game { namespace config {

    class Configuration;

    /** Alias option.
        This simply forwards all requests to another option.

        If the forwarded option does not exist, calls are ignored (option is not created)! */
    class AliasOption : public ConfigurationOption {
     public:
        /** Constructor.
            @param container Container
            @param forwardedOptionName Name of original option */
        AliasOption(Configuration& container, String_t forwardedOptionName);

        // ConfigurationOption:
        virtual ~AliasOption();
        virtual void set(String_t value);
        virtual String_t toString() const;

        /** Get forwarded option.
            @return option; null if not known */
        ConfigurationOption* getForwardedOption() const;

     private:
        Configuration& m_container;
        const String_t m_forwardedOptionName;
    };

    /** Instantiation information for AliasOption. */
    struct AliasOptionDescriptor {
        const char* m_name;                     ///< Name of alias. Non-null.
        const char* m_forwardedOptionName;      ///< Name of underlying option. Non-null.

        // Meta-information
        typedef AliasOption OptionType_t;
        OptionType_t* create(Configuration& config) const
            { return new OptionType_t(config, m_forwardedOptionName); }
    };

} }

#endif
