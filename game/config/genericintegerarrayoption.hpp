/**
  *  \file game/config/genericintegerarrayoption.hpp
  *  \brief Class game::config::GenericIntegerArrayOption
  */
#ifndef C2NG_GAME_CONFIG_GENERICINTEGERARRAYOPTION_HPP
#define C2NG_GAME_CONFIG_GENERICINTEGERARRAYOPTION_HPP

#include "game/config/configurationoption.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/types.hpp"

namespace game { namespace config {

    class ValueParser;

    /** Base integer option array.
        Base class for an int32_t array, can inform user about its dimensions.
        This is used for the script interface, but shouldn't be used directly. */
    class GenericIntegerArrayOption : public ConfigurationOption {
     public:
        /** Constructor.
            \param parser ValueParser for individual elements */
        explicit GenericIntegerArrayOption(const ValueParser& parser);

        /** Destructor. */
        ~GenericIntegerArrayOption();

        /** Get underlying array.
            Descendants must override this.
            Use this function to access the option's data and size.

            This function must not modify the GenericIntegerArrayOption object.

            \return Descriptor for option's array. */
        virtual afl::base::Memory<int32_t> getArray() = 0;

        /** Get underlying array, const version.
            \overload
            \return Descriptor for option's array. */
        afl::base::Memory<const int32_t> getArray() const;

        /** Check whether all values are the same (PHost "arrayized" option).
            \return true if all values are the same */
        bool isAllTheSame() const;

        /** Set individual element.
            \param index 1-based index
            \param value New value */
        void set(int index, int32_t value);

        /** Set all elements.
            \param value New value */
        void set(int32_t value);

        /** Get individual element.
            \param index 1-based index
            \return value */
        int32_t operator()(int index) const;

        // ConfigurationOption:
        virtual void set(String_t value);

        /** Get configured parser.
            \return parser */
        const ValueParser& parser() const;

     private:
        const ValueParser& m_parser;
    };

} }

#endif
