/**
  *  \file server/talk/notify.hpp
  *  \brief Notification Generation
  */
#ifndef C2NG_SERVER_TALK_NOTIFY_HPP
#define C2NG_SERVER_TALK_NOTIFY_HPP

#include "afl/data/stringlist.hpp"
#include "server/interface/mailqueue.hpp"

namespace server { namespace talk {

    class Forum;
    class Message;
    class Root;
    class Topic;
    class UserPM;

    /** Notify a forum message.
        Sends mail to all users observing this topic or forum.
        \param msg Forum message
        \param root Service root
        \param mq Mail queue */
    void notifyMessage(Message& msg, Root& root, server::interface::MailQueue& mq);


    /** Notify a private message.
        \param msg Private message
        \param notifyIndividual Users that receive individual message notifications
        \param notifyGroup Users that receive group notifications
        \param root Service root
        \param mq Mail queue */
    void notifyPM(UserPM& msg, const afl::data::StringList_t& notifyIndividual, const afl::data::StringList_t& notifyGroup, Root& root, server::interface::MailQueue& mq);

} }

#endif
