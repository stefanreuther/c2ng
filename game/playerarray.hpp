/**
  *  \file game/playerarray.hpp
  */
#ifndef C2NG_GAME_PLAYERARRAY_HPP
#define C2NG_GAME_PLAYERARRAY_HPP

#include "game/limits.hpp"

namespace game {

    /** Array indexed by player. */
    template<typename T>
    class PlayerArray {
     public:
        T* at(int pl);

        const T* at(int pl) const;

        void set(int pl, T value);

        T get(int pl) const;

        void setAll(T n);

     private:
        T m_data[MAX_PLAYERS+1];
    };

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
game::PlayerArray<T>::setAll(T n)
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

#endif
