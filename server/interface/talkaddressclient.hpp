/**
  *  \file server/interface/talkaddressclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKADDRESSCLIENT_HPP
#define C2NG_SERVER_INTERFACE_TALKADDRESSCLIENT_HPP

#include "server/interface/talkaddress.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class TalkAddressClient : public TalkAddress {
     public:
        TalkAddressClient(afl::net::CommandHandler& commandHandler);
        ~TalkAddressClient();

        virtual void parse(afl::base::Memory<const String_t> in, afl::data::StringList_t& out);
        virtual void render(afl::base::Memory<const String_t> in, afl::data::StringList_t& out);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
