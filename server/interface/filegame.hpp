/**
  *  \file server/interface/filegame.hpp
  *  \brief Interface server::interface::FileGame
  */
#ifndef C2NG_SERVER_INTERFACE_FILEGAME_HPP
#define C2NG_SERVER_INTERFACE_FILEGAME_HPP

#include <vector>
#include <utility>
#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/data/stringlist.hpp"

namespace server { namespace interface {

    /** Game File Server interface.
        This interface allows access to game-specific data in a filespace. */
    class FileGame : public afl::base::Deletable {
     public:
        /** Slot mapping.
            - first:  slot number
            - second: race name */
        typedef std::pair<int32_t, std::string> Slot_t;

        /** Array of slots. */
        typedef std::vector<Slot_t> Slots_t;

        /** Information about a game. */
        struct GameInfo {
            String_t pathName;                          /**< Path name (path). */
            String_t gameName;                          /**< Game name (name). */
            int32_t gameId;                             /**< Game Id if known (game). */
            int32_t hostTime;                           /**< Next host time (hosttime). */
            bool isFinished;                            /**< true if game is finished (finished). */
            Slots_t slots;                              /**< List of played slots (races). */
            afl::data::StringList_t missingFiles;       /**< List of missing files (missing). */
            afl::data::IntegerList_t conflictSlots;     /**< List of conflicting races (conflict). */

            GameInfo()
                : pathName(), gameName(), gameId(0), hostTime(0), isFinished(false), slots(),
                  missingFiles(), conflictSlots()
                { }
        };

        /** Information about a registration key. */
        struct KeyInfo {
            String_t pathName;                          /**< Directory name (path). */
            String_t fileName;                          /**< File name (file). */
            bool isRegistered;                          /**< true for registered key (reg). */
            String_t label1;                            /**< Key first line (key1). */
            String_t label2;                            /**< Key second line (key2). */
            afl::base::Optional<int32_t> useCount;      /**< Use count (useCount). */
            afl::base::Optional<String_t> keyId;        /**< Key Id (id). */

            KeyInfo()
                : pathName(), fileName(), isRegistered(false), label1(), label2(), useCount(), keyId()
                { }
        };

        /** Filter for key listing. */
        struct Filter {
            afl::base::Optional<String_t> keyId;        /**< Filter by key (ID). */
            bool unique;                                /**< List unique keys (UNIQ). */

            Filter()
                : keyId(), unique(false)
                { }
        };

        /** Get information about single game (STATGAME).
            \param [in]  path   Path
            \param [out] result Information is placed here */
        virtual void getGameInfo(String_t path, GameInfo& result) = 0;

        /** List information about games, recursively (LSGAME).
            \param [in]  path   Path
            \param [out] result List is placed here */
        virtual void listGameInfo(String_t path, afl::container::PtrVector<GameInfo>& result) = 0;

        /** Get information about single key (STATREG).
            \param [in]  path   Path
            \param [out] result Information is placed here */
        virtual void getKeyInfo(String_t path, KeyInfo& result) = 0;

        /** List information about keys, recursively (LSREG).
            \param [in]  path   Path
            \param [in]  filter Filter options
            \param [out] result List is placed here */
        virtual void listKeyInfo(String_t path, const Filter& filter, afl::container::PtrVector<KeyInfo>& result) = 0;
    };

} }

#endif
