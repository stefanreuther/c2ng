/**
  *  \file server/user/userdata.cpp
  *  \brief Class server::user::UserData
  *
  *  This functionality was originally implemented only in the API (api/user.cgi).
  */

#include <stdexcept>
#include "server/user/userdata.hpp"
#include "server/errors.hpp"
#include "server/user/configuration.hpp"
#include "server/user/root.hpp"
#include "server/user/user.hpp"

namespace {
    /*
     *  Database structure
     */

    class Data {
     public:
        Data(afl::net::redis::Subtree tree)
            : m_tree(tree)
            { }

        afl::net::redis::StringKey data(String_t key)
            { return m_tree.subtree("data").stringKey(key); }

        afl::net::redis::StringListKey usedKeys()
            { return m_tree.stringListKey("list"); }

        afl::net::redis::IntegerKey totalSize()
            { return m_tree.intKey("size"); }

     private:
        afl::net::redis::Subtree m_tree;
    };

    /* Estimate size of a key/value store.
       Empty values don't take any space.
       For non-empty values, count the key twice because we store it twice
       (as actual key name, and on the LRU list). */
    size_t estimateSize(const String_t& key, const String_t& value)
    {
        size_t size = value.size();
        if (size != 0) {
            size += 2*key.size();
        }
        return size;
    }

    /* Validate key. Needs to be printable and of reasonable size. */
    void validateKey(const String_t& key, const server::user::Configuration& config)
    {
        bool valid = (!key.empty() && key.size() <= config.userDataMaxKeySize);
        if (valid) {
            for (size_t i = 0, n = key.size(); i < n; ++i) {
                uint8_t uch = static_cast<uint8_t>(key[i]);
                if (uch < 0x20 || uch > 0x7E) {
                    valid = false;
                    break;
                }
            }
        }
        if (!valid) {
            throw std::runtime_error(server::INVALID_KEY);
        }
    }

    /* Validate value */
    void validateValue(const String_t& value, const server::user::Configuration& config)
    {
        if (value.size() > config.userDataMaxValueSize) {
            throw std::runtime_error(server::INVALID_VALUE);
        }
    }
}


// Constructor.
server::user::UserData::UserData(Root& root)
    : m_root(root)
{ }

void
server::user::UserData::set(String_t userId, String_t key, String_t value)
{
    // Validate
    validateKey(key, m_root.config());
    validateValue(value, m_root.config());

    // Database access
    Data d(User(m_root, userId).userData());
    String_t oldValue = d.data(key).replaceBy(value);

    // Account weights
    size_t newWeight = estimateSize(key, value);
    size_t oldWeight = estimateSize(key, oldValue);
    size_t newSize = (d.totalSize() += int(newWeight - oldWeight));

    // Maintain LRU list
    d.usedKeys().removeValue(key, 0);
    if (!value.empty()) {
        d.usedKeys().pushFront(key);
    } else {
        d.data(key).remove();
    }

    // Delete keys
    while (newSize > m_root.config().userDataMaxTotalSize) {
        String_t oldKey = d.usedKeys().popBack();
        if (key.empty()) {
            // Happens only on inconsistent data
            break;
        }
        int thisSize = int(estimateSize(oldKey, d.data(oldKey).get()));
        newSize -= thisSize;
        d.data(oldKey).remove();
        d.totalSize() -= thisSize;
    }
}

String_t
server::user::UserData::get(String_t userId, String_t key)
{
    validateKey(key, m_root.config());
    return Data(User(m_root, userId).userData()).data(key).get();
}
