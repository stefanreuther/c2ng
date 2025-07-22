/**
  *  \file server/interface/talkpost.hpp
  *  \brief Class server::interface::TalkPost
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

    /** Talk Post interface.
        This interface allows creation and modification of forum postings.
        Posts can create a new thread (create()), or be replies to existing threads (reply()). */
    class TalkPost : public afl::base::Deletable {
     public:
        /** Options for post/thread creation. */
        struct CreateOptions {
            /** Impersonate this user Id (USER).
                For admin use. */
            afl::base::Optional<String_t> userId;

            /** Set these read permissions for the thread (READPERM). */
            afl::base::Optional<String_t> readPermissions;

            /** Set these answer permissions for the thread (ANSWERPERM). */
            afl::base::Optional<String_t> answerPermissions;

            /** Crosspost to these forums. */
            afl::data::IntegerList_t alsoPostTo;
        };

        /** Options for reply creation. */
        struct ReplyOptions {
            /** Impersonate this user Id (USER).
                For admin use. */
            afl::base::Optional<String_t> userId;
        };

        /** Information about a posting. */
        struct Info {
            /** Containing thread Id. */
            int32_t threadId;
            /** Parent post Id; 0 if this is the thread starter. */
            int32_t parentPostId;
            /** Time when post was created. */
            Time_t postTime;
            /** Time when post was last modified. */
            Time_t editTime;
            /** Author (user Id). */
            String_t author;
            /** Subject. */
            String_t subject;
            /** Message-Id for RfC side. */
            String_t rfcMessageId;
        };

        /** Create a new thread and add a posting (POSTNEW).
            @param forumId    Forum Id
            @param subject    Subject/post title
            @param text       Text
            @param options    Options
            @return message Id */
        virtual int32_t create(int32_t forumId, String_t subject, String_t text, const CreateOptions& options) = 0;

        /** Create reply to a message (POSTREPLY).
            @param parentPostId Parent post Id (message we're answering)
            @param subject      Subject/post title
            @param text         Text
            @param options      Options
            @return message Id */
        virtual int32_t reply(int32_t parentPostId, String_t subject, String_t text, const ReplyOptions& options) = 0;

        /** Change an existing posting.
            @param postId Message to update
            @param subj   New subject
            @param text   New text */
        virtual void edit(int32_t postId, String_t subject, String_t text) = 0;

        /** Render a message.
            @param postId Message Id
            @param options Render options
            @return rendered message */
        virtual String_t render(int32_t postId, const TalkRender::Options& options) = 0;

        /** Render multiple messages.
            @param postIds [in]   Message Ids
            @param result  [out]  Rendered messages */
        virtual void render(afl::base::Memory<const int32_t> postIds, afl::data::StringList_t& result) = 0;

        /** Get information about a message.
            @param postId Message Id
            @return Information */
        virtual Info getInfo(int32_t postId) = 0;

        /** Get information about multiple messages.
            @param postIds [in]   Message Ids
            @param result  [out]  Information */
        virtual void getInfo(afl::base::Memory<const int32_t> postIds, afl::container::PtrVector<Info>& result) = 0;

        /** Get header field.
            @param postIds Message Id
            @param fieldName Header field name
            @return header field value */
        virtual String_t getHeaderField(int32_t postId, String_t fieldName) = 0;

        /** Remove a posting.
            @param postId Message Id
            @return true if message was removed; false if it did not exist */
        virtual bool remove(int32_t postId) = 0;

        /** List newwst postings.
            @param count    [in]   Number of postings to return
            @param postIds  [out]  List of posting Ids */
        virtual void getNewest(int count, afl::data::IntegerList_t& postIds) = 0;
    };

} }

#endif
