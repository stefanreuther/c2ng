/**
  *  \file game/config/stringoption.hpp
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
        explicit StringOption(const char* initialValue = "");

        ~StringOption();
        virtual void set(String_t value);
        void set(const char* value);
        virtual String_t toString() const;
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
