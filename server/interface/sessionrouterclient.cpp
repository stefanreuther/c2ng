/**
  *  \file server/interface/sessionrouterclient.cpp
  */

#include "server/interface/sessionrouterclient.hpp"
#include "afl/net/line/client.hpp"
#include "afl/string/format.hpp"
#include "afl/net/line/linehandler.hpp"
#include "afl/net/line/linesink.hpp"
#include "afl/except/remoteerrorexception.hpp"
#include "afl/except/invaliddataexception.hpp"
#include "afl/string/messages.hpp"
#include "afl/string/parse.hpp"
#include "afl/net/line/simplequery.hpp"

namespace {
    class OneLineCommand : public afl::net::line::LineHandler {
     public:
        OneLineCommand(const String_t& cmd)
            : m_command(cmd)
            { }
        virtual bool handleOpening(afl::net::line::LineSink& response)
            {
                response.handleLine(m_command);
                return false;
            }
        virtual bool handleLine(const String_t& line, afl::net::line::LineSink& /*response*/)
            {
                m_result = line;
                return true;
            }
        virtual void handleConnectionClose()
            { }

        const String_t& getResult() const
            { return m_result; }

     private:
        String_t m_command;
        String_t m_result;
    };
}


server::interface::SessionRouterClient::SessionRouterClient(afl::net::NetworkStack& net, afl::net::Name name)
    : m_networkStack(net),
      m_name(name)
{ }

server::interface::SessionRouterClient::~SessionRouterClient()
{
}

String_t
server::interface::SessionRouterClient::getStatus()
{
    afl::net::line::SimpleQuery cmd("LIST");
    call(cmd);
    return cmd.getResult();
}

String_t
server::interface::SessionRouterClient::getInfo(int32_t sessionId)
{
    afl::net::line::SimpleQuery cmd(afl::string::Format("INFO %d", sessionId));
    call(cmd);
    return cmd.getResult();
}

String_t
server::interface::SessionRouterClient::talk(int32_t sessionId, String_t command)
{
    // S. Talks to a session and produces a result.
    afl::net::line::SimpleQuery cmd(afl::string::Format("S %d\n%s", sessionId, command));
    call(cmd);

    afl::string::ConstStringMemory_t result = afl::string::toMemory(cmd.getResult());
    afl::string::ConstStringMemory_t firstLine = result.split(result.find('\n'));
    if (!firstLine.subrange(0, 4).equalContent(afl::string::toMemory("200 "))) {
        throw afl::except::RemoteErrorException(m_name.toString(), afl::string::fromMemory(firstLine));
    }
    
    return afl::string::fromMemory(result.subrange(1));
}

void
server::interface::SessionRouterClient::sessionAction(int32_t sessionId, Action action)
{
    // CLOSE/RESTART/SAVE/SAVENN with session Id
    // CLOSE/SAVE/SAVENN: produces
    // - "200 OK, n sessions closed/saved"
    // RESTART: produces
    // - "200 OK"
    // - "500 Restart failed"
    // - "452 Session timed out"
    OneLineCommand cmd(afl::string::Format("%s %d", formatAction(action), sessionId));
    call(cmd);
    if (cmd.getResult().compare(0, 4, "200 ", 4) != 0) {
        throw afl::except::RemoteErrorException(m_name.toString(), cmd.getResult());
    }
}

void
server::interface::SessionRouterClient::groupAction(String_t key, Action action, afl::data::IntegerList_t& result)
{
    // CLOSE/RESTART/SAVE/SAVENN with group key.
    class GroupCommand : public afl::net::line::LineHandler {
     public:
        GroupCommand(String_t cmd, afl::data::IntegerList_t& result)
            : m_command(cmd),
              m_first(true),
              m_result(result)
            { }
        virtual bool handleOpening(afl::net::line::LineSink& response)
            {
                response.handleLine(m_command);
                return false;
            }
        virtual bool handleLine(const String_t& line, afl::net::line::LineSink& /*response*/)
            {
                if (m_first) {
                    m_first = false;
                } else {
                    int32_t n;
                    if (afl::string::strToInteger(line, n)) {
                        m_result.push_back(n);
                    }
                }
                return false;
            }
        virtual void handleConnectionClose()
            { }
     private:
        String_t m_command;
        bool m_first;
        afl::data::IntegerList_t& m_result;
    };
    GroupCommand cmd(afl::string::Format("%s -%s", formatAction(action), key), result);
    call(cmd);
}

int32_t
server::interface::SessionRouterClient::create(afl::base::Memory<const String_t> args)
{
    // NEW. Returns session Id.
    // Build the command.
    String_t commandLine = "NEW";
    while (const String_t* p = args.eat()) {
        commandLine += ' ';
        commandLine += *p;
    }

    // Call it. Result will be "201 n Created" or an error message.
    OneLineCommand cmd(commandLine);
    call(cmd);
    if (cmd.getResult().compare(0, 4, "201 ", 4) != 0) {
        throw afl::except::RemoteErrorException(m_name.toString(), cmd.getResult());
    }

    // Extract the session Id
    String_t result = cmd.getResult().substr(4);
    String_t::size_type pos = 0;
    int32_t sessionId = 0;
    if (!afl::string::strToInteger(result, sessionId, pos)) {
        if (!afl::string::strToInteger(result.substr(0, pos), sessionId, pos)) {
            throw afl::except::InvalidDataException(afl::string::Messages::invalidNumber());
        }
    }
    return sessionId;
}

String_t
server::interface::SessionRouterClient::getConfiguration()
{
    afl::net::line::SimpleQuery cmd("CONFIG");
    call(cmd);
    return cmd.getResult();
}

void
server::interface::SessionRouterClient::call(afl::net::line::LineHandler& hdl)
{
    afl::net::line::Client(m_networkStack, m_name).call(hdl);
}
