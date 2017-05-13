/**
  *  \file game/config/integeroption.hpp
  *  \brief Class game::config::IntegerOption
  */
#ifndef C2NG_GAME_CONFIG_INTEGEROPTION_HPP
#define C2NG_GAME_CONFIG_INTEGEROPTION_HPP

#include "game/config/configurationoption.hpp"
#include "afl/base/types.hpp"

namespace game { namespace config {

    class ValueParser;
    class Configuration;

    /** Integer option.
        This contains a single value, parsed from text according to a ValueParser. */
    class IntegerOption : public ConfigurationOption {
     public:
        /** Constructor.
            \param parser ValueParser instance. Must have sufficient lifetime.
            \param initialValue Initial value */
        explicit IntegerOption(const ValueParser& parser, int32_t initialValue = 0);

        /** Destructor. */
        ~IntegerOption();

        // ConfigurationOption:
        virtual void set(String_t value);
        virtual String_t toString() const;

        /** Set value.
            \param newValue New value */
        void set(int32_t newValue);

        /** Get value.
            \return value */
        int32_t operator()() const;

        /** Copy from another option.
            \param other other option */
        void copyFrom(const IntegerOption& other);

        /** Access ValueParser.
            \return parser instance */
        const ValueParser& parser() const;

     private:
        const ValueParser& m_parser;
        int32_t m_value;
    };

    struct IntegerOptionDescriptor {
        // Instantiation information
        const char* m_name;
        ValueParser* m_parser;

        // Meta-information
        typedef IntegerOption OptionType_t;
        IntegerOption* create(Configuration& /*option*/) const
            { return new IntegerOption(*m_parser); }
    };

    struct IntegerOptionDescriptorWithDefault {
        // Instantiation information
        const char* m_name;
        ValueParser* m_parser;
        int32_t m_defaultValue;

        // Meta-information
        typedef IntegerOption OptionType_t;
        IntegerOption* create(Configuration& /*option*/) const
            { return new IntegerOption(*m_parser, m_defaultValue); }
    };

} }

#endif
