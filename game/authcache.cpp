/**
  *  \file game/authcache.cpp
  *  \brief Class game::AuthCache
  */

#include "game/authcache.hpp"

namespace {
    template<typename T>
    bool matchParameter(const afl::base::Optional<T>& required,
                        const afl::base::Optional<T>& available)
    {
        const T* pReq = required.get();
        const T* pAva = available.get();
        return pReq == 0                // requirement says: I don't care
            || pAva == 0                // available says: I match anything
            || *pReq == *pAva;          // exact match
    }
}

game::AuthCache::AuthCache()
    : m_content()
{ }

game::AuthCache::~AuthCache()
{ }

void
game::AuthCache::clear()
{
    // ex int/if/globalif.cc:clearSavedPasswords
    m_content.clear();
}

void
game::AuthCache::addNew(Item* pItem)
{
    // Just append. There's no general way to handle replacements.
    m_content.pushBackNew(pItem);
}

game::AuthCache::Items_t
game::AuthCache::find(const Item& match) const
{
    // ex int/if/globalif.cc:checkSavedPassword (sort-of)
    Items_t result;
    for (size_t i = 0, n = m_content.size(); i < n; ++i) {
        if (const Item* pItem = m_content[i]) {
            if (matchParameter(match.playerNr, pItem->playerNr)) {
                result.push_back(pItem);
            }
        }
    }
    return result;
}
