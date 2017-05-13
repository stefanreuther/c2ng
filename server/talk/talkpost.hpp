/**
  *  \file server/talk/talkpost.hpp
  *  \brief Class server::talk::TalkPost
  */
#ifndef C2NG_SERVER_TALK_TALKPOST_HPP
#define C2NG_SERVER_TALK_TALKPOST_HPP

#include "server/interface/talkpost.hpp"

namespace server { namespace talk {

    class Session;
    class Root;

    /** Implementation of POST commands. */
    class TalkPost : public server::interface::TalkPost {
     public:
        /** Constructor.
            \param session Session
            \param root Service root */
        TalkPost(Session& session, Root& root);

        // TalkPost interface:
        virtual int32_t create(int32_t forumId, String_t subject, String_t text, const CreateOptions& options);
        virtual int32_t reply(int32_t parentPostId, String_t subject, String_t text, const ReplyOptions& options);
        virtual void edit(int32_t postId, String_t subject, String_t text);
        virtual String_t render(int32_t postId, const server::interface::TalkRender::Options& options);
        virtual void render(afl::base::Memory<const int32_t> postIds, afl::data::StringList_t& result);
        virtual Info getInfo(int32_t postId);
        virtual void getInfo(afl::base::Memory<const int32_t> postIds, afl::container::PtrVector<Info>& result);
        virtual String_t getHeaderField(int32_t postId, String_t fieldName);
        virtual bool remove(int32_t postId);
        virtual void getNewest(int count, afl::data::IntegerList_t& postIds);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
