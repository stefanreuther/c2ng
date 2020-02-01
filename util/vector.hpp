/**
  *  \file util/vector.hpp
  *  \brief Template class util::Vector
  */
#ifndef C2NG_UTIL_VECTOR_HPP
#define C2NG_UTIL_VECTOR_HPP

#include <vector>
#include "afl/base/types.hpp"

namespace util {

    /** Automatic vector.
        Provides a container similar to std::vector (actually, based on it),
        that automatically grows when indexes beyond the current end are used.
        In addition, a minimum index can be set; stores below that index are ignored.

        The intended use is the multitude of (often 1-based) vectors of things
        that typically have a fixed upper bound in VGAP, which we want to keep flexible.

        \tparam Value Contained value type. Must be default-constructible.
        \tparam Index Index type. Must be possible to cast to and from size_t. */
    template<typename Value, typename Index>
    class Vector {
     public:
        /** Constructor.
            \param minIndex Minimum index */
        explicit Vector(Index minIndex = Index());

        /** Destructor. */
        ~Vector();

        /** Set value at index.
            If the element is after the current end of the vector, it will be created.
            If the element is before minIndex, the call will be ignored.
            \param index Index
            \param value Value */
        void set(Index index, Value value);

        /** Get value at index.
            If the element exists, it is returned as a copy.
            If the element does not exist, a default-constructed value is returned.
            \param index Index */
        Value get(Index index) const;

        /** Get pointer to element at index.
            If the element exists, a pointer is returned.
            If the element does not exist, a null pointer is returned.
            \param index Index */
        const Value* at(Index index) const;

        /** Get pointer to element at index.
            \overload */
        Value* at(Index index);

        /** Clear vector.
            \post size() == minIndex */
        void clear();

        /** Get size.
            Elements at this or a larger index will not exist.
            For a 1-based vector with elements 1,2,3, this will be 4.
            For a 0-based vector with elements 0,1,2, this will be 3.
            \return size */
        Index size() const;

        /** Check emptiness.
            \return true if the underlying vector is empty, i.e. size() == minIndex */
        bool empty() const;

     private:
        std::vector<Value> m_data;
        Index m_minIndex;
    };

}

template<typename Value, typename Index>
inline
util::Vector<Value,Index>::Vector(Index minIndex)
    : m_data(),
      m_minIndex(minIndex)
{ }

template<typename Value, typename Index>
util::Vector<Value,Index>::~Vector()
{ }

template<typename Value, typename Index>
void
util::Vector<Value,Index>::set(Index index, Value value)
{
    size_t slot = size_t(index) - size_t(m_minIndex);
    if (index >= m_minIndex && Index(slot + size_t(m_minIndex)) == index) {
        if (m_data.size() <= slot) {
            m_data.resize(slot + 1);
        }
        m_data[slot] = value;
    }
}

template<typename Value, typename Index>
Value
util::Vector<Value,Index>::get(Index index) const
{
    if (const Value* p = at(index)) {
        return *p;
    } else {
        return Value();
    }
}

template<typename Value, typename Index>
inline const Value*
util::Vector<Value,Index>::at(Index index) const
{
    return const_cast<Vector&>(*this).at(index);
}

template<typename Value, typename Index>
Value*
util::Vector<Value,Index>::at(Index index)
{
    size_t slot = size_t(index) - size_t(m_minIndex);
    if (index >= m_minIndex && slot < m_data.size()) {
        return &m_data[slot];
    } else {
        return 0;
    }
}

template<typename Value, typename Index>
inline void
util::Vector<Value,Index>::clear()
{
    m_data.clear();
}

template<typename Value, typename Index>
inline Index
util::Vector<Value,Index>::size() const
{
    return Index(m_data.size() + m_minIndex);
}

template<typename Value, typename Index>
inline bool
util::Vector<Value,Index>::empty() const
{
    return m_data.empty();
}

#endif
