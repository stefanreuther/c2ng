/**
  *  \file client/screenhistory.cpp
  */

#include "client/screenhistory.hpp"

/*
 *  FIXME: As of 20180916, ScreenHistory live on the UI side, within the UserSide object.
 *  An alternative implementation places it as a game::Session Extra.
 *  This would probably reduce the number of game<>ui transitions, and simplify expiry of obsolete objects.
 */

namespace {
    bool isSimilar(const client::ScreenHistory::Reference& a,
                   const client::ScreenHistory::Reference& b)
    {
        return a == b
            || (a.getType() == client::ScreenHistory::Starchart
                && b.getType() == client::ScreenHistory::Starchart);
    }
}


/*
 *  Reference
 */

// /** Create blank screen Id.
//     \post !isValid */
client::ScreenHistory::Reference::Reference()
    : m_type(Null), m_x(0), m_y(0)
{
    // ex WScreenId::WScreenId
}

// /** Create screen Id for an object.
//     \param screen_id Screen identifier
//     \param id        Selected object on that screen. Must be valid (not checked!)
//                      or zero. */
// /** Create screen Id for map location.
//     \param pt Map location */
client::ScreenHistory::Reference::Reference(Type type, int x, int y)
    : m_type(type), m_x(x), m_y(y)
{
    // ex WScreenId::WScreenId
}

bool
client::ScreenHistory::Reference::isSet() const
{
    // ex WScreenId::isNonNull
    // This function is not called "isValid" because it cannot check the validity of the referenced screen
    // (that is, whether "Planet 123" actually exists).
    return m_type != Null;
}

client::ScreenHistory::Type
client::ScreenHistory::Reference::getType() const
{
    // ex WScreenId::getScreenId
    return m_type;
}

int
client::ScreenHistory::Reference::getX() const
{
    // ex WScreenId::getId, WScreenId::getPos (simplified)
    return m_x;
}
int
client::ScreenHistory::Reference::getY() const
{
    return m_y;
}

bool
client::ScreenHistory::Reference::operator==(const Reference& other) const
{
    // ex WScreenId::operator==
    return m_type == other.m_type
        && m_x == other.m_x
        && m_y == other.m_y;
}
bool
client::ScreenHistory::Reference::operator!=(const Reference& other) const
{
    // ex WScreenId::operator!=
    return !operator==(other);
}


/*
 *  ScreenHistory
 */

client::ScreenHistory::ScreenHistory(size_t sizeLimit)
    : m_sizeLimit(sizeLimit),
      m_data()
{
}

// /** Push new history Id. This expresses that we're now in the
//     specified context. This function is idempotent in that it refuses
//     to push duplicates, and also tries to keep the history concise in
//     other ways. */
void
client::ScreenHistory::push(Reference ref)
{
    // ex pushHistoryScreenId, scrhist.pas:RegisterScreen
    // FIXME: deal with this guy -> discardBogusEntries();

    // If this is the same as what we already have on top, don't push anything.
    if (!m_data.empty() && isSimilar(ref, m_data.back())) {
        m_data.back() = ref;
        return;
    }

    // If this is the same that we already have on the bottom, uncover it.
    if (!m_data.empty() && isSimilar(ref, m_data.front())) {
        m_data.erase(m_data.begin());
        m_data.push_back(ref);
        return;
    }

    // If we have a situation A-B-A, and this is B, pop an A instead.
    // This avoids that the history clutters up with As and Bs if the
    // user rapidly switches between these two.
    // FIXME: make this smarter to detect any-size cycles
    if (m_data.size() > 2
        && m_data[m_data.size()-2] == ref
        && m_data[m_data.size()-1] == m_data[m_data.size()-3])
    {
        m_data.pop_back();
        return;
    }

    // If this would overflow the size, drop the first item
    if (m_data.size() >= m_sizeLimit) {
        m_data.erase(m_data.begin());
    }
    m_data.push_back(ref);
}

client::ScreenHistory::Reference
client::ScreenHistory::pop()
{
    // ex popHistoryScreenId (partial)
    // @change This function always removes the top element and moves it to the bottom.
    // Old popHistoryScreenId(current_screen_registered=true) translates to rotate() + pop().
    Reference result;
    if (!m_data.empty()) {
        result = m_data.back();
        rotate();
    }
    return result;
}

// /** Rotate history by one element. Turns situation A-B-C-D-E into
//     E-A-B-C-D. */
void
client::ScreenHistory::rotate()
{
    // ex client/history.cc:rotate
    if (!m_data.empty()) {
        Reference last = m_data.back();
        for (std::vector<Reference>::size_type i = m_data.size(); i > 1; --i) {
            m_data[i-1] = m_data[i-2];
        }
        m_data[0] = last;
    }
}

// /** Clear screen history. User upon every entry/exit of the race
//     screen. This makes sure we start with a blank list*/
void
client::ScreenHistory::clear()
{
    // ex clearScreenHistory, scrhist.pas:ClearBackup
    m_data.clear();
}

afl::base::Memory<const client::ScreenHistory::Reference>
client::ScreenHistory::getAll() const
{
    return m_data;
}

void
client::ScreenHistory::applyMask(afl::base::Memory<const bool> mask)
{
    size_t in = 0, out = 0, limit = m_data.size();
    while (in < limit) {
        const bool* p = mask.eat();
        if (p && *p) {
            m_data[out++] = m_data[in++];
        } else {
            ++in;
        }
    }
    m_data.resize(out);
}
