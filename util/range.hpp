/**
  *  \file util/range.hpp
  *  \brief Template class util::Range
  */
#ifndef C2NG_UTIL_RANGE_HPP
#define C2NG_UTIL_RANGE_HPP

#include <algorithm>

namespace util {

    /** Number range.

        Represents a range of two numbers and provides operations on that.
        Ranges are represented by their (inclusive) bounds and can therefore cover
        the entire range of a given type.

        Ranges can also be empty, represented by the lower bound greater than the upper one.

        \tparam Underlying type */
    template<typename T>
    class Range {
     public:
        /** Default constructor.
            Makes an empty range.
            \post empty() */
        Range();

        /** Constructor.
            Makes a range as specified.
            \param min Minimum
            \param max Maximum */
        Range(T min, T max);

        /** Make unit range.
            Makes a range containing one single value.
            \param value Value
            \return range */
        static Range fromValue(T value);

        /** Check emptiness.
            \return true if range is empty. */
        bool empty() const;

        /** Check for unit range.
            \return true if this range contains just one value. */
        bool isUnit() const;

        /** Get minimum.
            \pre !empty()
            \return minimum value */
        T min() const;

        /** Get maximum.
            \pre !empty()
            \return maximum value */
        T max() const;

        /** Check for value.
            \param value Value
            \return true if range contains the given value */
        bool contains(T value) const;

        /** Check equality.
            \param other Other range
            \return true if both ranges are empty, or both cover the same values. */
        bool operator==(const Range& other) const;

        /** Check inequality.
            \param other Other range
            \return false if both ranges are empty, or both cover the same values. */
        bool operator!=(const Range& other) const;

        /** Include a single value.
            Modifies the range in-place to contain the given value.
            \param value Value
            \return *this
            \post contains(value) */
        Range& include(T value);

        /** Include another range.
            Modifies the range in-place to contain all values contained in the other range.
            \param other Other range
            \return *this */
        Range& include(const Range& other);

        /** Intersect ranges.
            Modifies the range in-place to contain only values also contained in the other range.
            \param other Other range
            \return *this */
        Range& intersect(const Range& other);

        /** Clear.
            \post empty() */
        void clear();

        /** Add two ranges.
            Modifies the range in-place to contain possible values that can be obtained by
            adding a number from this range to a number from the other one.
            \param other Other range
            \return *this */
        Range& operator+=(const Range& other);

        /** Subtract two ranges.
            Modifies the range in-place to contain possible values that can be obtained by
            subtracting a number from the other range from a number from this one.
            \param other Other range
            \return *this */
        Range& operator-=(const Range& other);

     private:
        T m_min;
        T m_max;
    };

}


template<typename T>
inline
util::Range<T>::Range()
    : m_min(1), m_max(0)
{ }

template<typename T>
inline
util::Range<T>::Range(T min, T max)
    : m_min(min), m_max(max)
{ }

template<typename T>
inline
util::Range<T>
util::Range<T>::fromValue(T value)
{
    return Range(value, value);
}

template<typename T>
inline bool
util::Range<T>::empty() const
{
    return m_min > m_max;
}

template<typename T>
inline bool
util::Range<T>::isUnit() const
{
    return m_min == m_max;
}

template<typename T>
inline T
util::Range<T>::min() const
{
    return m_min;
}

template<typename T>
inline T
util::Range<T>::max() const
{
    return m_max;
}

template<typename T>
inline bool
util::Range<T>::contains(T value) const
{
    return value >= m_min && value <= m_max;
}

template<typename T>
bool
util::Range<T>::operator==(const Range& other) const
{
    return empty() ? other.empty() : m_min == other.min() && m_max == other.max();
}

template<typename T>
inline bool
util::Range<T>::operator!=(const Range& other) const
{
    return !operator==(other);
}

template<typename T>
util::Range<T>&
util::Range<T>::include(T value)
{
    if (empty()) {
        m_min = m_max = value;
    } else {
        m_min = std::min(m_min, value);
        m_max = std::max(m_max, value);
    }
    return *this;
}

template<typename T>
util::Range<T>&
util::Range<T>::include(const Range& other)
{
    if (!other.empty()) {
        include(other.min());
        include(other.max());
    }
    return *this;
}

template<typename T>
util::Range<T>&
util::Range<T>::intersect(const Range& other)
{
    if (other.empty()) {
        *this = other;
    } else {
        m_min = std::max(m_min, other.min());
        m_max = std::min(m_max, other.max());
    }
    return *this;
}

template<typename T>
inline void
util::Range<T>::clear()
{
    m_min = 1;
    m_max = 0;
}

template<typename T>
util::Range<T>&
util::Range<T>::operator+=(const Range& other)
{
    if (empty() || other.empty()) {
        clear();
    } else {
        m_min += other.min();
        m_max += other.max();
    }
    return *this;
}

template<typename T>
util::Range<T>&
util::Range<T>::operator-=(const Range& other)
{
    if (empty() || other.empty()) {
        clear();
    } else {
        m_min -= other.max();
        m_max -= other.min();
    }
    return *this;
}

#endif
