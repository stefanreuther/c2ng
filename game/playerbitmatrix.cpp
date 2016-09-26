/**
  *  \file game/playerbitmatrix.cpp
  */

#include "game/playerbitmatrix.hpp"
#include "afl/base/countof.hpp"

/** Constructor. */
game::PlayerBitMatrix::PlayerBitMatrix()
{ }

/** Get one bit.
    \param subj Subject
    \param obj  Object
    \return bit value; false if parameters are out of range. */
bool
game::PlayerBitMatrix::get(int subj, int obj) const
{
    if (subj <= 0 || subj > int(countof(m_data))) {
        return false;
    }
    if (obj <= 0 || obj > int(countof(m_data))) {
        return false;
    }

    return m_data[subj-1].contains(obj);
}

/** Set one bit.
    \param subj  Subject
    \param obj   Object
    \param value New value */
void
game::PlayerBitMatrix::set(int subj, int obj, bool value)
{
    if (subj <= 0 || subj > int(countof(m_data))) {
        return;
    }
    if (obj <= 0 || obj > int(countof(m_data))) {
        return;
    }

    if (value) {
        m_data[subj-1] += obj;
    } else {
        m_data[subj-1] -= obj;
    }
}

/** Get one row.
    \param subj Subject
    \return set of all objects for this subject. */
game::PlayerSet_t
game::PlayerBitMatrix::getRow(int subj) const
{
    if (subj <= 0 || subj > int(countof(m_data))) {
        return PlayerSet_t();
    } else {
        return m_data[subj-1];
    }
}

/** Clear this set. */
void
game::PlayerBitMatrix::clear()
{
    for (int i = 0; i < int(countof(m_data)); ++i) {
        m_data[i].clear();
    }
}
