/**
  *  \file game/playerarray.hpp
  *  \brief Class game::PlayerArray
  */
#ifndef C2NG_GAME_PLAYERARRAY_HPP
#define C2NG_GAME_PLAYERARRAY_HPP

#include "game/limits.hpp"

namespace game {

    /** Array indexed by player.
        Contains an array of elements indexed by player numbers (including 0).
        \tparam T content type */
    template<typename T>
    class PlayerArray {
     public:
        /** Constructor.
            Initialize all elements to the given value.
            \param value default value */
        PlayerArray(T value = T());

        /** Access one player's element.
            \param pl Player number [0,MAX_PLAYERS]
            \return Pointer to element, null if out of range */
        T* at(int pl);

        /** Access one player's element.
            \param pl Player number [0,MAX_PLAYERS]
            \return Pointer to element, null if out of range */
        const T* at(int pl) const;

        /** Set one player's element.
            \param pl Player number [0,MAX_PLAYERS]. Call is ignored if out-of-range.
            \param value Value */
        void set(int pl, T value);

        /** Get one player's element.
            \param pl Player number [0,MAX_PLAYERS]
            \return Value; default-constructed if out of range */
        T get(int pl) const;

        /** Set all values.
            \param n New value */
        void setAll(const T& n);

        /** Compare for equality.
            \param other Other array
            \return true on equality */
        bool operator==(const PlayerArray& other) const;

        /** Compare for inequality.
            \param other Other array
            \return true on inequality */
        bool operator!=(const PlayerArray& other) const;

     private:
        T m_data[MAX_PLAYERS+1];
    };

}


template<typename T>
game::PlayerArray<T>::PlayerArray(T value)
{
    setAll(value);
}

template<typename T>
T*
game::PlayerArray<T>::at(int pl)
{
    // ex GPlayerArray<T>::operator[](int pl)
    if (pl >= 0 && pl <= MAX_PLAYERS) {
        return &m_data[pl];
    } else {
        return 0;
    }
}

template<typename T>
const T*
game::PlayerArray<T>::at(int pl) const
{
    // ex GPlayerArray<T>::operator[](int pl)
    if (pl >= 0 && pl <= MAX_PLAYERS) {
        return &m_data[pl];
    } else {
        return 0;
    }
}

template<typename T>
void
game::PlayerArray<T>::setAll(const T& n)
{
    // ex GPlayerArray<T>::setAll
    for (int i = 0; i < MAX_PLAYERS+1; ++i) {
        m_data[i] = n;
    }
}

template<typename T>
void
game::PlayerArray<T>::set(int pl, T value)
{
    if (T* p = at(pl)) {
        *p = value;
    }
}

template<typename T>
T
game::PlayerArray<T>::get(int pl) const
{
    if (const T* p = at(pl)) {
        return *p;
    } else {
        return T();
    }
}

template<typename T>
bool
game::PlayerArray<T>::operator==(const PlayerArray& other) const
{
    for (int i = 0; i < MAX_PLAYERS+1; ++i) {
        if (m_data[i] != other.m_data[i]) {
            return false;
        }
    }
    return true;
}

template<typename T>
inline bool
game::PlayerArray<T>::operator!=(const PlayerArray& other) const
{
    return !operator==(other);
}

#endif
