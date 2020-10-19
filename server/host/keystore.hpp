/**
  *  \file server/host/keystore.hpp
  *  \brief Class server::host::KeyStore
  */
#ifndef C2NG_SERVER_HOST_KEYSTORE_HPP
#define C2NG_SERVER_HOST_KEYSTORE_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "afl/charset/charset.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/string.hpp"
#include "game/v3/registrationkey.hpp"
#include "server/types.hpp"

namespace server { namespace host {

    struct Configuration;

    /** Server-side registration key store.
        This is an (optional) feature of the host server to help users retrieve their keys.
        KeyStore is intended as a short-lived class to access a user's keys. */
    class KeyStore {
     public:
        /** Information about a key. */
        struct Info {
            // Key meta-info
            String_t keyId;             /**< Key Id. */
            Time_t lastUsed;            /**< Time of last use (see addKey()). */
            int32_t lastGame;           /**< Game of last use (see addKey()). */
            int32_t useCount;           /**< Number of uses so far (number of addKey() calls). */

            // Key content
            bool isRegistered;          /**< Registration status. */
            String_t label1;            /**< Key line 1. */
            String_t label2;            /**< Key line 2. */
        };
        typedef std::vector<Info> Infos_t;

        /** Constructor.
            \param tree Database tree to use (see User::keyStore())
            \param config Configuration (will be copied) */
        KeyStore(afl::net::redis::Subtree tree, const Configuration& config);

        /** Add a key.
            Call whenever a key is used (turn file upload).
            \param key Key
            \param time Time
            \param gameId Game Id to associate with this use */
        void addKey(const game::v3::RegistrationKey& key, Time_t time, int32_t gameId);

        /** Get list of all stored keys.
            \param [out] result Result
            \param [in] charset Character set for decoding blobs */
        void listKeys(Infos_t& result, afl::charset::Charset& charset);

        /** Get key by Id.
            \param [in]  keyId Id
            \param [out] key Key
            \retval true Key successfully retrieved
            \retval false Key was not accessible */
        bool getKey(String_t keyId, game::v3::RegistrationKey& key);

     private:
        /** Database subtree to use. */
        afl::net::redis::Subtree m_tree;

        /** Maximum number of stored keys (configuration option). */
        int m_maxStoredKeys;

        /** Database wrapper: set of all keys. */
        afl::net::redis::StringSetKey allKeys();

        /** Database wrapper: data for one key. */
        class Key;
        Key keyById(String_t keyId);

        /** Expire keys if maximum exceeded. */
        void expireKeys();
    };

} }

#endif
