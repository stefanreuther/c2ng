/**
  *  \file test/game/proxy/mailboxadaptortest.cpp
  *  \brief Test for game::proxy::MailboxAdaptor
  */

#include "game/proxy/mailboxadaptor.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.proxy.MailboxAdaptor")
{
    class Tester : public game::proxy::MailboxAdaptor {
     public:
        virtual game::Session& session() const
            { throw 0; }
        virtual game::msg::Mailbox& mailbox() const
            { throw 0; }
        virtual game::msg::Configuration* getConfiguration() const
            { return 0; }
        virtual size_t getCurrentMessage() const
            { return 0; }
        virtual void setCurrentMessage(size_t /*n*/)
            { }
    };
    Tester t;
}
