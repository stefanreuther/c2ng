/**
  *  \file util/vector.hpp
  */
#ifndef C2NG_UTIL_VECTOR_HPP
#define C2NG_UTIL_VECTOR_HPP

#include <vector>

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
        explicit Vector(Index minIndex = Index());

        ~Vector();

        void set(Index index, Value value);

        Value get(Index index) const;

        const Value* at(Index index) const;

        Value* at(Index index);

        void clear();

        Index size() const;

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

#endif
