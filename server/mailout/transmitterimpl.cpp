/**
  *  \file server/mailout/transmitterimpl.cpp
  *  \brief Class server::mailout::TransmitterImpl
  */

#include <memory>
#include "server/mailout/transmitterimpl.hpp"
#include "afl/io/textfile.hpp"
#include "afl/net/mimebuilder.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/mutexguard.hpp"
#include "server/mailout/message.hpp"
#include "server/mailout/root.hpp"
#include "server/mailout/template.hpp"

using afl::string::Format;

namespace {
    const char*const LOG_NAME = "mailout.transmit";
    const char*const THREAD_NAME = "mailout.transmit";
}

/************************* TransmitterImpl::Data *************************/

inline
server::mailout::TransmitterImpl::Data::Data()
    : m_wake(0),
      m_mutex(),
      m_stopRequest(false),
      m_workQueue(),
      m_postponedMessages()
{ }

inline bool
server::mailout::TransmitterImpl::Data::isStopRequested()
{
    afl::sys::MutexGuard g(m_mutex);
    return m_stopRequest;
}

inline void
server::mailout::TransmitterImpl::Data::requestStop()
{
    afl::sys::MutexGuard g(m_mutex);
    m_stopRequest = true;
    m_wake.post();
}

inline bool
server::mailout::TransmitterImpl::Data::getNextWork(int32_t& msgId)
{
    afl::sys::MutexGuard g(m_mutex);
    if (m_workQueue.empty()) {
        return false;
    } else {
        msgId = m_workQueue.front();
        return true;
    }
}

inline void
server::mailout::TransmitterImpl::Data::addToWork(int32_t msgId)
{
    // ex Transmitter::send
    afl::sys::MutexGuard g(m_mutex);
    m_workQueue.push_back(msgId);
    m_wake.post();
}

inline void
server::mailout::TransmitterImpl::Data::removeFromWork(int32_t msgId)
{
    afl::sys::MutexGuard g(m_mutex);
    m_workQueue.remove(msgId);
}

inline void
server::mailout::TransmitterImpl::Data::moveToPending(int32_t msgId)
{
    afl::sys::MutexGuard g(m_mutex);
    m_workQueue.remove(msgId);
    m_postponedMessages.push_back(msgId);
}

inline void
server::mailout::TransmitterImpl::Data::movePendingToWork()
{
    // ex Transmitter::runQueue
    // We're moving the queue in one mutexed transaction, to avoid that a parallel process moves them back
    // causing this process to run forever. We still move them elementwise to be able to post the semaphore.
    afl::sys::MutexGuard g(m_mutex);
    while (!m_postponedMessages.empty()) {
        m_workQueue.splice(m_workQueue.end(), m_postponedMessages, m_postponedMessages.begin());
        m_wake.post();
    }
}

inline void
server::mailout::TransmitterImpl::Data::wait()
{
    m_wake.wait();
}

/**************************** TransmitterImpl ****************************/

server::mailout::TransmitterImpl::TransmitterImpl(Root& root,
                                                  afl::base::Ref<afl::io::Directory> templateDir,
                                                  afl::net::NetworkStack& net,
                                                  afl::net::Name smtpAddress,
                                                  const afl::net::smtp::Configuration& smtpConfig)
    : m_thread(THREAD_NAME, *this),
      m_root(root),
      m_templateDirectory(templateDir),
      m_smtpClient(net, smtpAddress, smtpConfig),
      m_smtpConfig(smtpConfig),
      m_networkStack(net),
      m_data()
{
    // ex Transmitter::Transmitter

    // Logger
    m_smtpClient.log().addListener(m_root.log());

    // Start worker thread
    m_thread.start();
}

server::mailout::TransmitterImpl::~TransmitterImpl()
{
    stop();
    m_thread.join();

    m_smtpClient.log().removeListener(m_root.log());
}

// Send a message. Called after an element is added to the Sending queue.
void
server::mailout::TransmitterImpl::send(int32_t messageId)
{
    m_data.addToWork(messageId);
}

// Reconsider mails to an address for sending.
void
server::mailout::TransmitterImpl::notifyAddress(String_t /*address*/)
{
    // ex Transmitter::notifyAddress
    // Simple and stupid: just reconsider all messages
    runQueue();
}

// Reconsider all messages for sending.
void
server::mailout::TransmitterImpl::runQueue()
{
    m_data.movePendingToWork();
}

void
server::mailout::TransmitterImpl::run()
{
    // ex Transmitter::entry
    while (1) {
        m_data.wait();
        if (m_data.isStopRequested()) {
            break;
        }
        try {
            processWork();
        }
        catch (std::exception& e) {
            const bool inShutdown = m_data.isStopRequested();
            m_root.log().write(inShutdown ? afl::sys::LogListener::Info : afl::sys::LogListener::Warn, LOG_NAME, "exception in transmitter", e);
            if (!inShutdown) {
                afl::sys::Thread::sleep(2000);
            }
        }
    }
}

void
server::mailout::TransmitterImpl::stop()
{
    m_data.requestStop();
}

void
server::mailout::TransmitterImpl::processWork()
{
    // ex Transmitter::processWork
    // Fetch message id
    int32_t mid;
    if (!m_data.getNextWork(mid)) {
        return;
    }

    // Obtain message object
    Message msg(m_root, mid, Message::Sending);

    // Still active?
    bool active = true;
    String_t uid = msg.uniqueId().get();
    if (!uid.empty() && m_root.uniqueIdMap().intField(uid).get() != mid) {
        m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, Format("[msg:%d] expired (replaced by new instance)", mid));
        active = false;
    }

    if (m_root.getCurrentTime() > msg.expireTime().get()) {
        m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, Format("[msg:%d] expired (too old)", mid));
        active = false;
    }

    if (!active) {
        msg.remove();
        m_data.removeFromWork(mid);
        return;
    }

    // Get receivers
    afl::data::StringList_t receivers;
    msg.receivers().getAll(receivers);

    // Send it
    bool keep = false;
    for (size_t i = 0; i < receivers.size(); ++i) {
        bool keepThis = false;
        try {
            if (!sendMessage(msg, receivers[i])) {
                keepThis = true;
                m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, Format("[msg:%d] receiver '%s' postponed", mid, receivers[i]));
            } else {
                m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, Format("[msg:%d] receiver '%s' succeeded", mid, receivers[i]));
            }
        }
        catch (std::exception& e) {
            m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, Format("[msg:%d] receiver '%s' failed", mid, receivers[i]), e);
            if (m_data.isStopRequested()) {
                // Exception may be caused by shutdown; better keep the message
                keepThis = true;
            }
        }
        if (!keepThis) {
            // Message failed or succeeded: remove from database
            msg.receivers().remove(receivers[i]);
        } else {
            // Message postponed: keep it
            keep = true;
        }
    }

    // Postprocess
    if (keep) {
        // Keep message because it has unverified addresses
        m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, Format("[msg:%d] keeping", mid));
        m_data.moveToPending(mid);
    } else {
        // Discard message because it has been sent or permanently failed
        m_root.log().write(afl::sys::LogListener::Info, LOG_NAME, Format("[msg:%d] finished", mid));
        msg.remove();
        m_data.removeFromWork(mid);
    }
}

bool
server::mailout::TransmitterImpl::sendMessage(Message& msg, String_t address)
{
    // ex Transmitter::sendMessage
    // Resolve email address
    String_t smtpAddress;
    String_t authUser;
    if (!m_root.resolveAddress(address, smtpAddress, authUser)) {
        return false;
    }

    // Prepare message
    String_t tplName;
    Template tpl;
    tpl.addVariable("SMTP_FROM", m_smtpConfig.from);
    tpl.addVariable("SMTP_FQDN", m_smtpConfig.hello);
    tpl.addVariable("SMTP_TO", smtpAddress);
    tpl.addVariable("USER", authUser);
    tpl.addVariable("CGI_ROOT", m_root.config().baseUrl);

    {
        afl::data::StringList_t args;
        msg.arguments().getAll(args);
        for (size_t i = 0; i+1 < args.size(); i += 2) {
            tpl.addVariable(args[i], args[i+1]);
        }
    }
    {
        afl::data::StringList_t atts;
        msg.attachments().getAll(atts);
        for (size_t i = 0; i < atts.size(); ++i) {
            tpl.addFile(atts[i]);
        }
    }
    tplName = msg.templateName().get();

    // Generate
    afl::base::Ref<afl::io::Stream> s = m_templateDirectory->openFile(tplName, afl::io::FileSystem::OpenRead);
    afl::io::TextFile tf(*s);
    std::auto_ptr<afl::net::MimeBuilder> smtpMessage(tpl.generate(tf, m_networkStack, authUser, smtpAddress));

    // Send
    const String_t to[] = {smtpAddress};
    m_smtpClient.send(to, *smtpMessage);

    return true;
}
