/**
  *  \file server/interface/formatserver.cpp
  */

#include <stdexcept>
#include "server/interface/formatserver.hpp"
#include "interpreter/arguments.hpp"
#include "server/interface/format.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"

namespace {
    using server::toString;

    struct FormatConfig {
        String_t formatName;
        afl::data::Value* data;
        afl::base::Optional<String_t> format;
        afl::base::Optional<String_t> charset;
    };

    void parseFormatConfig(FormatConfig& result, interpreter::Arguments& args)
    {
        // ex FormatConfig::configure
        // Must have at least two args
        args.checkArgumentCountAtLeast(2);

        // Check format
        result.formatName = toString(args.getNext());

        // Remember data
        result.data = args.getNext();

        // Optional args
        while (args.getNumArgs() > 0) {
            String_t kw(afl::string::strUCase(toString(args.getNext())));
            if (kw == "FORMAT") {
                args.checkArgumentCountAtLeast(1);
                result.format = toString(args.getNext());
            } else if (kw == "CHARSET") {
                args.checkArgumentCountAtLeast(1);
                result.charset = toString(args.getNext());
            } else {
                throw std::runtime_error(server::INVALID_OPTION);
            }
        }
    }
}

server::interface::FormatServer::FormatServer(Format& impl)
    : CommandHandler(),
      m_implementation(impl)
{ }

server::interface::FormatServer::~FormatServer()
{ }

server::interface::FormatServer::Value_t*
server::interface::FormatServer::call(const Segment_t& command)
{
    // ex FormatConnection::handleCommand (sort-of)

    // Fetch command
    interpreter::Arguments args(command, 0, command.size());
    args.checkArgumentCountAtLeast(1);
    String_t cmd = afl::string::strUCase(toString(args.getNext()));

    // Dispatch command
    if (cmd == "PING") {
        /* @q PING (Format Command)
           Alive test.
           @retval Str "PONG". */
        return makeStringValue("PONG");
    } else if (cmd == "HELP") {
        /* @q HELP (Format Command)
           @retval Str Help page. */
        return makeStringValue("Commands:\n"
                               "  PING\n"
                               "  HELP\n"
                               "  PACK type data [options]\n"
                               "  UNPACK type data [options]\n"
                               "\n"
                               "Options:\n"
                               "  CHARSET cs\n"
                               "  FORMAT {json|obj}\n");
    } else if (cmd == "PACK") {
        /* @q PACK out:Format, data, [FORMAT in:Str, CHARSET cs:Str] (Format Command)
           Pack %data into a blob.
           - %in="obj" (default): %data is a RESP object.
           - %in="json": %data is a {@type Str|string} containing a JSON object.

           The character set defines the character encoding in the returned blob and defaults to Latin-1.

           @rettype Blob
           @argtype Str
           @see UNPACK (Format Command) */
        FormatConfig cfg;
        parseFormatConfig(cfg, args);
        return m_implementation.pack(cfg.formatName, cfg.data, cfg.format, cfg.charset);
    } else if (cmd == "UNPACK") {
        /* @q UNPACK in:Format, data:Blob, [FORMAT out:Str, CHARSET cs:Str] (Format Command)
           Unpack data from a blob.
           - %out="obj" (default): produce a RESP object.
           - %out="json": produce a {@type Str|string} containing a JSON object.

           The character set defines the character encoding in %data and defaults to Latin-1.

           @rettype Str
           @see PACK (Format Command) */
        FormatConfig cfg;
        parseFormatConfig(cfg, args);
        return m_implementation.unpack(cfg.formatName, cfg.data, cfg.format, cfg.charset);
    } else {
        throw std::runtime_error(UNKNOWN_COMMAND);
    }
}

void
server::interface::FormatServer::callVoid(const Segment_t& command)
{
    delete call(command);
}
