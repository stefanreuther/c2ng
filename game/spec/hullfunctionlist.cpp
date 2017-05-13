/**
  *  \file game/spec/hullfunctionlist.cpp
  *  \brief Class game::spec::HullFunctionList
  */

#include <algorithm>
#include "game/spec/hullfunctionlist.hpp"
#include "afl/base/staticassert.hpp"
#include "game/limits.hpp"

namespace {
    int firstLevel(game::ExperienceLevelSet_t s)
    {
        int f = 0;
        while (f <= game::MAX_EXPERIENCE_LEVELS && !s.contains(f)) {
            ++f;
        }
        return f;
    }

    /** Sort predicate for HullFunctionList::simplify.
        Sort by function, then levels, then kind, then players. */
    class SortByFunctionAndLevel {
     public:
        bool operator()(const game::spec::HullFunction& lhs, const game::spec::HullFunction& rhs) const
            {
                if (lhs.getBasicFunctionId() != rhs.getBasicFunctionId()) {
                    return lhs.getBasicFunctionId() < rhs.getBasicFunctionId();
                }
                if (lhs.getLevels().toInteger() != rhs.getLevels().toInteger()) {
                    return lhs.getLevels().toInteger() < rhs.getLevels().toInteger();
                }
                if (lhs.getKind() != rhs.getKind()) {
                    return lhs.getKind() < rhs.getKind();
                }
                return lhs.getPlayers().toInteger() < rhs.getPlayers().toInteger();
            }
    };

    /** Sort predicate for players.
        This produces a convenient sort order for a player or set there-of. */
    class SortByPlayer {
     public:
        SortByPlayer(game::PlayerSet_t forPlayer)
            : m_forPlayer(forPlayer)
            { }
        bool operator()(const game::spec::HullFunction& lhs, const game::spec::HullFunction& rhs) const
            {
                // Racial abilities are boring and go last
                bool lrace = lhs.getKind() == game::spec::HullFunction::AssignedToRace;
                bool rrace = rhs.getKind() == game::spec::HullFunction::AssignedToRace;
                if (lrace != rrace) {
                    return lrace < rrace;
                }

                // Functions available to forPlayer go first
                bool lsee = lhs.getPlayers().containsAnyOf(m_forPlayer);
                bool rsee = rhs.getPlayers().containsAnyOf(m_forPlayer);
                if (lsee != rsee) {
                    return lsee > rsee;
                }

                // Next, sort by level
                int lfirst = firstLevel(lhs.getLevels());
                int rfirst = firstLevel(rhs.getLevels());
                if (lfirst != rfirst) {
                    return lfirst < rfirst;
                }
                if (lhs.getLevels() != rhs.getLevels()) {
                    return lhs.getLevels().toInteger() < rhs.getLevels().toInteger();
                }

                // Then, functions
                if (lhs.getBasicFunctionId() != rhs.getBasicFunctionId()) {
                    return lhs.getBasicFunctionId() < rhs.getBasicFunctionId();
                }

                // Finally, kind and players
                if (lhs.getKind() != rhs.getKind()) {
                    return lhs.getKind() < rhs.getKind();
                }
                return lhs.getPlayers().toInteger() < rhs.getPlayers().toInteger();
            }
     private:
        const game::PlayerSet_t m_forPlayer;
    };
}


// Default constructor.
game::spec::HullFunctionList::HullFunctionList()
    : m_data()
{ }

// Destructor.
game::spec::HullFunctionList::~HullFunctionList()
{ }

// Append new item at end.
void
game::spec::HullFunctionList::add(const HullFunction& f)
{
    m_data.push_back(f);
}

// Clear list.
void
game::spec::HullFunctionList::clear()
{
    m_data.clear();
}

// Simplify the list.
void
game::spec::HullFunctionList::simplify()
{
    Container_t::size_type in, out;

    // Early exit.
    // (Borland used to crash when size() is zero, and it doesn't hurt.)
    if (m_data.size() == 0) {
        return;
    }

    // Pass 1: combine identical functions, and wield out empty assignments.
    std::sort(m_data.begin(), m_data.end(), SortByFunctionAndLevel());
    in = out = 0;
    while (in < m_data.size()) {
        HullFunction func = m_data[in++];
        while (in < m_data.size() && m_data[in].isSame(func) && m_data[in].getKind() == func.getKind()) {
            func.setPlayers(func.getPlayers() | m_data[in].getPlayers());
            ++in;
        }
        if (func.getPlayers().nonempty()) {
            m_data[out++] = func;
        }
    }
    m_data.resize(out);

    // Pass 2: remove racial abilities that are a strict subset of hull functions, and vice versa
    /* FIXME: what do we do with hull vs. ship functions? */
    static_assert(HullFunction::AssignedToHull+1 == HullFunction::AssignedToRace, "must be adjacent so our comparison finds them");
    in = out = 0;
    while (in+1 < m_data.size()) {
        if (m_data[in].isSame(m_data[in+1])
            && m_data[in].getKind() == HullFunction::AssignedToHull
            && m_data[in+1].getKind() == HullFunction::AssignedToRace)
        {
            if (m_data[in].getPlayers().contains(m_data[in+1].getPlayers())) {
                // in (the hull function) contains everything
                m_data[out++] = m_data[in++];
                ++in;
            } else if (m_data[in+1].getPlayers().contains(m_data[in].getPlayers())) {
                // in+1 (the racial ability) contains everything
                ++in;
                m_data[out++] = m_data[in++];
            } else {
                // no change, copy both
                m_data[out++] = m_data[in++];
                m_data[out++] = m_data[in++];
            }
        } else {
            // no match, copy one
            m_data[out++] = m_data[in++];
        }
    }

    // copy final item, if any
    if (in < m_data.size()) {
        m_data[out++] = m_data[in++];
    }

    m_data.resize(out);

    /* Other things we could do:
       - remove implied functions
       - combine level-restricted functions, i.e. if a ship has both a
         cloak-that-works-on-L1 and a cloak-that-works-on-L2, combine that
         into a cloak-that-works-on-L1-and-L2 */
}

// Sort list for new ships.
void
game::spec::HullFunctionList::sortForNewShip(PlayerSet_t forPlayer)
{
    std::sort(m_data.begin(), m_data.end(), SortByPlayer(forPlayer));
}

// Get number of items in list.
game::spec::HullFunctionList::Container_t::size_type
game::spec::HullFunctionList::size() const
{
    return m_data.size();
}

// Get iterator to first item.
game::spec::HullFunctionList::Iterator_t
game::spec::HullFunctionList::begin() const
{
    return m_data.begin();
}

// Get iterator to one-past-end.
game::spec::HullFunctionList::Iterator_t
game::spec::HullFunctionList::end() const
{
    return m_data.end();
}

// Indexed access.
const game::spec::HullFunction&
game::spec::HullFunctionList::operator[](Container_t::size_type i) const
{
    return m_data[i];
}
