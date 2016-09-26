/**
  *  \file game/config/collapsibleintegerarrayoption.hpp
  */
#ifndef C2NG_GAME_CONFIG_COLLAPSIBLEINTEGERARRAYOPTION_HPP
#define C2NG_GAME_CONFIG_COLLAPSIBLEINTEGERARRAYOPTION_HPP

#include "game/config/genericintegerarrayoption.hpp"
#include "game/config/valueparser.hpp"

namespace game { namespace config {

    /** Integer option array.
        This contains an array of int32_t values,
        parsed from a comma-separated list according to a ValueParser. */
    template<int N>
    class CollapsibleIntegerArrayOption : public GenericIntegerArrayOption {
     public:
        explicit CollapsibleIntegerArrayOption(const ValueParser& parser);

        ~CollapsibleIntegerArrayOption();

        // GenericIntegerArrayOption:
        virtual afl::base::Memory<int32_t> getArray();

        // ConfigurationOption:
        virtual String_t toString() const;

        void copyFrom(const CollapsibleIntegerArrayOption<N>& other);

     private:
        int32_t m_values[N];
    };


    template<int N>
    struct CollapsibleIntegerArrayOptionDescriptor {
        // Instantiation information
        const char* m_name;
        const ValueParser* m_parser;

        // Meta-information
        typedef CollapsibleIntegerArrayOption<N> OptionType_t;
        CollapsibleIntegerArrayOption<N>* create(Configuration& /*option*/) const
            { return new CollapsibleIntegerArrayOption<N>(*m_parser); }
    };

} }

template<int N>
inline
game::config::CollapsibleIntegerArrayOption<N>::CollapsibleIntegerArrayOption(const ValueParser& parser)
    : GenericIntegerArrayOption(parser)
{ }

template<int N>
inline
game::config::CollapsibleIntegerArrayOption<N>::~CollapsibleIntegerArrayOption()
{ }

template<int N>
afl::base::Memory<int32_t>
game::config::CollapsibleIntegerArrayOption<N>::getArray()
{
    return afl::base::Memory<int32_t>(m_values);
}

template<int N>
String_t
game::config::CollapsibleIntegerArrayOption<N>::toString() const
{
    // ex ConfigStdOption::toString
    if (isAllTheSame()) {
        return parser().toString(m_values[0]);
    } else {
        return parser().toStringArray(m_values);
    }
}

template<int N>
void
game::config::CollapsibleIntegerArrayOption<N>::copyFrom(const CollapsibleIntegerArrayOption<N>& other)
{
    // getArray().copyFrom(other.m_values);
    getArray().copyFrom(const_cast<CollapsibleIntegerArrayOption<N>&>(other).getArray());
    markChanged();
}

#endif
