/**
  *  \file server/interface/formatserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_FORMATSERVER_HPP
#define C2NG_SERVER_INTERFACE_FORMATSERVER_HPP

#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class Format;

    class FormatServer : public afl::net::CommandHandler {
     public:
        FormatServer(Format& impl);
        ~FormatServer();

        virtual Value_t* call(const Segment_t& command);
        virtual void callVoid(const Segment_t& command);

     private:
        Format& m_implementation;
    };

} }

#endif
