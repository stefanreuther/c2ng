/**
  *  \file u/t_server_mailin_mailprocessor.cpp
  *  \brief Test for server::mailin::MailProcessor
  */

#include <map>
#include <stdexcept>
#include "server/mailin/mailprocessor.hpp"

#include "t_server_mailin.hpp"
#include "afl/container/ptrqueue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/net/mimeparser.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/log.hpp"
#include "server/errors.hpp"
#include "server/interface/composablecommandhandler.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::net::MimeParser;
using server::makeIntegerValue;
using server::makeStringValue;

namespace {
    /*
     *  Host Mock
     *
     *  This emulates the necessary host commands.
     *  It can operate in three modes:
     *  - Dead (default): do not expect any host calls
     *  - Failure: respond to turn upload calls with an error, do not expect other calls
     *  - Success: respond to turn upload calls with success, answer other calls
     */
    class HostMock : public server::interface::ComposableCommandHandler {
     public:
        HostMock()
            : m_mode(Dead), m_gameId(), m_slot(), m_state(), m_user(), m_error()
            { }
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);
        void setFailure(String_t msg)
            { m_mode = Failure; m_error = msg; }
        void setSuccess(int32_t gameId, int32_t slot, int32_t state, String_t user)
            { m_mode = Success; m_gameId = gameId; m_slot = slot; m_state = state; m_user = user; }

     private:
        enum Mode { Dead, Failure, Success } m_mode;
        int32_t m_gameId;
        int32_t m_slot;
        int32_t m_state;
        String_t m_user;
        String_t m_error;
    };

    /*
     *  Mail Mock
     *
     *  This simulates a mail queue.
     *  It verifies the command sequence.
     *  It stashes away received messages.
     *  It takes a few simplifications for our purposes.
     */
    class MailMock : public server::interface::MailQueue {
     public:
        struct Message {
            String_t templateName;
            std::map<String_t, String_t> parameters;
            String_t receiver;
        };
        MailMock()
            : m_current(), m_queue()
            { }
        // MailQueue methods:
        virtual void startMessage(String_t templateName, afl::base::Optional<String_t> uniqueId);
        virtual void addParameter(String_t parameterName, String_t value);
        virtual void addAttachment(String_t /*url*/)
            { TS_ASSERT(0); }
        virtual void send(afl::base::Memory<const String_t> receivers);
        virtual void cancelMessage(String_t /*uniqueId*/)
            { TS_ASSERT(0); }
        virtual void confirmAddress(String_t /*address*/, String_t /*key*/, afl::base::Optional<String_t> /*info*/)
            { TS_ASSERT(0); }
        virtual void requestAddress(String_t /*user*/)
            { TS_ASSERT(0); }
        virtual void runQueue()
            { TS_ASSERT(0); }
        virtual UserStatus getUserStatus(String_t /*user*/)
            { TS_ASSERT(0); return UserStatus(); }

        Message* extract();
        bool empty() const;

     private:
        std::auto_ptr<Message> m_current;
        afl::container::PtrQueue<Message> m_queue;
    };

    /*
     *  Some standard mails
     */

    afl::base::ConstBytes_t getSimpleTurnMail()
    {
        return afl::string::toBytes("From stefan@rocket.streu.home Wed Sep 27 18:36:28 2017\n"
                                    "Return-path: <stefan@rocket.streu.home>\n"
                                    "Envelope-to: stefan@localhost\n"
                                    "Delivery-date: Wed, 27 Sep 2017 18:36:28 +0200\n"
                                    "Received: from stefan by rocket.speedport.ip with local (Exim 4.84)\n"
                                    "        (envelope-from <stefan@rocket.streu.home>)\n"
                                    "        id 1dxFK0-0001ao-De\n"
                                    "        for stefan@localhost; Wed, 27 Sep 2017 18:36:28 +0200\n"
                                    "Date: Wed, 27 Sep 2017 18:36:28 +0200\n"
                                    "From: Stefan Reuther <stefan@localhost>\n"
                                    "To: stefan@localhost\n"
                                    "Subject: test\n"
                                    "Message-ID: <20170927163628.GA6110@rocket.streu.home>\n"
                                    "MIME-Version: 1.0\n"
                                    "Content-Type: multipart/mixed; boundary=\"LZvS9be/3tNcYl/X\"\n"
                                    "Content-Disposition: inline\n"
                                    "User-Agent: Mutt/1.5.23 (2014-03-12)\n"
                                    "Status: RO\n"
                                    "Content-Length: 1085\n"
                                    "Lines: 26\n"
                                    "\n"
                                    "\n"
                                    "--LZvS9be/3tNcYl/X\n"
                                    "Content-Type: text/plain; charset=us-ascii\n"
                                    "Content-Disposition: inline\n"
                                    "\n"
                                    "the mail\n"
                                    "\n"
                                    "--LZvS9be/3tNcYl/X\n"
                                    "Content-Type: application/octet-stream\n"
                                    "Content-Disposition: attachment; filename=\"player2.trn\"\n"
                                    "Content-Transfer-Encoding: base64\n"
                                    "\n"
                                    "AgAAAAAAMDYtMDItMjAxMjE5OjA3OjA0AACQA1ZFUjMuNTAxNo7TcqllgGnHrFJME0KOeQny\n"
                                    "TAxhU8wa5KB+1+CaF/KnlNg3KnIsritshzhkBCHsOsSAXvfAujfSh5LoWN5apmZIbQtpzAmc\n"
                                    "Nu06Bwsaf/UDRZ3Wmj2tPsMIZE1MDL5k/ViUGicrOl/VI2W9Q2xpZW50OiBQbGFuZXRzIENv\n"
                                    "bW1hbmQgQ2VudGVyIElJICh2Mi4wLjIpICAgICAgICBodHRwOi8vcGhvc3QuZGUvfnN0ZWZh\n"
                                    "bi9wY2MyLmh0bWwgICAgICAgICAgICAgICAgIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
                                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
                                    "AAAAAAAAAAAAAAAAAAAAAAAAAADKXgAAUENDMjcEAADICwAAYw8AALgUAAChGAAAhCEAAGAL\n"
                                    "AABQIQAAKS4AAGo7AADMQAAAYD8AAK1CAAAMUQAAYBgAAAAaAACgGwAAQB0AAOAeAACAIAAA\n"
                                    "ICIAAMAjAABgJQAAACcAAKAoAAB0AwAAlAsAAGMPAABcFwAAZBkAAMYeAAAaJwAA4BEAAKAO\n"
                                    "AAAOJAAAazgAAHhFAAD1RwAA9kQAAMpTAABQYgAAoBsAAEAdAADgHgAAgCAAACAiAADAIwAA\n"
                                    "YCUAAAAnAACgKAAAJE4HAAAAAAAzYAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
                                    "AAAAAAAA\n"
                                    "\n"
                                    "--LZvS9be/3tNcYl/X--\n"
                                    "\n");
    }
}

/*
 *  HostMock
 */

bool
HostMock::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& /*args*/, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "TRN") {
        switch (m_mode) {
         case Dead:
            TS_ASSERT(0);
            break;

         case Failure:
            throw std::runtime_error(m_error);
            break;

         case Success:
            Hash::Ref_t h = Hash::create();
            h->setNew("status",   makeIntegerValue(m_state));
            h->setNew("output",   makeStringValue("output..."));
            h->setNew("game",     makeIntegerValue(m_gameId));
            h->setNew("slot",     makeIntegerValue(m_slot));
            h->setNew("previous", makeIntegerValue(0));
            h->setNew("user",     makeStringValue(m_user));
            h->setNew("name",     makeStringValue(afl::string::Format("Game %d", m_gameId)));
            h->setNew("turn",     makeIntegerValue(75));
            h->setNew("allowtemp", makeIntegerValue(1));
            result.reset(new HashValue(h));
            break;
        }
        return true;
    }
    return false;
}

/*
 *  MailMock
 */

void
MailMock::startMessage(String_t templateName, afl::base::Optional<String_t> /*uniqueId*/)
{
    TS_ASSERT(m_current.get() == 0);
    m_current.reset(new Message());
    m_current->templateName = templateName;
}

void
MailMock::addParameter(String_t parameterName, String_t value)
{
    TS_ASSERT(m_current.get() != 0);
    TS_ASSERT(m_current->parameters.find(parameterName) == m_current->parameters.end());
    m_current->parameters.insert(std::make_pair(parameterName, value));
}

void
MailMock::send(afl::base::Memory<const String_t> receivers)
{
    TS_ASSERT(m_current.get() != 0);
    TS_ASSERT_EQUALS(receivers.size(), 1U);
    m_current->receiver = *receivers.at(0);
    m_queue.pushBackNew(m_current.release());
}

MailMock::Message*
MailMock::extract()
{
    return m_queue.extractFront();
}

bool
MailMock::empty() const
{
    return m_queue.empty() && m_current.get() == 0;
}

/*
 *  Helper
 */

namespace {
    bool processMail(afl::base::ConstBytes_t text, MailMock& mail, HostMock& host)
    {
        // Parse the mail
        MimeParser p;
        p.handleFullData(text);
        p.finish();

        // Process mail
        afl::sys::Log log;
        return server::mailin::MailProcessor(log, mail, host).process(p);
    }
}


/********************* TestServerMailinMailProcessor *********************/


/** Test simple mail without attachment. */
void
TestServerMailinMailProcessor::testSimple()
{
    // Process mail
    MailMock mail;
    HostMock host;
    TS_ASSERT(!processMail(afl::string::toBytes("From: user <user@host>\n"
                                                "To: host@localhost\n"
                                                "Subject: whatever\n"
                                                "\n"
                                                "Some text here.\n"),
                           mail, host));

    // Verify result
    TS_ASSERT(mail.empty());
}

/** Test successful turn submission.
    "Successful" means I have extracted the turn file and sent it to host.
    There is no difference between different results.
    That is solved using mail templates. */
void
TestServerMailinMailProcessor::testTurn()
{
    // Process mail
    MailMock mail;
    HostMock host;
    host.setSuccess(32, 3, 1, "uu");
    TS_ASSERT(processMail(getSimpleTurnMail(), mail, host));

    // Verify result
    std::auto_ptr<MailMock::Message> m(mail.extract());
    TS_ASSERT(m.get() != 0);
    TS_ASSERT_EQUALS(m->templateName, "turn");
    TS_ASSERT_EQUALS(m->receiver, "user:uu");
    TS_ASSERT_EQUALS(m->parameters["trn_status"], "1");
    TS_ASSERT_EQUALS(m->parameters["trn_output"], "output...");
    TS_ASSERT_EQUALS(m->parameters["gameid"], "32");
    TS_ASSERT_EQUALS(m->parameters["gameturn"], "75");
    TS_ASSERT_EQUALS(m->parameters["gamename"], "Game 32");
    TS_ASSERT_EQUALS(m->parameters["mail_subject"], "test");

    // No more mail
    TS_ASSERT(mail.empty());
}

/** Test turn submission with a 407 error.
    This happens if host cannot associate an email address with the game. */
void
TestServerMailinMailProcessor::testError407()
{
    // Process mail
    MailMock mail;
    HostMock host;
    host.setFailure(server::TRN_MAIL_MISMATCH);
    TS_ASSERT(processMail(getSimpleTurnMail(), mail, host));

    // Verify result
    std::auto_ptr<MailMock::Message> m(mail.extract());
    TS_ASSERT(m.get() != 0);
    TS_ASSERT_EQUALS(m->templateName, "turn-mismatch");
    TS_ASSERT_EQUALS(m->receiver, "mail:stefan@localhost");
    TS_ASSERT_EQUALS(m->parameters["mail_subject"], "test");

    // No more mail
    TS_ASSERT(mail.empty());
}

/** Test turn submission with a 404 error.
    This happens if the timestamp in the turn ist not known to the system. */
void
TestServerMailinMailProcessor::testError404()
{
    // Process mail
    MailMock mail;
    HostMock host;
    host.setFailure(server::GAME_NOT_FOUND);
    TS_ASSERT(processMail(getSimpleTurnMail(), mail, host));

    // Verify result
    std::auto_ptr<MailMock::Message> m(mail.extract());
    TS_ASSERT(m.get() != 0);
    TS_ASSERT_EQUALS(m->templateName, "turn-stale");
    TS_ASSERT_EQUALS(m->receiver, "mail:stefan@localhost");
    TS_ASSERT_EQUALS(m->parameters["mail_subject"], "test");

    // No more mail
    TS_ASSERT(mail.empty());
}

/** Test turn submission with a 412 error.
    This happens if a turn is submitted for a game that is not running. */
void
TestServerMailinMailProcessor::testError412()
{
    // Process mail
    MailMock mail;
    HostMock host;
    host.setFailure(server::WRONG_GAME_STATE);
    TS_ASSERT(processMail(getSimpleTurnMail(), mail, host));

    // Verify result
    std::auto_ptr<MailMock::Message> m(mail.extract());
    TS_ASSERT(m.get() != 0);
    TS_ASSERT_EQUALS(m->templateName, "turn-stale");
    TS_ASSERT_EQUALS(m->receiver, "mail:stefan@localhost");
    TS_ASSERT_EQUALS(m->parameters["mail_subject"], "test");

    // No more mail
    TS_ASSERT(mail.empty());
}

/** Test turn submission with a 422 error.
    This happens if the turn fails to parse. */
void
TestServerMailinMailProcessor::testError422()
{
    // Process mail
    MailMock mail;
    HostMock host;
    host.setFailure(server::INVALID_FILE_FORMAT);
    TS_ASSERT(processMail(getSimpleTurnMail(), mail, host));

    // Verify result
    std::auto_ptr<MailMock::Message> m(mail.extract());
    TS_ASSERT(m.get() != 0);
    TS_ASSERT_EQUALS(m->templateName, "turn-error");
    TS_ASSERT_EQUALS(m->receiver, "mail:stefan@localhost");
    TS_ASSERT_EQUALS(m->parameters["mail_subject"], "test");

    // No more mail
    TS_ASSERT(mail.empty());
}

/** Test turn submission with another error. */
void
TestServerMailinMailProcessor::testErrorOther()
{
    // Process mail
    MailMock mail;
    HostMock host;
    host.setFailure(server::GAME_IN_USE);
    TS_ASSERT(!processMail(getSimpleTurnMail(), mail, host));

    // No mail sent
    TS_ASSERT(mail.empty());
}

/** Test turn submission with multiple turns in one mail. */
void
TestServerMailinMailProcessor::testMultiple()
{
    // Process mail. This mail has three attachments, two of them turn files
    // (exercise variance in file names while we are at it).
    MailMock mail;
    HostMock host;
    host.setSuccess(47, 3, 1, "uu");
    TS_ASSERT(processMail(afl::string::toBytes("From: a@b\n"
                                               "To: c@d\n"
                                               "Subject: multi\n"
                                               "Content-Type: multipart/mixed; boundary=\"xxx\"\n"
                                               "Content-Disposition: inline\n"
                                               "\n"
                                               "\n"
                                               "--xxx\n"
                                               "Content-Type: application/octet-stream\n"
                                               "Content-Disposition: attachment; filename=\"player2.trn\"\n"
                                               "Content-Transfer-Encoding: base64\n"
                                               "\n"
                                               "AgAAAAAAMDYtMDItMjAxMjE5OjA3OjA0AACQA1ZFUjMuNTAxNo7TcqllgGnHrFJME0KOeQny\n"
                                               "--xxx\n"
                                               "Content-Type: application/octet-stream\n"
                                               "Content-Disposition: attachment; filename=\"player3.doc\"\n"
                                               "Content-Transfer-Encoding: base64\n"
                                               "\n"
                                               "AgAAAAAAMDYtMDItMjAxMjE5OjA3OjA0AACQA1ZFUjMuNTAxNo7TcqllgGnHrFJME0KOeQny\n"
                                               "--xxx\n"
                                               "Content-Type: application/octet-stream\n"
                                               "Content-Disposition: attachment; filename=\"PLAYER4.TRN\"\n"
                                               "Content-Transfer-Encoding: base64\n"
                                               "\n"
                                               "AgAAAAAAMDYtMDItMjAxMjE5OjA3OjA0AACQA1ZFUjMuNTAxNo7TcqllgGnHrFJME0KOeQny\n"
                                               "--xxx--\n\n"),
                          mail, host));

    // Verify
    std::auto_ptr<MailMock::Message> m(mail.extract());
    TS_ASSERT(m.get() != 0);
    TS_ASSERT_EQUALS(m->templateName, "turn");
    TS_ASSERT_EQUALS(m->receiver, "user:uu");
    TS_ASSERT_EQUALS(m->parameters["gameid"], "47");
    TS_ASSERT_EQUALS(m->parameters["gamename"], "Game 47");
    TS_ASSERT_EQUALS(m->parameters["mail_subject"], "multi");
    TS_ASSERT_EQUALS(m->parameters["mail_path"], "/part1/player2.trn");

    // Second part
    m.reset(mail.extract());
    TS_ASSERT(m.get() != 0);
    TS_ASSERT_EQUALS(m->templateName, "turn");
    TS_ASSERT_EQUALS(m->receiver, "user:uu");
    TS_ASSERT_EQUALS(m->parameters["mail_path"], "/part3/player4.trn");  // name is normalized

    // No more parts
    TS_ASSERT(mail.empty());
}

/** Test turn submission, nested attachments. */
void
TestServerMailinMailProcessor::testNested()
{
    // Process mail. This mail has been created by forwarding a mail three times with mutt (and shortened a bit).
    MailMock mail;
    HostMock host;
    host.setSuccess(47, 3, 1, "qq");
    TS_ASSERT(processMail(afl::string::toBytes(
                              "From stefan@rocket.streu.home Wed Sep 27 22:17:32 2017\n"
                              "Return-path: <stefan@rocket.streu.home>\n"
                              "Date: Wed, 27 Sep 2017 22:17:32 +0200\n"
                              "From: Stefan Reuther <stefan@rocket.streu.home>\n"
                              "To: stefan@localhost\n"
                              "Subject: 3\n"
                              "Message-ID: <20170927201732.GD21431@rocket.streu.home>\n"
                              "MIME-Version: 1.0\n"
                              "Content-Type: multipart/mixed; boundary=\"xo44VMWPx7vlQ2+2\"\n"
                              "Content-Disposition: inline\n"
                              "\n"
                              "\n"
                              "--xo44VMWPx7vlQ2+2\n"
                              "Content-Type: text/plain; charset=us-ascii\n"
                              "Content-Disposition: inline\n"
                              "\n"
                              "3\n"
                              "\n"
                              "--xo44VMWPx7vlQ2+2\n"
                              "Content-Type: message/rfc822\n"
                              "Content-Disposition: inline\n"
                              "\n"
                              "Return-path: <stefan@rocket.streu.home>\n"
                              "Date: Wed, 27 Sep 2017 22:17:09 +0200\n"
                              "From: Stefan Reuther <stefan@rocket.streu.home>\n"
                              "To: stefan@localhost\n"
                              "Subject: 2\n"
                              "Message-ID: <20170927201709.GC21431@rocket.streu.home>\n"
                              "MIME-Version: 1.0\n"
                              "Content-Type: multipart/mixed; boundary=\"ZoaI/ZTpAVc4A5k6\"\n"
                              "Content-Disposition: inline\n"
                              "\n"
                              "\n"
                              "--ZoaI/ZTpAVc4A5k6\n"
                              "Content-Type: text/plain; charset=us-ascii\n"
                              "Content-Disposition: inline\n"
                              "\n"
                              "2\n"
                              "\n"
                              "--ZoaI/ZTpAVc4A5k6\n"
                              "Content-Type: message/rfc822\n"
                              "Content-Disposition: inline\n"
                              "\n"
                              "Return-path: <stefan@rocket.streu.home>\n"
                              "Date: Wed, 27 Sep 2017 22:16:46 +0200\n"
                              "From: Stefan Reuther <stefan@rocket.streu.home>\n"
                              "To: stefan@localhost\n"
                              "Subject: 1\n"
                              "Message-ID: <20170927201645.GB21431@rocket.streu.home>\n"
                              "MIME-Version: 1.0\n"
                              "Content-Type: multipart/mixed; boundary=\"jI8keyz6grp/JLjh\"\n"
                              "Content-Disposition: inline\n"
                              "\n"
                              "\n"
                              "--jI8keyz6grp/JLjh\n"
                              "Content-Type: text/plain; charset=us-ascii\n"
                              "Content-Disposition: inline\n"
                              "\n"
                              "1\n"
                              "\n"
                              "--jI8keyz6grp/JLjh\n"
                              "Content-Type: message/rfc822\n"
                              "Content-Disposition: inline\n"
                              "\n"
                              "Return-path: <stefan@rocket.streu.home>\n"
                              "Date: Wed, 27 Sep 2017 18:36:28 +0200\n"
                              "From: Stefan Reuther <stefan@rocket.streu.home>\n"
                              "To: stefan@localhost\n"
                              "Subject: test\n"
                              "Message-ID: <20170927163628.GA6110@rocket.streu.home>\n"
                              "MIME-Version: 1.0\n"
                              "Content-Type: multipart/mixed; boundary=\"LZvS9be/3tNcYl/X\"\n"
                              "Content-Disposition: inline\n"
                              "\n"
                              "\n"
                              "--LZvS9be/3tNcYl/X\n"
                              "Content-Type: text/plain; charset=us-ascii\n"
                              "Content-Disposition: inline\n"
                              "\n"
                              "the mail\n"
                              "\n"
                              "--LZvS9be/3tNcYl/X\n"
                              "Content-Type: application/octet-stream\n"
                              "Content-Disposition: attachment; filename=\"player2.trn\"\n"
                              "Content-Transfer-Encoding: base64\n"
                              "\n"
                              "AgAAAAAAMDYtMDItMjAxMjE5OjA3OjA0AACQA1ZFUjMuNTAxNo7TcqllgGnHrFJME0KOeQny\n"
                              "AAAAAAAA\n"
                              "\n"
                              "--LZvS9be/3tNcYl/X--\n"
                              "\n"
                              "--jI8keyz6grp/JLjh--\n"
                              "\n"
                              "--ZoaI/ZTpAVc4A5k6--\n"
                              "\n"
                              "--xo44VMWPx7vlQ2+2--\n"
                              "\n"
                              "\n"),
                          mail, host));

    // Verify
    std::auto_ptr<MailMock::Message> m(mail.extract());
    TS_ASSERT(m.get() != 0);
    TS_ASSERT_EQUALS(m->templateName, "turn");
    TS_ASSERT_EQUALS(m->receiver, "user:qq");
    TS_ASSERT_EQUALS(m->parameters["gameid"], "47");
    TS_ASSERT_EQUALS(m->parameters["gamename"], "Game 47");
    TS_ASSERT_EQUALS(m->parameters["mail_subject"], "3");
    TS_ASSERT_EQUALS(m->parameters["mail_path"], "/part2/part1/part2/part1/part2/part1/part2/player2.trn");

    // No more parts
    TS_ASSERT(mail.empty());
}

/** Test deep nesting.
    This exercises the DoS (maximum nesting) protection. */
void
TestServerMailinMailProcessor::testDeep()
{
    // Process mail.
    MailMock mail;
    HostMock host;
    host.setSuccess(47, 3, 1, "qq");
    TS_ASSERT(!processMail(afl::string::toBytes(
                               "Subject: test\n"
                               "From: stefan@localhost\n"
                               "Content-Type: multipart/mixed; boundary=10\n"
                               "\n"
                               "--10\n"
                               "Content-Type: multipart/mixed; boundary=9\n"
                               "\n"
                               "--9\n"
                               "Content-Type: multipart/mixed; boundary=8\n"
                               "\n"
                               "--8\n"
                               "Content-Type: multipart/mixed; boundary=7\n"
                               "\n"
                               "--7\n"
                               "Content-Type: multipart/mixed; boundary=6\n"
                               "\n"
                               "--6\n"
                               "Content-Type: multipart/mixed; boundary=5\n"
                               "\n"
                               "--5\n"
                               "Content-Type: multipart/mixed; boundary=4\n"
                               "\n"
                               "--4\n"
                               "Content-Type: multipart/mixed; boundary=3\n"
                               "\n"
                               "--3\n"
                               "Content-Type: multipart/mixed; boundary=2\n"
                               "\n"
                               "--2\n"
                               "Content-Type: multipart/mixed; boundary=1\n"
                               "\n"
                               "--1\n"
                               "Content-Type: application/octet-stream\n"
                               "Content-Disposition: attachment; filename=\"player2.trn\"\n"
                               "Content-Transfer-Encoding: base64\n"
                               "\n"
                               "AgAAAAAAMDYtMDItMjAxMjE5OjA3OjA0AACQA1ZFUjMuNTAxNo7TcqllgGnHrFJME0KOeQny\n"
                               "--1--\n"
                               "--2--\n"
                               "--3--\n"
                               "--4--\n"
                               "--5--\n"
                               "--6--\n"
                               "--7--\n"
                               "--8--\n"
                               "--9--\n"
                               "--10--\n"),
                           mail, host));

    // No mail sent
    TS_ASSERT(mail.empty());
}

