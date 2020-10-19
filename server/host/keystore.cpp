/**
  *  \file server/host/keystore.cpp
  *  \brief Class server::host::KeyStore
  */

#include "server/host/keystore.hpp"
#include "afl/data/access.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/sortoperation.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "server/host/configuration.hpp"

using game::v3::RegistrationKey;

/* Wrapper for the user:$UID:key:id:$KEYID subtree */
class server::host::KeyStore::Key {
 public:
    Key(afl::net::redis::HashKey hash)
        : m_hash(hash)
        { }

    afl::net::redis::StringField blob()
        { return m_hash.stringField("blob"); }
    afl::net::redis::IntegerField useCount()
        { return m_hash.intField("useCount"); }
    afl::net::redis::IntegerField lastUsed()
        { return m_hash.intField("lastUsed"); }
    afl::net::redis::IntegerField lastGame()
        { return m_hash.intField("lastGame"); }
    void remove()
        { m_hash.remove(); }

 private:
    afl::net::redis::HashKey m_hash;
};



// Constructor.
server::host::KeyStore::KeyStore(afl::net::redis::Subtree tree, const Configuration& config)
    : m_tree(tree),
      m_maxStoredKeys(config.maxStoredKeys)
{ }

// Add a key.
void
server::host::KeyStore::addKey(const game::v3::RegistrationKey& key, Time_t time, int32_t gameId)
{
    if (m_maxStoredKeys != 0) {
        const String_t keyId = key.getKeyId();
        Key k = keyById(keyId);

        // Remember game Id
        if (gameId != 0) {
            k.lastUsed().set(time);
            k.lastGame().set(gameId);
        }

        // Add use count. If this is the first use, add it to the global list.
        if (++k.useCount() == 1) {
            uint8_t blob[RegistrationKey::KEY_SIZE_BYTES];
            key.packIntoBytes(blob);
            k.blob().set(afl::string::fromBytes(blob));
            allKeys().add(keyId);
            expireKeys();
        }
    }
}

// Get list of all stored keys.
void
server::host::KeyStore::listKeys(Infos_t& result, afl::charset::Charset& charset)
{
    // Obtain list from server
    Key tpl = keyById("*");
    std::auto_ptr<Value_t> p(allKeys().sort().sortLexicographical()
                             .get()
                             .get(tpl.lastUsed())
                             .get(tpl.lastGame())
                             .get(tpl.useCount())
                             .get(tpl.blob())
                             .getResult());
    afl::data::Access a(p.get());

    // Produce result
    size_t n = a.getArraySize();
    size_t i = 0;
    while (i < n) {
        Info elem;
        elem.keyId = a[i++].toString();
        elem.lastUsed = a[i++].toInteger();
        elem.lastGame = a[i++].toInteger();
        elem.useCount = a[i++].toInteger();
        String_t blob = a[i++].toString();
        if (!blob.empty()) {
            RegistrationKey key(std::auto_ptr<afl::charset::Charset>(charset.clone()));
            key.unpackFromBytes(afl::string::toBytes(blob));
            elem.isRegistered = (key.getStatus() == key.Registered);
            elem.label1 = key.getLine(RegistrationKey::Line1);
            elem.label2 = key.getLine(RegistrationKey::Line2);
            result.push_back(elem);
        }
    }
}

// Get key by Id.
bool
server::host::KeyStore::getKey(String_t keyId, game::v3::RegistrationKey& key)
{
    // Present in index?
    if (!allKeys().contains(keyId)) {
        return false;
    }

    // Blob present? Might be missing if we have a parallel/crashed expireKeys() or addKey().
    String_t blob = keyById(keyId).blob().get();
    if (blob.empty()) {
        return false;
    }

    key.unpackFromBytes(afl::string::toBytes(blob));
    return true;
}

afl::net::redis::StringSetKey
server::host::KeyStore::allKeys()
{
    return m_tree.stringSetKey("all");
}

server::host::KeyStore::Key
server::host::KeyStore::keyById(String_t keyId)
{
    return m_tree.subtree("id").hashKey(keyId);
}

void
server::host::KeyStore::expireKeys()
{
    if (m_maxStoredKeys <= 0) {
        return;
    }

    int32_t numKeys = allKeys().size();
    if (numKeys <= m_maxStoredKeys) {
        return;
    }

    afl::data::StringList_t expiredKeys;
    allKeys().sort()
        .by(keyById("*").lastUsed())
        .limit(0, numKeys - m_maxStoredKeys)
        .getResult(expiredKeys);

    for (size_t i = 0, n = expiredKeys.size(); i < n; ++i) {
        const String_t& keyId = expiredKeys[i];

        // Remove content first, so addKey() will detect that this is a new one
        keyById(keyId).remove();

        // Remove from index
        allKeys().remove(keyId);
    }
}
