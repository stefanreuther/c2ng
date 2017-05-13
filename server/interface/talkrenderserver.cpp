/**
  *  \file server/interface/talkrenderserver.cpp
  */

#include <stdexcept>
#include "server/interface/talkrenderserver.hpp"
#include "interpreter/arguments.hpp"
#include "afl/string/string.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"

server::interface::TalkRenderServer::TalkRenderServer(TalkRender& impl)
    : m_implementation(impl)
{ }

server::interface::TalkRenderServer::~TalkRenderServer()
{ }

bool
server::interface::TalkRenderServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "RENDEROPTION") {
        /* @q RENDEROPTION [FORMAT fmt:Str] [BASEURL url:Str] (Talk Command)
           Set renderer options.
           Options are used for all future rendering jobs on this connection that do not specify an override.

           The %url is used to generate links.

           The %fmt can be one of:
           - raw: do not render; just produce the raw {@type TalkText}.
           - format: do not render; just produce the type of the {@type TalkText}.
           - html: render HTML.
           - mail: render into internet email.
           - news: render into a Usenet posting.
           - text: produce just the raw text.
           - format<em>LS</em>: produce BBcode (with auto-link, auto-smiley option as given).
           If %fmt equals the format of the original {@type TalkText},
           the original text is returned as-is.

           %fmt can also contain modifiers:
           - quote:<em>format</em>: quote the text and render it in the given format.
           - noquote:<em>format</em>: remove all quotes and render the result in the given format.
           - break:<em>format</em>: render only up to the given break indicator.
           - abstract:<em>format</em>: render an abstract.
           - force:<em>format</em>: force rendering even if input and output format are the same.

           Permissions: none. */
        TalkRender::Options opts;
        parseOptions(args, opts);
        m_implementation.setOptions(opts);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "RENDER") {
        /* @q RENDER text:TalkText [renderOptions...] (Talk Command)
           Render text.

           The message is rendered using the current render options, see {RENDEROPTION}.
           You can temporarily override rendering options by specifying the new settings within the command.

           Permissions: none.

           @retval Str rendered text */
        args.checkArgumentCountAtLeast(1);
        String_t text = toString(args.getNext());

        TalkRender::Options opts;
        parseOptions(args, opts);
        result.reset(makeStringValue(m_implementation.render(text, opts)));
        return true;
    } else {
        return false;
    }
}

void
server::interface::TalkRenderServer::parseOptions(interpreter::Arguments& args, TalkRender::Options& opts)
{
    // ex RenderState::handleOption (sort-of)
    while (args.getNumArgs() > 0) {
        const String_t name = afl::string::strUCase(toString(args.getNext()));
        if (name == "FORMAT") {
            args.checkArgumentCountAtLeast(1);
            opts.format = toString(args.getNext());
        } else if (name == "BASEURL") {
            args.checkArgumentCountAtLeast(1);
            opts.baseUrl = toString(args.getNext());
        } else {
            throw std::runtime_error(INVALID_OPTION);
        }
    }
}
