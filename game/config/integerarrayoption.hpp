/**
  *  \file game/config/integerarrayoption.hpp
  */
#ifndef C2NG_GAME_CONFIG_INTEGERARRAYOPTION_HPP
#define C2NG_GAME_CONFIG_INTEGERARRAYOPTION_HPP

#include "game/config/genericintegerarrayoption.hpp"

namespace game { namespace config {

    /** Integer option array.
        This contains an array of int32_t values,
        parsed from a comma-separated list according to a ValueParser. */
    template<int N>
    class IntegerArrayOption : public GenericIntegerArrayOption {
     public:
        explicit IntegerArrayOption(const ValueParser& parser);
        explicit IntegerArrayOption(const ValueParser& parser, const int32_t (&defaultValue)[N]);

        ~IntegerArrayOption();

        // GenericIntegerArrayOption:
        virtual afl::base::Memory<int32_t> getArray();

        // ConfigurationOption:
        virtual String_t toString() const;

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
{ }

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
