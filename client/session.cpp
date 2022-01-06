/**
  *  \file client/session.cpp
  */

#include "client/session.hpp"
#include "client/si/commands.hpp"

client::Session::Session(ui::Root& root,
                         util::RequestSender<game::Session> gameSender,
                         afl::string::Translator& tx,
                         util::MessageCollector& console,
                         afl::sys::Log& mainLog)
    : m_root(root),
      m_interface(root, gameSender, tx, root.engine().dispatcher(), console, mainLog),
      m_translator(tx)
{
    registerCommands(m_interface);
}

ui::Root&
client::Session::root()
{
    return m_root;
}

util::RequestSender<game::Session>
client::Session::gameSender()
{
    return m_interface.gameSender();
}

util::RequestDispatcher&
client::Session::dispatcher()
{
    return m_root.engine().dispatcher();
}

client::si::UserSide&
client::Session::interface()
{
    return m_interface;
}

afl::string::Translator&
client::Session::translator()
{
    return m_translator;
}
