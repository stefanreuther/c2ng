/**
  *  \file server/interface/mailqueueserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_MAILQUEUESERVER_HPP
#define C2NG_SERVER_INTERFACE_MAILQUEUESERVER_HPP

#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class MailQueue;

    class MailQueueServer : public afl::net::CommandHandler {
     public:
        MailQueueServer(MailQueue& impl);
        ~MailQueueServer();

        virtual Value_t* call(const Segment_t& command);
        virtual void callVoid(const Segment_t& command);

     private:
        MailQueue& m_implementation;
    };

} }

#endif
