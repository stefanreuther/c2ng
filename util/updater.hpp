/**
  *  \file util/updater.hpp
  *  \brief Class util::Updater
  */
#ifndef C2NG_UTIL_UPDATER_HPP
#define C2NG_UTIL_UPDATER_HPP

namespace util {

    /** Helper for updating variables.
        We often implement a "if these values differ, update them and call a listener" pattern.
        This class helps implementing this pattern:
        - create an Updater
        - call set(target, source) for all variables to update
        - use the Updater as a boolean predicate to test whether to call a listener.

        Calls can be chained, so we can write, for example,
        <tt>if (Updater().set(m_a, a).set(m_b, b)) {</tt> */
    class Updater {
     public:
        /** Constructor.
            Mark updater as "no update needed". */
        Updater();

        /** Set variable.
            Performs "out = in", and marks the updater as "update needed" if this is a change.
            \param out Target variable
            \param in Source value
            \return *this */
        template<typename T>
        Updater& set(T& out, const T& in);

        /** Check whether update is needed.
            \return non-null (true-ish) value if update is needed */
        operator const volatile void*() const;

     private:
        bool m_flag;
    };

}

inline
util::Updater::Updater()
    : m_flag(false)
{ }

template<typename T>
inline util::Updater&
util::Updater::set(T& out, const T& in)
{
    if (out != in) {
        out = in;
        m_flag = true;
    }
    return *this;
}

inline
util::Updater::operator const volatile void*() const
{
    return m_flag ? this : 0;
}

#endif
