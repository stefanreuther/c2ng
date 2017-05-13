/**
  *  \file server/interface/talkforum.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKFORUM_HPP
#define C2NG_SERVER_INTERFACE_TALKFORUM_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/string/string.hpp"
#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/data/value.hpp"
#include "afl/base/optional.hpp"

namespace server { namespace interface {

    class TalkForum : public afl::base::Deletable {
     public:
        struct Info {
            String_t name;
            String_t parentGroup;
            String_t description;
            String_t newsgroupName;
        };

        struct Size {
            int32_t numThreads;
            int32_t numStickyThreads;
            int32_t numMessages;
        };

        struct ListParameters {
            enum Mode {
                WantAll,        // get the whole list.
                WantRange,      // get subrange of list. Uses start+count.
                WantSize,       // get size of list.
                WantMemberCheck // check presence of an item. Uses item.
            };
            Mode mode;
            int32_t start;
            int32_t count;
            int32_t item;
            afl::base::Optional<String_t> sortKey;

            ListParameters()
                : mode(WantAll),
                  start(0),
                  count(0),
                  item(0),
                  sortKey()
                { }
        };

        // FORUMADD [key:Str value:Str ...] (Talk Command)
        // @retval FID new forum Id
        virtual int32_t add(afl::base::Memory<const String_t> config) = 0;

        // FORUMSET forum:FID [key:Str value:Str ...] (Talk Command)
        virtual void configure(int32_t fid, afl::base::Memory<const String_t> config) = 0;

        // FORUMGET forum:FID key:Str (Talk Command)
        // @rettype Str, GRID, TalkText, TalkPerm, Time, Int
        virtual afl::data::Value* getValue(int32_t fid, String_t keyName) = 0;

        // FORUMSTAT forum:FID (Talk Command)
        // @retval TalkForumInfo information about forum
        virtual Info getInfo(int32_t fid) = 0;

        // FORUMMSTAT forum:FID... (Talk Command)
        // @retval TalkForumInfo[] information about forum
        virtual void getInfo(afl::base::Memory<const int32_t> fids, afl::container::PtrVector<Info>& result) = 0;

        // FORUMPERMS forum:FID [perm:Str ...] (Talk Command)
        // @retval Int permissions
        // FIXME: can we find a better interface?
        virtual int32_t getPermissions(int32_t fid, afl::base::Memory<const String_t> permissionList) = 0;

        // FORUMSIZE forum:FID (Talk Command)
        virtual Size getSize(int32_t fid) = 0;

        // FORUMLSTHREAD forum:FID [listParameters...] (Talk Command)
        // @rettype Any, TID
        virtual afl::data::Value* getThreads(int32_t fid, const ListParameters& params) = 0;

        // FORUMLSSTICKY forum:FID [listParameters...] (Talk Command)
        // @rettype Any, TID
        virtual afl::data::Value* getStickyThreads(int32_t fid, const ListParameters& params) = 0;

        // FORUMLSPOST forum:FID [listParameters...] (Talk Command)
        virtual afl::data::Value* getPosts(int32_t fid, const ListParameters& params) = 0;

        int32_t getIntegerValue(int32_t fid, String_t keyName);
        String_t getStringValue(int32_t fid, String_t keyName);
    };

} }

#endif
