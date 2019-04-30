/**
  *  \file server/mailin/mailprocessor.cpp
  *  \brief Class server::mailin::MailProcessor
  */

#include <cstring>
#include "server/mailin/mailprocessor.hpp"
#include "afl/net/headerfield.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/log.hpp"
#include "server/interface/hostturnclient.hpp"

using afl::sys::Log;
using afl::string::Format;
using afl::net::MimeParser;

namespace {
    /* Logger name */
    const char LOG_NAME[] = "mailin";

    /* DoS protection: a mail with many nested attachments will cause processPart to be called recursively.
       Each nesting level uses about 60-70 bytes mail text (Content-Type header, two boundaries),
       and causes us to copy a MimeParser and consume about 600 bytes stack (x64 build).
       We must therefore limit the nesting level.
       Limiting the path length is a simple opportunity to add that, without adding additional depth tracking logic.
       50 bytes should be plenty (7 levels).

       It should be noted that as long as this code is instantiated in a new process for every mail,
       this is a rather harmless problem; the result of stack or heap overflow is the process being killed.
       However, once this code is used in a longer-lived or multithreaded process, it gets more serious. */
    const size_t MAX_PATH_LENGTH = 50;

    /* Convenience function to extract an address value. */
    String_t getAddress(const afl::net::HeaderField* hf)
    {
        if (hf == 0) {
            return String_t();
        }

        String_t result;
        if (!hf->getAddressValue(result)) {
            return String_t();
        }

        return result;
    }

    /* Quote a string. */
    String_t quote(String_t s)
    {
        for (size_t i = s.size(); i > 0; --i) {
            if (s[i-1] == '\\' || s[i-1] == '"') {
                s.insert(i-1, "\\");
            }
        }
        return s;
    }
}

/***************************** MailProcessor *****************************/

// Constructor.
server::mailin::MailProcessor::MailProcessor(afl::sys::LogListener& log,
                                             server::interface::MailQueue& mq,
                                             afl::net::CommandHandler& host)
    : m_log(log),
      m_mailQueue(mq),
      m_host(host)
{ }

// Process a mail message.
bool
server::mailin::MailProcessor::process(const afl::net::MimeParser& mail)
{
    // ex planetscentral/mailin/mailin.cc:processMail
    // Log the mail
    m_log.write(Log::Info, LOG_NAME, "Processing mail message:");
    logHeader(mail, "From");
    logHeader(mail, "To");
    logHeader(mail, "Date");
    logHeader(mail, "Message-Id");

    // Parse the "From" address
    String_t address = getAddress(mail.getHeaders().get("From"));
    if (address.empty()) {
        m_log.write(Log::Warn, LOG_NAME, "[reject] unable to figure out sender address");
        return false;
    }

    // Try to extract turn files
    try {
        if (processPart(mail, mail, address, String_t())) {
            // No log needed; successful branches will log [ok] or [reject].
            // If we're here user has got an email.
            return true;
        } else {
            m_log.write(Log::Info, LOG_NAME, "[reject] no usable content in message");
            return false;
        }
    }
    catch (std::exception& e) {
        m_log.write(Log::Warn, LOG_NAME, "[reject] exception", e);
        return false;
    }
}

// Convenience function to log a header value.
void
server::mailin::MailProcessor::logHeader(const afl::net::MimeParser& mail, const char* key)
{
    if (const afl::net::HeaderField* hf = mail.getHeaders().get(key)) {
        m_log.write(Log::Info, LOG_NAME, afl::string::Format("  %s: %s", key, hf->getValue()));
    }
}

/** Process a mail part.
    \param root Root MimeParser (containing the complete mail)
    \param part Part to process
    \param address Sender's mail address
    \param path Virtual path to this mail part */
bool
server::mailin::MailProcessor::processPart(const afl::net::MimeParser& root, const afl::net::MimeParser& part, const String_t& address, const String_t path)
{
    // ex planetscentral/mailin/mailin.cc:processMailPart

    // DoS protection
    if (path.size() >= MAX_PATH_LENGTH) {
        return false;
    }

    // Is it multipart?
    afl::base::Ptr<afl::base::Enumerator<MimeParser> > parts(part.getParts());
    if (parts.get() != 0) {
        // Process multipart
        int i = 0;
        bool result = false;
        MimeParser subpart;
        while (parts->getNextElement(subpart)) {
            ++i;
            result |= processPart(root, subpart, address, Format("%s/part%d", path, i));
        }
        return result;
    } else {
        // Try to process single part
        const String_t fn = afl::string::strLCase(part.getFileName().orElse(""));
        if (fn.size() > 4 && fn.compare(fn.size()-4, 4, ".trn", 4) == 0) {
            // We only process things that look like turn files.
            // This means only if a file called "*.trn" is attached, we will reply to the mail,
            // giving sufficient certainity to not work as a spam relay.
            return processTurnFile(root, part.getBodyAsString(), address, path + "/" + fn);
        } else {
            return false;
        }
    }
}

/** Process a turn file.
    \param root Root MimeParser (containing the complete mail)
    \param content Turn file content (decoded attachment, blob)
    \param address Sender's mail address
    \param path Virtual path to this mail part */
bool
server::mailin::MailProcessor::processTurnFile(const afl::net::MimeParser& root, const String_t& content, const String_t& address, const String_t path)
{
    // ex planetscentral/mailin/mailin.cc:processTurnFile

    // Find user agent for info string
    // X-Mailer: used by Outlook, Eudora, Pegasus, The Bat!, Lotus Notes, phpBB3, ...
    // User-Agent: used by Mozilla, Mutt, KMail, Opera Mail, Alpine, ...
    String_t ua = root.getHeader("X-Mailer").orElse("");
    if (ua.empty()) {
        ua = root.getHeader("User-Agent").orElse("");
    }

    // Submit a turn file command.
    server::interface::HostTurnClient turnClient(m_host);

    server::interface::HostTurn::Result result;
    try {
        result = turnClient.submit(content,
                                   afl::base::Nothing,       // game
                                   afl::base::Nothing,       // slot
                                   address,
                                   String_t(Format("mail: addr=\"%s\", ua=\"%s\", route=\"%s\"",
                                                   quote(address),
                                                   quote(ua),
                                                   quote(root.getTrace()))));
    }
    catch (std::runtime_error& e) {
        if (std::strncmp(e.what(), "407 ", 4) == 0) {
            // "407 Mail mismatch" happens when the player does not play in this game
            sendRejection(root, address, path, "turn-mismatch");
            return true;
        } else if (std::strncmp(e.what(), "404 ", 4) == 0 || std::strncmp(e.what(), "412 ", 4) == 0) {
            // "412 Wrong game state" happens when trying to submit a turn to a finished game
            // "404 Game does not exist" happens when then turn file does not match any game
            sendRejection(root, address, path, "turn-stale");
            return true;
        } else if (std::strncmp(e.what(), "422 ", 4) == 0) {
            // "422 Invalid file format"
            sendRejection(root, address, path, "turn-error");
            return true;
        } else {
            // Anything else, e.g. internal errors. Don't bother uses with those.
            throw;
        }
    }

    // Generate a reply
    m_log.write(Log::Info, LOG_NAME,
                Format("[ok] file '%s': user '%s', game '%d', slot %d, state %d")
                << path << result.userId << result.gameId << result.slot << result.state);

    // Send mail
    m_mailQueue.startMessage("turn", String_t(Format("turn-%s-%d-%d", result.userId, result.gameId, result.slot)));
    m_mailQueue.addParameter("gamename", result.gameName);
    m_mailQueue.addParameter("gameid", Format("%d", result.gameId));
    m_mailQueue.addParameter("gameturn", Format("%d", result.turnNumber));
    m_mailQueue.addParameter("slot", Format("%d", result.slot));
    m_mailQueue.addParameter("trn_status", Format("%d", result.state));
    m_mailQueue.addParameter("trn_output", result.output);
    m_mailQueue.addParameter("mail_subject", root.getHeader("Subject").orElse("(none)"));
    m_mailQueue.addParameter("mail_date", root.getHeader("Date").orElse("(none)"));
    m_mailQueue.addParameter("mail_messageid", root.getHeader("Message-Id").orElse("(none)"));
    m_mailQueue.addParameter("mail_path", path);

    const String_t receivers[] = { "user:" + result.userId };
    m_mailQueue.send(receivers);

    return true;
}

/** Send a rejection mail.
    \param root Root MimeParser (containing the complete mail)
    \param address Sender's mail address
    \param path Virtual path to rejected mail part
    \param tpl Name of mail template to use */
void
server::mailin::MailProcessor::sendRejection(const afl::net::MimeParser& root, const String_t& address, const String_t& path, const char* tpl)
{
    m_log.write(Log::Warn, LOG_NAME, Format("[reject] file '%s', %s", path, tpl));

    // Send mail
    // Do not use a uniquifier; this goes to a mail address, not a user, and thus is not queued
    m_mailQueue.startMessage(tpl, afl::base::Nothing);
    m_mailQueue.addParameter("mail_from", address);
    m_mailQueue.addParameter("mail_subject", root.getHeader("Subject").orElse("(none)"));
    m_mailQueue.addParameter("mail_date", root.getHeader("Date").orElse("(none)"));
    m_mailQueue.addParameter("mail_messageid", root.getHeader("Message-Id").orElse("(none)"));
    m_mailQueue.addParameter("mail_path", path);

    const String_t receivers[] = { "mail:" + address };
    m_mailQueue.send(receivers);
}
