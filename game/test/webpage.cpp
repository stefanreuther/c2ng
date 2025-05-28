/**
  *  \file game/test/webpage.cpp
  *  \brief Class game::test::WebPage
  */

#include "game/test/webpage.hpp"
#include "util/stringparser.hpp"

using afl::net::HeaderField;
using util::StringParser;

game::test::WebPage::WebPage(afl::base::Memory<const Response> responses)
    : m_responses(responses)
{ }

bool
game::test::WebPage::isValidMethod(const String_t& /*method*/) const
{
    return true;
}

bool
game::test::WebPage::isValidPath() const
{
    return false;
}

void
game::test::WebPage::handleRequest(afl::net::http::PageRequest& in, afl::net::http::PageResponse& out)
{
    const Response* resp = findResponse(in);
    if (resp != 0) {
        const afl::base::ConstBytes_t response = afl::string::toBytes(resp->responseBody);

        // Default headers
        out.headers().add("Content-Type", "text/plain");

        // Custom headers
        if (resp->responseHeaders != 0) {
            StringParser p(resp->responseHeaders);
            while (!p.parseEnd()) {
                String_t name, value;
                p.parseDelim(":", name);
                p.parseCharacter(':');
                p.parseDelim("|", value);
                p.parseCharacter('|');
                out.headers().set(name, value);
            }
        }

        out.body().handleFullData(response);
        out.setStatusCode(afl::net::http::PageResponse::OK);
    } else {
        out.setStatusCode(afl::net::http::PageResponse::NOT_FOUND);
        out.headers().add("Content-Type", "text/plain");
        out.body().handleFullData(afl::string::toBytes("Not found"));
    }
}

const game::test::WebPage::Response*
game::test::WebPage::findResponse(const afl::net::http::PageRequest& req) const
{
    for (size_t i = 0; i < m_responses.size(); ++i) {
        const Response* p = m_responses.at(i);
        if (matchResponse(req, *p)) {
            return p;
        }
    }
    return 0;
}

bool
game::test::WebPage::matchResponse(const afl::net::http::PageRequest& req, const Response& resp)
{
    return matchMethod(req.getMethod(), resp.methods)
        && matchHeaders(req.headers(), resp.requestHeaders)
        && matchHeaders(req.arguments(), resp.requestParameters);
}

bool
game::test::WebPage::matchMethod(const String_t& method, const char* allowed)
{
    if (allowed == 0) {
        return true;
    }

    StringParser p(allowed);
    while (!p.parseEnd()) {
        String_t tok;
        p.parseDelim(",", tok);
        if (method == tok) {
            return true;
        }
        p.parseCharacter(',');
    }

    return false;
}

bool
game::test::WebPage::matchHeaders(const afl::net::HeaderTable& headers, const char* required)
{
    if (required == 0) {
        return true;
    }

    StringParser p(required);
    while (!p.parseEnd()) {
        String_t name, value;
        p.parseDelim(":", name);
        p.parseCharacter(':');
        p.parseDelim("|", value);
        p.parseCharacter('|');

        const HeaderField* f = headers.get(name);
        if (f == 0) {
            return false;
        }
        if (f->getValue() != value) {
            return false;
        }
    }

    return true;
}
