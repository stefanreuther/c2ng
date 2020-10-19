/**
  *  \file server/interface/hostkey.hpp
  *  \brief Interface server::interface::HostKey
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTKEY_HPP
#define C2NG_SERVER_INTERFACE_HOSTKEY_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "afl/base/types.hpp"
#include "afl/base/optional.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Host Key Access Interface.
        This interface allows to access stored registration keys.

        Keys are identified by key Ids that are intended to be used on this interface only,
        to allow the implementation to encode information in them.
        That aside, keys are to be treated as untrusted information. */
    class HostKey : public afl::base::Deletable {
     public:
        /** Information about a key. */
        struct Info {
            // Header
            String_t keyId;                             /**< Key Id (id). */
            bool isRegistered;                          /**< true for registered key (reg). */
            String_t label1;                            /**< Key first line (key1). */
            String_t label2;                            /**< Key second line (key2). */

            // Information from file server
            afl::base::Optional<String_t> filePathName; /**< Path name if key present on file server (filePathName). */
            afl::base::Optional<int32_t> fileUseCount;  /**< Number of uses on filer (fileUseCount). */

            // Information from host key store
            afl::base::Optional<int32_t> lastGame;      /**< Last game in which this key was used (game). */
            afl::base::Optional<String_t> lastGameName; /**< Name of game in which this key was last used (gameName). */
            afl::base::Optional<int32_t> gameUseCount;  /**< Number of uses on host (gameUseCount). */
            afl::base::Optional<Time_t> gameLastUsed;   /**< Time when last used on host (gameLastUsed). */

            Info()
                : keyId(), isRegistered(false), label1(), label2(),
                  filePathName(), fileUseCount(),
                  lastGame(), lastGameName(), gameUseCount(), gameLastUsed()
                { }
        };
        typedef std::vector<Info> Infos_t;

        /** Get list of stored keys (KEYLS).
            \param [out] out Information about keys */
        virtual void listKeys(Infos_t& out) = 0;

        /** Get single key (KEYGET).
            \param keyId Key Id */
        virtual String_t getKey(String_t keyId) = 0;
    };

} }

#endif
