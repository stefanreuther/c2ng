/**
  *  \file server/interface/talkpostclient.hpp
  *  \brief Class server::interface::TalkPostClient
  */
#ifndef C2NG_SERVER_INTERFACE_TALKPOSTCLIENT_HPP
#define C2NG_SERVER_INTERFACE_TALKPOSTCLIENT_HPP

#include "server/interface/talkpost.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    /** Client for TalkPost interface.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class TalkPostClient : public TalkPost {
     public:
        /** Constructor.
            @param commandHandler Server connection. Lifetime must exceed that of the TalkPostClient. */
        explicit TalkPostClient(afl::net::CommandHandler& commandHandler);

        // TalkPost:
        virtual int32_t create(int32_t forumId, String_t subject, String_t text, const CreateOptions& options);
        virtual int32_t reply(int32_t parentPostId, String_t subject, String_t text, const ReplyOptions& options);
        virtual void edit(int32_t postId, String_t subject, String_t text);
        virtual String_t render(int32_t postId, const TalkRender::Options& options);
        virtual void render(afl::base::Memory<const int32_t> postIds, afl::data::StringList_t& result);
        virtual Info getInfo(int32_t postId);
        virtual void getInfo(afl::base::Memory<const int32_t> postIds, afl::container::PtrVector<Info>& result);
        virtual String_t getHeaderField(int32_t postId, String_t fieldName);
        virtual bool remove(int32_t postId);
        virtual void getNewest(int count, afl::data::IntegerList_t& postIds);

        /** Deserialize a TalkPost::Info.
            @param value Data received from server
            @return deserialized information */
        static Info unpackInfo(const afl::data::Value* value);

     private:
        afl::net::CommandHandler& m_commandHandler;

    };

} }

#endif
