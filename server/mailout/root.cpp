/**
  *  \file server/mailout/root.cpp
  *  \brief Class server::mailout::Root
  */

#include "server/mailout/root.hpp"
#include "afl/charset/base64.hpp"
#include "afl/charset/urlencoding.hpp"
#include "afl/checksums/md5.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/time.hpp"
#include "server/mailout/configuration.hpp"
#include "server/mailout/message.hpp"
#include "server/mailout/transmitter.hpp"
#include "server/types.hpp"

using afl::string::Format;

namespace {
    const char*const EMAIL_ROOT = "email:";
    const char*const USER_ROOT = "user:";

    const char*const LOG_NAME_AUTH = "mailout.auth";
    const char*const LOG_NAME_QUEUE = "mailout.queue";

    String_t getConfirmLink(const server::mailout::Configuration& config, String_t user, String_t userEmail)
    {
        // Compute md5("$system_key,$user,$userEmail")
        static const uint8_t COMMA[] = {','};
        afl::checksums::MD5 md5;
        md5.add(afl::string::toBytes(config.confirmationKey));
        md5.add(COMMA);
        md5.add(afl::string::toBytes(user));
        md5.add(COMMA);
        md5.add(afl::string::toBytes(userEmail));

        // FIXME: can we get rid of this byte/char conversion madness?
        uint8_t result[afl::checksums::MD5::HASH_SIZE];
        String_t stringToEncode = user + "," + afl::string::fromBytes(md5.getHash(result));
        return afl::string::fromBytes(afl::charset::Base64().encode(afl::string::toMemory(stringToEncode)));
    }

    String_t encodeUrl(const String_t& str)
    {
        return afl::string::fromBytes(afl::charset::UrlEncoding().encode(afl::string::toMemory(str)));
    }
}

server::mailout::Root::Root(afl::net::CommandHandler& db, const Configuration& config)
    : server::common::Root(db),
      m_db(db),
      m_config(config),
      m_log(),
      m_transmitter(0)
{ }

afl::net::redis::Subtree
server::mailout::Root::mailRoot()
{
    // ex MAIL_ROOT
    return afl::net::redis::Subtree(m_db, "mqueue:");
}

const server::mailout::Configuration&
server::mailout::Root::config() const
{
    return m_config;
}

// Prepare mail queues.
void
server::mailout::Root::prepareQueues()
{
    // ex Transmitter::start
    // @change This originally was a function of Transmitter; moved to Root because it does not need Transmitter's privates.
    afl::net::redis::Subtree root(mailRoot());

    // Discard partially-prepared messages
    afl::data::IntegerList_t partial;
    root.intSetKey("preparing").getAll(partial);
    for (size_t i = 0; i < partial.size(); ++i) {
        Message(root, partial[i], "preparing").remove();
    }
    log().write(afl::sys::LogListener::Info, LOG_NAME_QUEUE, Format("%d partial messages deleted", partial.size()));

    // Trigger sending of outgoing mail
    if (Transmitter* p = getTransmitter()) {
        afl::data::IntegerList_t out;
        root.intSetKey("sending").getAll(out);
        for (size_t i = 0; i < out.size(); ++i) {
            p->send(out[i]);
        }
        log().write(afl::sys::LogListener::Info, LOG_NAME_QUEUE, Format("%d items initially in queue", out.size()));
    }
}

// Allocate a message.
std::auto_ptr<server::mailout::Message>
server::mailout::Root::allocateMessage()
{
    // ex planetscentral/mailout/message.cc:allocateMessage
    afl::net::redis::Subtree root(mailRoot());
    int32_t mid = ++root.subtree("msg").intKey("id");
    root.intSetKey("preparing").add(mid);

    std::auto_ptr<server::mailout::Message> result(new server::mailout::Message(root, mid, "preparing"));
    result->expireTime().set(getCurrentTime() + m_config.maximumAge);
    return result;
}

/** Request confirmation for an email address. */
void
server::mailout::Root::requestConfirmation(String_t user, String_t userEmail)
{
    // ex Transmitter::requestConfirmation
    // Mark that we requested confirmation
    afl::net::redis::HashKey emailInfo(afl::net::redis::Subtree(m_db, EMAIL_ROOT).subtree(userEmail).hashKey("status"));
    emailInfo.stringField("status/" + user).set("r");
    emailInfo.intField("expire/" + user).set(getCurrentTime() + 200*24*60 /* 200 days */);

    String_t userName = afl::net::redis::Subtree(m_db, USER_ROOT).subtree(user).stringKey("name").get();

    // Create message
    std::auto_ptr<Message> msg(allocateMessage());
    msg->uniqueId().set("confirmation-" + userEmail);
    msg->templateName().set("confirm");
    msg->arguments().stringField("email").set(userEmail);
    msg->arguments().stringField("user").set(userName);
    msg->arguments().stringField("confirmlink").set(m_config.baseUrl + "confirm.cgi?key="
                                                    + encodeUrl(getConfirmLink(m_config, user, userEmail)) + "&mail="
                                                    + encodeUrl(userEmail));
    msg->receivers().add("mail:" + userEmail);

    // Log it
    log().write(afl::sys::Log::Info, LOG_NAME_AUTH, Format("[msg:%d] confirmation request for '%s', user '%s' queued", msg->getId(), userEmail, user));
    msg->send();

    if (Transmitter* p = getTransmitter()) {
        p->send(msg->getId());
    }
}

// Resolve an address.
bool
server::mailout::Root::resolveAddress(String_t address, String_t& smtpAddress, String_t& authUser)
{
    // ex Transmitter::resolveAddress
    afl::net::redis::Subtree emailRoot(m_db, EMAIL_ROOT);

    if (address.size() > 5 && address.compare(0, 5, "mail:", 5) == 0) {
        // directly specified SMTP address
        smtpAddress.assign(address, 5, String_t::npos);
        authUser = "anon";

        // check blocked address
        afl::net::redis::HashKey emailInfo(emailRoot.subtree(smtpAddress).hashKey("status"));
        if (emailInfo.stringField("status/" + authUser).get() == "b") {
            throw std::runtime_error(Format("email address '%s' is blocked", smtpAddress));
        }
        return true;
    } else if (address.size() > 5 && address.compare(0, 5, "user:", 5) == 0) {
        // user Id
        String_t user(address, 5);

        // Fetch user email
        afl::net::redis::Subtree userRoot(m_db, USER_ROOT);
        String_t userEmail = userRoot.subtree(user).hashKey("profile").stringField("email").get();
        if (userEmail.empty()) {
            throw std::runtime_error(Format("user '%s' has no email address", user));
        }

        // Check email status
        afl::net::redis::HashKey emailInfo(emailRoot.subtree(userEmail).hashKey("status"));
        String_t emailStatus = emailInfo.stringField("status/" + user).get();
        if (emailStatus.empty() || emailStatus == "u") {
            // Unconfirmed. Request confirmation.
            requestConfirmation(user, userEmail);
            return false;
        } else if (emailStatus == "r") {
            // Confirmation is requested. Check for expiration.
            int32_t expire = emailInfo.intField("expire/" + user).get();
            if (expire != 0 && getCurrentTime() > expire) {
                requestConfirmation(user, userEmail);
            }
            return false;
        } else if (emailStatus == "c") {
            // Address is confirmed, so return it
            smtpAddress = userEmail;
            authUser = user;
            return true;
        } else {
            // treat as blocked
            throw std::runtime_error(Format("user '%s's email address '%s' is blocked", user, userEmail));
        }
    } else {
        throw std::runtime_error(Format("invalid address '%s'", address));
    }
}

// Confirm an email address.
bool
server::mailout::Root::confirmMail(String_t mail, String_t key, String_t info)
{
    // ex planetscentral/mailout/transmitter.cc:confirmMail
    // Find user Id
    String_t decoded = afl::charset::Base64().decode(afl::string::toBytes(key));
    String_t::size_type n = decoded.find(',');
    if (n == 0 || n >= decoded.size()) {
        log().write(afl::sys::Log::Info, LOG_NAME_AUTH, Format("request for '%s' is syntactically invalid", mail.c_str()));
        return false;
    }
    String_t user(decoded, 0, n);

    // Check hash
    if (key != getConfirmLink(m_config, user, mail)) {
        log().write(afl::sys::Log::Info, LOG_NAME_AUTH, Format("request for '%s' lacks proper signature", mail.c_str()));
        return false;
    }

    // FIXME: should we verify that this actually IS the user's current email address?
    // User may have changed their address in the meantime.

    // OK, operate
    afl::net::redis::HashKey emailInfo(afl::net::redis::Subtree(m_db, EMAIL_ROOT).subtree(mail).hashKey("status"));
    emailInfo.stringField("status/" + user).set("c");
    if (info.empty()) {
        emailInfo.stringField("confirm/" + user).remove();
    } else {
        emailInfo.stringField("confirm/" + user).set(info);
    }
    log().write(afl::sys::Log::Info, LOG_NAME_AUTH, Format("request for '%s' user '%s' accepted", mail.c_str(), user.c_str()));
    return true;
}

// Get user's email status.
server::interface::MailQueue::UserStatus
server::mailout::Root::getUserStatus(String_t user)
{
    using server::interface::MailQueue;

    MailQueue::UserStatus result;

    afl::net::redis::Subtree emailRoot(m_db, EMAIL_ROOT);
    afl::net::redis::Subtree userRoot(m_db, USER_ROOT);
    String_t userEmail = userRoot.subtree(user).hashKey("profile").stringField("email").get();
    if (!userEmail.empty()) {
        afl::net::redis::HashKey emailInfo(emailRoot.subtree(userEmail).hashKey("status"));
        String_t emailStatus = emailInfo.stringField("status/" + user).get();

        result.address = userEmail;
        result.status = (emailStatus.empty()
                         ? MailQueue::Unconfirmed
                         : MailQueue::parseAddressStatus(emailStatus));
    }
    return result;
}

// Get current time.
int32_t
server::mailout::Root::getCurrentTime()
{
    return packTime(afl::sys::Time::getCurrentTime());
}
