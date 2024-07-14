/**
  *  \file server/talk/accesschecker.hpp
  *  \brief Class server::talk::AccessChecker
  */
#ifndef C2NG_SERVER_TALK_ACCESSCHECKER_HPP
#define C2NG_SERVER_TALK_ACCESSCHECKER_HPP

#include "afl/base/types.hpp"

namespace server { namespace talk {

    class Message;
    class Topic;
    class Root;
    class Session;

    /** Forum message access permission checker.

        Message access permissions are mainly determined by topics.
        This class optimizes by combining consecutive accesses to the same topic.
        Otherwise, it is intended to be used as temporary object. */
    class AccessChecker {
     public:
        /** Constructor.
            @param root    Service root
            @param session Session */
        AccessChecker(Root& root, Session& session);

        /** Get access permission for message.
            @param m Message
            @return true if message is accessible */
        bool isAllowed(Message& m);

        /** Get access permission for topic.
            @param t topic
            @return true if topic is accessible */
        bool isAllowed(Topic& t);

        /** Check access permission for message.
            @param m Message
            @throw std::runtime_error if message is not accessible */
        void checkMessage(Message& m);

        /** Check access permission for topic.
            @param t topic
            @throw std::runtime_error if message is not accessible */
        void checkTopic(Topic& t);

     private:
        Root& m_root;
        Session& m_session;

        int32_t m_lastTopicId;
        bool m_lastTopicPermitted;
    };

} }

#endif
