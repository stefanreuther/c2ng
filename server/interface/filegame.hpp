/**
  *  \file server/interface/filegame.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_FILEGAME_HPP
#define C2NG_SERVER_INTERFACE_FILEGAME_HPP

#include <vector>
#include <utility>
#include "afl/base/deletable.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/container/ptrvector.hpp"

namespace server { namespace interface {

    class FileGame : public afl::base::Deletable {
     public:
        typedef std::pair<int32_t, std::string> Slot_t;
        typedef std::vector<Slot_t> Slots_t;
        struct GameInfo {
            String_t pathName;
            String_t gameName;
            int32_t gameId;
            int32_t hostTime;
            bool isFinished;
            Slots_t slots;
            afl::data::StringList_t missingFiles;
            afl::data::IntegerList_t conflictSlots;

            GameInfo()
                : pathName(), gameName(), gameId(0), hostTime(0), isFinished(false), slots(),
                  missingFiles(), conflictSlots()
                { }
        };
        struct KeyInfo {
            String_t pathName;
            String_t fileName;
            bool isRegistered;
            String_t label1;
            String_t label2;

            KeyInfo()
                : pathName(), fileName(), isRegistered(false), label1(), label2()
                { }
        };

        virtual void getGameInfo(String_t path, GameInfo& result) = 0;
        virtual void listGameInfo(String_t path, afl::container::PtrVector<GameInfo>& result) = 0;
        virtual void getKeyInfo(String_t path, KeyInfo& result) = 0;
        virtual void listKeyInfo(String_t path, afl::container::PtrVector<KeyInfo>& result) = 0;
    };

} }

#endif
