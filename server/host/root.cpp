/**
  *  \file server/host/root.cpp
  *  \brief Class server::host::Root
  */

#include "server/host/root.hpp"
#include "afl/net/reconnectable.hpp"
#include "afl/sys/time.hpp"
#include "server/host/cron.hpp"

namespace {
    void configure(afl::net::CommandHandler& hdl)
    {
        if (afl::net::Reconnectable* rc = dynamic_cast<afl::net::Reconnectable*>(&hdl)) {
            rc->setReconnectMode(afl::net::Reconnectable::Once);
        }
    }
}

server::host::Root::ToolTree::ToolTree(afl::net::redis::Subtree tree)
    : m_tree(tree)
{ }

afl::net::redis::StringSetKey
server::host::Root::ToolTree::all()
{
    return m_tree.stringSetKey("list");
}

afl::net::redis::HashKey
server::host::Root::ToolTree::byName(String_t name)
{
    return m_tree.subtree("prog").hashKey(name);
}

afl::net::redis::StringKey
server::host::Root::ToolTree::defaultName()
{
    return m_tree.stringKey("default");
}

/********************************** Root *********************************/

server::host::Root::Root(afl::net::CommandHandler& db,
                         afl::net::CommandHandler& hostFile,
                         afl::net::CommandHandler& userFile,
                         server::interface::MailQueue& mailQueue,
                         util::ProcessRunner& checkturnRunner,
                         afl::io::FileSystem& fs,
                         const Configuration& config)
    : server::common::Root(db),
      m_log(),
      m_mutex(),
      m_db(db),
      m_hostFile(hostFile),
      m_userFile(userFile),
      m_mailQueue(mailQueue),
      m_arbiter(),
      m_checkturnRunner(checkturnRunner),
      m_fileSystem(fs),
      m_pTalkListener(0),
      m_pCron(0),
      m_pRouter(0),
      m_config(config),
      m_rng(afl::sys::Time::getTickCounter())
{ }

server::host::Root::~Root()
{ }

afl::sys::Log&
server::host::Root::log()
{
    return m_log;
}

afl::sys::Mutex&
server::host::Root::mutex()
{
    return m_mutex;
}

void
server::host::Root::configureReconnect()
{
    // What to reconnect?
    // - database is stateless
    // - host file, user file are stateful and could cause a command to be executed in wrong user context if the connection drops mid-way
    // - mail queue is stateful. However, since we only have the interface reference, we cannot access the underlying CommandHandler.
    //   However, the worst thing that can happen if the connection drops midway is that a result mail gets lost,
    //   which I consider acceptable.
    configure(m_hostFile);
    configure(m_userFile);
}

void
server::host::Root::setCron(Cron* p)
{
    m_pCron = p;
}

void
server::host::Root::setForum(TalkListener* p)
{
    m_pTalkListener = p;
}

void
server::host::Root::setRouter(server::interface::SessionRouter* p)
{
    m_pRouter = p;
}

afl::net::CommandHandler&
server::host::Root::hostFile()
{
    // ex file_rc, file_connection
    return m_hostFile;
}

afl::net::CommandHandler&
server::host::Root::userFile()
{
    // ex userfile_rc, userfile_connection
    return m_userFile;
}

server::host::TalkListener*
server::host::Root::getForum()
{
    // ex talk_rc, talk_connection
    return m_pTalkListener;
}

server::interface::MailQueue&
server::host::Root::mailQueue()
{
    // ex mail_rc, mail_connection
    return m_mailQueue;
}

server::host::GameArbiter&
server::host::Root::arbiter()
{
    return m_arbiter;
}

const server::host::Configuration&
server::host::Root::config() const
{
    return m_config;
}

util::RandomNumberGenerator&
server::host::Root::rng()
{
    return m_rng;
}


util::ProcessRunner&
server::host::Root::checkturnRunner()
{
    // ex checkturn_runner
    return m_checkturnRunner;
}

afl::io::FileSystem&
server::host::Root::fileSystem()
{
    return m_fileSystem;
}

// /** Get current time. We store MINUTES since epoch, giving us a little longer
//     than 2038 if time() works right.

//     This is therefore a minutes counter since Thu Jan 1 1970, 0:00.
//     To obtain minutes: %60.
//     To obtain hours: /60%24. */
server::Time_t
server::host::Root::getTime()
{
    // ex planetscentral/host/schedule.h:getCurrentTime
    return Time_t(afl::sys::Time::getCurrentTime().getUnixTime() / m_config.timeScale);
}

afl::sys::Time
server::host::Root::getSystemTimeFromTime(Time_t t)
{
    // ex planetscentral/host/schedule.h:getSystemTimeFromTime
    return afl::sys::Time::fromUnixTime(t * m_config.timeScale);
}

server::host::Cron*
server::host::Root::getCron()
{
    return m_pCron;
}

void
server::host::Root::handleGameChange(int32_t gameId)
{
    if (Cron* p = getCron()) {
        p->handleGameChange(gameId);
    }
}

server::interface::SessionRouter*
server::host::Root::getRouter()
{
    return m_pRouter;
}

void
server::host::Root::tryCloseRouterSessions(String_t key)
{
    static const char LOG_NAME[] = "host.router";
    if (m_pRouter != 0) {
        try {
            afl::data::IntegerList_t tmp;
            m_pRouter->groupAction(key, server::interface::SessionRouter::Close, tmp);
        }
        catch (std::exception& e) {
            m_log.write(afl::sys::LogListener::Info, LOG_NAME, "router failure", e);
        }
    }
}

server::host::Root::ToolTree
server::host::Root::hostRoot()
{
    // ex HOST_PROGRAM_ROOT
    return afl::net::redis::Subtree(m_db, "prog:host:");
}

server::host::Root::ToolTree
server::host::Root::masterRoot()
{
    // ex MASTER_PROGRAM_ROOT
    return afl::net::redis::Subtree(m_db, "prog:master:");
}

server::host::Root::ToolTree
server::host::Root::shipListRoot()
{
    // ex SHIPLIST_PROGRAM_ROOT
    return afl::net::redis::Subtree(m_db, "prog:sl:");
}

server::host::Root::ToolTree
server::host::Root::toolRoot()
{
    // ex TOOL_PROGRAM_ROOT
    return afl::net::redis::Subtree(m_db, "prog:tool:");
}

afl::net::redis::StringSetKey
server::host::Root::activeUsers()
{
    // ex USER_ACTIVE
    return afl::net::redis::StringSetKey(m_db, "user:active");
}

afl::net::redis::StringListKey
server::host::Root::globalHistory()
{
    // ex GLOBAL_HISTORY
    return afl::net::redis::StringListKey(m_db, "global:history");
}
