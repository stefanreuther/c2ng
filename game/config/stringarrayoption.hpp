/**
  *  \file game/config/stringarrayoption.hpp
  *  \brief Class game::config::StringArrayOption
  */
#ifndef C2NG_GAME_CONFIG_STRINGARRAYOPTION_HPP
#define C2NG_GAME_CONFIG_STRINGARRAYOPTION_HPP

#include <vector>
#include "afl/string/string.hpp"
#include "game/config/configurationoption.hpp"

namespace game { namespace config {

    class Configuration;

    /** String array option.
        Represents an array of strings, represented as comma-separated list. */
    class StringArrayOption : public ConfigurationOption {
     public:
        /** Constructor.
            @param minIndex   Index of first value
            @param numSlots   Number of values */
        StringArrayOption(int minIndex, int numSlots);

        /** Destructor. */
        ~StringArrayOption();

        /** Set one value.
            @param index   Index [minIndex, minIndex+numSlots)
            @param value   New value */
        void set(int index, String_t value);

        /** Get one value.
            @param index   Index [minIndex, minIndex+numSlots)
            @return value; empty string if index is out of range */
        String_t operator()(int index) const;

        /** Get first valid index.
            @return index */
        int getFirstIndex() const;

        /** Get number of items.
            @return number */
        int getNumSlots() const;

        // ConfigurationOption:
        virtual void set(String_t value);
        virtual String_t toString() const;

     private:
        int m_minIndex;
        std::vector<String_t> m_data;
    };

    /** Descriptor for StringArrayOption. */
    struct StringArrayOptionDescriptor {
        // Instantiation information
        const char* m_name;
        int minIndex;
        int numSlots;

        // Meta-information
        typedef StringArrayOption OptionType_t;
        StringArrayOption* create(Configuration& /*option*/) const
            { return new StringArrayOption(minIndex, numSlots); }
    };

} }

#endif
