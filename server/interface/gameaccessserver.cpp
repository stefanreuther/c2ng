/**
  *  \file server/interface/gameaccessserver.cpp
  */

#include <memory>
#include "server/interface/gameaccessserver.hpp"
#include "afl/string/format.hpp"
#include "afl/data/defaultvaluefactory.hpp"
#include "afl/string/char.hpp"
#include "afl/net/line/linesink.hpp"
#include "server/interface/gameaccess.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/bufferedstream.hpp"
#include "afl/io/json/parser.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/json/writer.hpp"
#include "server/errors.hpp"

using afl::string::Format;

namespace {
    bool isNumeric(const char* p)
    {
        return afl::string::charIsDigit(p[0])
            && afl::string::charIsDigit(p[1])
            && afl::string::charIsDigit(p[2]);
    }

    String_t formatException(const char* tpl, const char* exception)
    {
        if (isNumeric(exception)) {
            return exception;
        } else {
            return Format("%s (%s)", tpl, exception);
        }
    }
}


server::interface::GameAccessServer::GameAccessServer(GameAccess& impl)
    : m_implementation(impl),
      m_state(Normal),
      m_postTarget(),
      m_postBody()
{ }

bool
server::interface::GameAccessServer::handleOpening(afl::net::line::LineSink& response)
{
    response.handleLine("100 OK");
    return false;
}

bool
server::interface::GameAccessServer::handleLine(const String_t& line, afl::net::line::LineSink& response)
{
    // ex server/server.cc:play, server/postobj.cc:doPostObject
    switch (m_state) {
     case Normal:
        try {
            String_t command;
            String_t arg;
            String_t::size_type split = line.find(' ');
            if (split != String_t::npos) {
                command.assign(line, 0, split);
                arg.assign(line, split+1, String_t::npos);
            } else {
                command = line;
            }
            if (afl::string::strCaseCompare(command, "HELP") == 0) {
                // ex doHelp / HELP: just respond with message
                response.handleLine("200 OK, help follows");
                sendResponse(response,
                             "  HELP   this message\n"
                             "  SAVE   save data\n"
                             "  STAT   status (show console)\n"
                             "  GET x  data access\n"
                             "     obj/id,id,id   dynamic JSON data\n"
                             "     item/id        rendered static data\n"
                             "     query/q        query\n"
                             "  POST x data modify (content follows)\n"
                             "     obj/id         modify object");
                return false;
            } else if (afl::string::strCaseCompare(command, "SAVE") == 0) {
                /* @q SAVE (Game Access Command)
                   Save current data.
                   Returns a success response. */
                // ex doSave / SAVE: save, no parameters
                m_implementation.save();
                response.handleLine("100 OK, data saved");
                return false;
            } else if (afl::string::strCaseCompare(command, "STAT") == 0) {
                /* @q STAT (Game Access Command)
                   Get status.
                   Produces information useful for debugging. */
                // ex doStat
                String_t reply = m_implementation.getStatus();
                response.handleLine("200 OK");
                sendResponse(response, reply);
                return false;
            } else if (afl::string::strCaseCompare(command, "GET") == 0) {
                /* @q GET path:Str (Game Access Command)
                   Fetch an object. Possible paths are:
                   - obj/id (regular game object)
                   - query/id (dynamic data)
                   @rettype Any */
                // ex doGet / GET: one parameter, URL
                std::auto_ptr<Value_t> reply(m_implementation.get(arg));
                response.handleLine("200 OK");
                sendResponse(response, reply.get());
                return false;
            } else if (afl::string::strCaseCompare(command, "POST") == 0) {
                /* @q POST path:Str (Game Access Command)
                   Update an object.
                   Takes a payload consisting of a list of commands.
                   Each command is a list consisting of a command verb and parameters. */
                // ex doPost / POST: stash away data
                m_state = Posting;
                m_postTarget = arg;
                return false;
            } else if (afl::string::strCaseCompare(command, "QUIT") == 0) {
                /* @q QUIT (Game Access Command)
                   Terminate this session. */
                return true;
            } else {
                response.handleLine(UNKNOWN_COMMAND);
                return false;
            }
        }
        catch (std::exception& e) {
            response.handleLine(formatException(INTERNAL_ERROR, e.what()));
            return false;
        }

     case Posting:
        if (line == ".") {
            try {
                // Parse JSON
                afl::data::DefaultValueFactory factory;
                afl::io::ConstMemoryStream ms(afl::string::toBytes(m_postBody));
                afl::io::BufferedStream bs(ms);
                std::auto_ptr<Value_t> value(afl::io::json::Parser(bs, factory).parseComplete());

                // Call user
                try {
                    std::auto_ptr<Value_t> reply(m_implementation.post(m_postTarget, value.get()));
                    response.handleLine("200 OK");
                    sendResponse(response, reply.get());
                }
                catch (std::exception& e) {
                    response.handleLine(formatException(INTERNAL_ERROR, e.what()));
                }
            }
            catch (std::exception& e) {
                // @change This is now 400 Syntax error; was 412 before.
                response.handleLine(formatException(SYNTAX_ERROR, e.what()));
            }

            m_postBody.clear();
            m_postTarget.clear();
            m_state = Normal;
        } else {
            m_postBody += line;
            m_postBody += '\n';
        }
        return false;
    }
    return true;
}

void
server::interface::GameAccessServer::handleConnectionClose()
{
    // Nothing to do
}

void
server::interface::GameAccessServer::sendResponse(afl::net::line::LineSink& response, Value_t* value)
{
    afl::io::InternalSink sink;
    afl::io::json::Writer writer(sink);
    writer.setLineLength(80);
    writer.visit(value);

    sendMemoryResponse(response, sink.getContent());
}

void
server::interface::GameAccessServer::sendResponse(afl::net::line::LineSink& response, const String_t& lines)
{
    sendMemoryResponse(response, afl::string::toBytes(lines));
}

void
server::interface::GameAccessServer::sendMemoryResponse(afl::net::line::LineSink& response, afl::base::ConstBytes_t mem)
{
    while (1) {
        size_t n = mem.find('\n');
        response.handleLine(afl::string::fromBytes(mem.split(n)));
        if (mem.empty()) {
            break;
        }
        mem.split(1);
    }
    response.handleLine(".");
}
