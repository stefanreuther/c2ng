/**
  *  \file game/spec/info/filter.hpp
  *  \brief Class game::spec::info::Filter
  */
#ifndef C2NG_GAME_SPEC_INFO_FILTER_HPP
#define C2NG_GAME_SPEC_INFO_FILTER_HPP

#include "game/spec/info/types.hpp"

namespace game { namespace spec { namespace info {

    class Browser;

    /** Filter for object lists.
        A filter consists of a list of FilterElement's, each filtering on a numeric attribute.
        In addition, it can contain a single optional name filter which is treated specially.

        Internally, the filter is represented as a list of these numeric filters.
        For display in the UI, it can be formatted into a textual information with metadata
        (describe() methods). */
    class Filter {
     public:
        /** Shortcut: FilterElement container. */
        typedef std::vector<FilterElement> Container_t;

        /** Shortcut: FilterElement iterator. */
        typedef Container_t::const_iterator Iterator_t;


        /** Constructor. */
        Filter();

        /** Destructor. */
        ~Filter();

        /** Add filter element.
            If an element of the same type already exists, it is overwritten.
            \param e element to add (must not be String_Name) */
        void add(const FilterElement& e);

        /** Describe entire filter.
            \param [out] result Result; the result will include the name filter if any.
            \param [in] browser Browser (provides access to environment) */
        void describe(FilterInfos_t& result, const Browser& browser) const;

        /** Describe single element.
            \param e element to describe
            \param browser Browser (provides access to environment)
            \return populated FilterInfo */
        FilterInfo describe(const FilterElement& e, const Browser& browser) const;

        /** Get player filter.
            If this filter contains a Value_Player element, returns its parameter.
            Otherwise, returns 0.
            \return result */
        int getPlayerFilter() const;

        /** Erase element by index.

            The index points into the result of describe(FilterInfos_t&).
            - [0,size()) for numeric filters
            - size(), if applicable, for name filter

            \param index Index to remove */
        void erase(size_t index);

        /** Get number of numeric filter elements.
            \return number of elements */
        size_t size() const;

        /** Set 'range' in an element.
            Call is ignored if index is out of range.
            \param index Index [0,size())
            \param range New range */
        void setRange(size_t index, IntRange_t range);

        /** Set 'value' in an element.
            Call is ignored if index is out of range.
            \param index Index [0,size())
            \param value New value */
        void setValue(size_t index, int32_t value);

        /** Set name filter.
            \param value new filter */
        void setNameFilter(const String_t& value);

        /** Get name filter.
            \return name filter */
        const String_t& getNameFilter() const;

        /** Get iterator to beginning of numeric filter list.
            \return iterator */
        Iterator_t begin() const;

        /** Get iterator to one-past-end of numeric filter list.
            \return iterator */
        Iterator_t end() const;

     private:
        Container_t m_content;
        String_t m_nameFilter;

        FilterElement* find(FilterAttribute a);
    };

} } }

inline game::spec::info::Filter::Iterator_t
game::spec::info::Filter::begin() const
{
    return m_content.begin();
}

inline game::spec::info::Filter::Iterator_t
game::spec::info::Filter::end() const
{
    return m_content.end();
}

#endif
