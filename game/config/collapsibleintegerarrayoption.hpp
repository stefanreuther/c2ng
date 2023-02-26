/**
  *  \file game/config/collapsibleintegerarrayoption.hpp
  *  \brief Template Class game::config::CollapsibleIntegerArrayOption
  */
#ifndef C2NG_GAME_CONFIG_COLLAPSIBLEINTEGERARRAYOPTION_HPP
#define C2NG_GAME_CONFIG_COLLAPSIBLEINTEGERARRAYOPTION_HPP

#include "game/config/genericintegerarrayoption.hpp"
#include "game/config/valueparser.hpp"

namespace game { namespace config {

    class Configuration;

    /** Collapsible integer option array.
        This contains an array of int32_t values,
        parsed from a comma-separated list according to a ValueParser.

        If all values are the same, the string representation is shortened to a single element.

        \tparam N Array size */
    template<int N>
    class CollapsibleIntegerArrayOption : public GenericIntegerArrayOption {
     public:
        /** Constructor.
            Makes an option that uses the given ValueParser to parse values.
            The option is initialized to all-zero.
            \param parser Parser */
        explicit CollapsibleIntegerArrayOption(const ValueParser& parser);

        /** Destructor. */
        ~CollapsibleIntegerArrayOption();

        // GenericIntegerArrayOption:
        virtual afl::base::Memory<int32_t> getArray();

        // ConfigurationOption:
        virtual String_t toString() const;

        /** Copy from another option of the same type.
            \param other Source option */
        void copyFrom(const CollapsibleIntegerArrayOption<N>& other);

     private:
        int32_t m_values[N];
    };


    /** Instantiation information for CollapsibleIntegerArrayOption.

        \tparam N Array size */
    template<int N>
    struct CollapsibleIntegerArrayOptionDescriptor {
        // Instantiation information
        const char* m_name;                    ///< Name of option. Non-null.
        const ValueParser* m_parser;           ///< ValueParser instance. Non-null.

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
{
    getArray().fill(0);
}

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
