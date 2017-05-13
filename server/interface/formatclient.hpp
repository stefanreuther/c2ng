/**
  *  \file server/interface/formatclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_FORMATCLIENT_HPP
#define C2NG_SERVER_INTERFACE_FORMATCLIENT_HPP

#include "server/interface/format.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class FormatClient : public Format {
     public:
        FormatClient(afl::net::CommandHandler& commandHandler);
        ~FormatClient();

        virtual afl::data::Value* pack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset);
        virtual afl::data::Value* unpack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
