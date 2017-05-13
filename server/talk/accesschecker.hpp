/**
  *  \file server/talk/accesschecker.hpp
  */
#ifndef C2NG_SERVER_TALK_ACCESSCHECKER_HPP
#define C2NG_SERVER_TALK_ACCESSCHECKER_HPP

#include "afl/base/types.hpp"

namespace server { namespace talk {

    class Message;
    class Topic;
    class Root;
    class Session;

    class AccessChecker {
     public:
        AccessChecker(Root& root, Session& session);

        bool isAllowed(Message& m);
        bool isAllowed(Topic& t);
        void checkMessage(Message& m);
        void checkTopic(Topic& t);
     private:
        Root& m_root;
        Session& m_session;

        int32_t m_lastTopicId;
        bool m_lastTopicPermitted;
    };

} }

#endif
