/**
  *  \file server/interface/talkgroupclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKGROUPCLIENT_HPP
#define C2NG_SERVER_INTERFACE_TALKGROUPCLIENT_HPP

#include "server/interface/talkgroup.hpp"
#include "afl/net/commandhandler.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class TalkGroupClient : public TalkGroup {
     public:
        TalkGroupClient(afl::net::CommandHandler& commandHandler);
        ~TalkGroupClient();

        virtual void add(String_t groupId, const Description& info);
        virtual void set(String_t groupId, const Description& info);
        virtual String_t getField(String_t groupId, String_t fieldName);
        virtual void list(String_t groupId, afl::data::StringList_t& groups, afl::data::IntegerList_t& forums);
        virtual Description getDescription(String_t groupId);
        virtual void getDescriptions(const afl::data::StringList_t& groups, afl::container::PtrVector<Description>& results);

        static void packDescription(afl::data::Segment& command, const Description& info);
        static Description unpackDescription(const Value_t* value);
        
     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
