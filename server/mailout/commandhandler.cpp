/**
  *  \file server/mailout/commandhandler.cpp
  *  \brief Class server::mailout::CommandHandler
  */

#include "server/mailout/commandhandler.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "server/common/session.hpp"
#include "server/interface/mailqueueserver.hpp"
#include "server/mailout/mailqueue.hpp"
#include "server/mailout/root.hpp"
#include "server/mailout/session.hpp"
#include "server/types.hpp"

namespace {
    const char*const LOG_NAME = "mailout.command";
}

server::mailout::CommandHandler::CommandHandler(Root& root, Session& session)
    : afl::net::CommandHandler(),
      m_root(root),
      m_session(session)
{ }

server::mailout::CommandHandler::Value_t*
server::mailout::CommandHandler::call(const Segment_t& command)
{
    // Log it
    String_t line;
    if (m_session.currentMessage.get() != 0) {
        line = afl::string::Format("[msg:%d]", m_session.currentMessage->getId());
    }
    for (size_t i = 0, n = command.size(); i < n; ++i) {
        if (!line.empty()) {
            line += ' ';
        }
        line += server::common::Session::formatWord(toString(command[i]), false);
    }
    m_root.log().write(afl::sys::Log::Info, LOG_NAME, line);

    // Process it
    MailQueue mq(m_root, m_session);
    return server::interface::MailQueueServer(mq).call(command);
}

void
server::mailout::CommandHandler::callVoid(const Segment_t& command)
{
    delete call(command);
}
