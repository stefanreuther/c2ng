/**
  *  \file game/config/integerarrayoption.hpp
  *  \brief Template class game::config::IntegerArrayOption
  */
#ifndef C2NG_GAME_CONFIG_INTEGERARRAYOPTION_HPP
#define C2NG_GAME_CONFIG_INTEGERARRAYOPTION_HPP

#include "game/config/genericintegerarrayoption.hpp"
#include "game/config/valueparser.hpp"

namespace game { namespace config {

    class Configuration;

    /** Integer option array.
        This contains an array of int32_t values,
        parsed from a comma-separated list according to a ValueParser.
        \tparam N Array size */
    template<int N>
    class IntegerArrayOption : public GenericIntegerArrayOption {
     public:
        /** Constructor.
            Initializes the option to all-zero.
            \param parser Parser */
        explicit IntegerArrayOption(const ValueParser& parser);

        /** Constructor.
            Initializes the option to the given defaults.
            \param parser Parser
            \param defaultValue Default values */
        explicit IntegerArrayOption(const ValueParser& parser, const int32_t (&defaultValue)[N]);

        /** Destructor. */
        ~IntegerArrayOption();

        // GenericIntegerArrayOption:
        virtual afl::base::Memory<int32_t> getArray();

        // ConfigurationOption:
        virtual String_t toString() const;

        /** Copy from another option of the same type.
            \param other Source option */
        void copyFrom(const IntegerArrayOption<N>& other);

     private:
        int32_t m_values[N];
    };

    template<int N>
    struct IntegerArrayOptionDescriptor {
        // Instantiation information
        const char* m_name;
        const ValueParser* m_parser;

        // Meta-information
        typedef IntegerArrayOption<N> OptionType_t;
        IntegerArrayOption<N>* create(Configuration& /*option*/) const
            { return new IntegerArrayOption<N>(*m_parser); }
    };

    template<int N>
    struct IntegerArrayOptionDescriptorWithDefault {
        // Instantiation information
        const char* m_name;
        const ValueParser* m_parser;
        int32_t m_defaultValue[N];

        // Meta-information
        typedef IntegerArrayOption<N> OptionType_t;
        IntegerArrayOption<N>* create(Configuration& /*option*/) const
            { return new IntegerArrayOption<N>(*m_parser, m_defaultValue); }
    };

} }

template<int N>
inline
game::config::IntegerArrayOption<N>::IntegerArrayOption(const ValueParser& parser)
    : GenericIntegerArrayOption(parser)
{
    getArray().fill(0);
}

template<int N>
inline
game::config::IntegerArrayOption<N>::IntegerArrayOption(const ValueParser& parser, const int32_t (&defaultValue)[N])
    : GenericIntegerArrayOption(parser)
{
    getArray().copyFrom(defaultValue);
}

template<int N>
inline
game::config::IntegerArrayOption<N>::~IntegerArrayOption()
{ }

template<int N>
afl::base::Memory<int32_t>
game::config::IntegerArrayOption<N>::getArray()
{
    return afl::base::Memory<int32_t>(m_values);
}

template<int N>
String_t
game::config::IntegerArrayOption<N>::toString() const
{
    // ex ConfigIntArrayOption<N>::toString
    return parser().toStringArray(m_values);
}

template<int N>
void
game::config::IntegerArrayOption<N>::copyFrom(const IntegerArrayOption<N>& other)
{
    getArray().copyFrom(other.m_values);
    markChanged();
}

#endif
