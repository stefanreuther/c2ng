/**
  *  \file game/config/stringoption.hpp
  *  \brief Class game::config::StringOption
  */
#ifndef C2NG_GAME_CONFIG_STRINGOPTION_HPP
#define C2NG_GAME_CONFIG_STRINGOPTION_HPP

#include "afl/string/string.hpp"
#include "game/config/configurationoption.hpp"

namespace game { namespace config {

    class Configuration;

    /** String option.
        This contains a single string. */
    class StringOption : public ConfigurationOption {
     public:
        /** Constructor.
            \param initialValue [optional] Initial value */
        explicit StringOption(const char* initialValue = "");

        /** Destructor. */
        ~StringOption();

        /** Set value from string.
            \param value New value */
        virtual void set(String_t value);

        /** Set value from C string.
            \param value New value */
        void set(const char* value);

        /** Get value as string.
            \return value */
        virtual String_t toString() const;

        /** Get value as string.
            \return value */
        String_t operator()() const;

     private:
        String_t m_value;
    };

    struct StringOptionDescriptor {
        // Instantiation information
        const char* m_name;

        // Meta-information
        typedef StringOption OptionType_t;
        StringOption* create(Configuration& /*option*/) const
            { return new StringOption(); }
    };

} }

#endif
