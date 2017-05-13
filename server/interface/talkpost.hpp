/**
  *  \file server/interface/talkpost.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKPOST_HPP
#define C2NG_SERVER_INTERFACE_TALKPOST_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/string/string.hpp"
#include "server/interface/talkrender.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class TalkPost : public afl::base::Deletable {
     public:
        struct CreateOptions {
            afl::base::Optional<String_t> userId;
            afl::base::Optional<String_t> readPermissions;
            afl::base::Optional<String_t> answerPermissions;
        };

        struct ReplyOptions {
            afl::base::Optional<String_t> userId;
        };

        struct Info {
            int32_t threadId;
            int32_t parentPostId;
            Time_t postTime;
            Time_t editTime;
            String_t author;
            String_t subject;
            String_t rfcMessageId;
        };
        
        // POSTNEW forum:FID subj:Str text:TalkText [USER user:UID] [READPERM rp:Str] [ANSWERPERM ap:Str]
        // @retval MID new message Id
        virtual int32_t create(int32_t forumId, String_t subject, String_t text, const CreateOptions& options) = 0;

        // POSTREPLY parent:MID subj:Str text:TalkText [USER user:UID]
        // @retval MID new message Id
        virtual int32_t reply(int32_t parentPostId, String_t subject, String_t text, const ReplyOptions& options) = 0;

        // POSTEDIT msg:MID subj:Str text:TalkText
        virtual void edit(int32_t postId, String_t subject, String_t text) = 0;

        // POSTRENDER msg:MID [renderOptions...]
        // @retval Str rendered posting
        virtual String_t render(int32_t postId, const TalkRender::Options& options) = 0;

        // POSTMRENDER msg:MID...
        // @retval StrList rendered postings
        virtual void render(afl::base::Memory<const int32_t> postIds, afl::data::StringList_t& result) = 0;

        // POSTSTAT msg:MID
        // @retval TalkPostInfo information about posting
        virtual Info getInfo(int32_t postId) = 0;

        // POSTMSTAT msg:MID...
        // @retval TalkPostInfo[]
        virtual void getInfo(afl::base::Memory<const int32_t> postIds, afl::container::PtrVector<Info>& result) = 0;

        // POSTGET msg:MID key:Str
        // @retval Any result (string, Id, Time, etc.)
        virtual String_t getHeaderField(int32_t postId, String_t fieldName) = 0;

        // POSTRM msg:MID
        // @retval Int 1=removed, 0=not removed, posting did not exist
        virtual bool remove(int32_t postId) = 0;

        // POSTLSNEW n:Int (Talk Command)
        virtual void getNewest(int count, afl::data::IntegerList_t& postIds) = 0;
    };

} }

#endif
