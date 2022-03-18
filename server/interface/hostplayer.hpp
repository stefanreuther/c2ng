/**
  *  \file server/interface/hostplayer.hpp
  *  \brief Interface server::interface::HostPlayer
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTPLAYER_HPP
#define C2NG_SERVER_INTERFACE_HOSTPLAYER_HPP

#include <map>
#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/string/string.hpp"

namespace server { namespace interface {

    /** Host Player interface.
        This interface allows manipulating players associated with games. */
    class HostPlayer : public afl::base::Deletable {
     public:
        /** Information about a player slot in a game. */
        struct Info {
            String_t longName;                ///< Long race name.
            String_t shortName;               ///< Short race name.
            String_t adjectiveName;           ///< Adjective race name.
            afl::data::StringList_t userIds;  ///< Players in this slot. First is primary.
            int32_t numEditable;              ///< Number of slots current player can modify.
            bool joinable;                    ///< True if current player can join this slot.

            Info();
        };

        /** File creation permission. */
        enum FileStatus {
            Stale,              ///< Directory is stale, file upload allowed.
            Allow,              ///< File upload allowed, use FileBase.putFile.
            Turn,               ///< Turn file, use HostTurn.submit.
            Refuse              ///< File upload not allowed.
        };


        /** Join a game (PLAYERJOIN).
            \param gameId game Id
            \param slot Slot number
            \param userId user Id */
        virtual void join(int32_t gameId, int32_t slot, String_t userId) = 0;

        /** Set replacement player (PLAYERSUBST).
            \param gameId game Id
            \param slot Slot number
            \param userId user Id */
        virtual void substitute(int32_t gameId, int32_t slot, String_t userId) = 0;

        /** Remove player (PLAYERRESIGN).
            \param gameId game Id
            \param slot Slot number
            \param userId user Id */
        virtual void resign(int32_t gameId, int32_t slot, String_t userId) = 0;

        /** Add player to game (PLAYERADD).
            \param gameId game Id
            \param userId user Id */
        virtual void add(int32_t gameId, String_t userId) = 0;

        /** Get information about all players in a game (PLAYERLS).
            \param [in] gameId game Id
            \param [in] all true to return all current slot, false to return all slots ever
            \param [out] result Result */
        virtual void list(int32_t gameId, bool all, std::map<int,Info>& result) = 0;

        /** Get information about one player slot (PLAYERSTAT).
            \param gameId game Id
            \param slot Slot
            \return information */
        virtual Info getInfo(int32_t gameId, int32_t slot) = 0;

        /** Set directory name for online play (PLAYERSETDIR).
            \param gameId game Id
            \param userId user Id
            \param dirName directory name */
        virtual void setDirectory(int32_t gameId, String_t userId, String_t dirName) = 0;

        /** Get directory name for online play (PLAYERGETDIR).
            \param gameId game Id
            \param userId user Id
            \return directory name */
        virtual String_t getDirectory(int32_t gameId, String_t userId) = 0;

        /** Check file creation permission (PLAYERCHECKFILE).
            \param gameId game Id
            \param userId user Id
            \param fileName file name
            \param dirName directory name
            \return file creation permission */
        virtual FileStatus checkFile(int32_t gameId, String_t userId, String_t fileName, afl::base::Optional<String_t> dirName) = 0;

        /** Set player-specific configuration value (PLAYERSET).
            \param gameId game Id
            \param userId user Id
            \param key Key
            \param value New value */
        virtual void set(int32_t gameId, String_t userId, String_t key, String_t value) = 0;

        /** Get player-specific configuration value (PLAYERGET).
            \param gameId game Id
            \param userId user Id
            \param key Key
            \return value */
        virtual String_t get(int32_t gameId, String_t userId, String_t key) = 0;

        static String_t formatFileStatus(FileStatus st);
        static bool parseFileStatus(const String_t& str, FileStatus& st);
    };

} }

#endif
