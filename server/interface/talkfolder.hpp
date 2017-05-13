/**
  *  \file server/interface/talkfolder.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKFOLDER_HPP
#define C2NG_SERVER_INTERFACE_TALKFOLDER_HPP

#include "afl/base/deletable.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/base/memory.hpp"
#include "afl/container/ptrvector.hpp"
#include "server/interface/talkforum.hpp"

namespace server { namespace interface {

    class TalkFolder : public afl::base::Deletable {
     public:
        struct Info {
            String_t name;
            String_t description;
            int32_t numMessages;
            bool hasUnreadMessages;
            bool isFixedFolder;
        };

        typedef TalkForum::ListParameters ListParameters;
        
        virtual void getFolders(afl::data::IntegerList_t& result) = 0;
        virtual Info getInfo(int32_t ufid) = 0;
        virtual void getInfo(afl::base::Memory<const int32_t> ufids, afl::container::PtrVector<Info>& results) = 0;
        virtual int32_t create(String_t name, afl::base::Memory<const String_t> args) = 0;
        virtual bool remove(int32_t ufid) = 0;
        virtual void configure(int32_t ufid, afl::base::Memory<const String_t> args) = 0;
        virtual afl::data::Value* getPMs(int32_t ufid, const ListParameters& params) = 0;
    };

} }

#endif
