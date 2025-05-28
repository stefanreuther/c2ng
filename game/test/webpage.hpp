/**
  *  \file game/test/webpage.hpp
  *  \brief Class game::test::WebPage
  */
#ifndef C2NG_GAME_TEST_WEBPAGE_HPP
#define C2NG_GAME_TEST_WEBPAGE_HPP

#include "afl/base/memory.hpp"
#include "afl/net/headertable.hpp"
#include "afl/net/http/page.hpp"
#include "afl/net/http/pagerequest.hpp"
#include "afl/net/http/pageresponse.hpp"

namespace game { namespace test {

    /** Simple test web page.
        This class is intended to be used with game::test::WebServer.
        It produces text output, optionally chosen depending on method, headers, and parameters.
        Match and output are specified in a static structure, making no attempt to be generic for production use. */
    class WebPage : public afl::net::http::Page {
     public:
        /** A possible response.
            From an array of responses, WebPage picks the first matching one.
            If none matches, a NOT_FOUND (404) error is generated. */
        struct Response {
            /** Required method.
                Null to accept any method, or a comma-separated list of methods such as "GET,HEAD". */
            const char* methods;

            /** Required headers.
                Null to require none, or a list of "Field:Value|Field:Value|...".
                All required headers must be present with the exact value. */
            const char* requestHeaders;

            /** Required parameters.
                Null to require none, or a list of "Field:Value|Field:Value|...".
                All required parameters must be present with the exact value. */
            const char* requestParameters;

            /** Response headers.
                Null for none, or a list of "Field:Value|Field:Value|...".
                If no Content-Type header is given, text/plain is assumed. */
            const char* responseHeaders;

            /** Response body.
                Just the content of the body. */
            const char* responseBody;
        };

        /** Constructor.
            @param responses Static array of responses. */
        explicit WebPage(afl::base::Memory<const Response> responses);

        // Page:
        virtual bool isValidMethod(const String_t& method) const;
        virtual bool isValidPath() const;
        virtual void handleRequest(afl::net::http::PageRequest& in, afl::net::http::PageResponse& out);

     private:
        afl::base::Memory<const Response> m_responses;

        const Response* findResponse(const afl::net::http::PageRequest& req) const;
        static bool matchResponse(const afl::net::http::PageRequest& req, const Response& resp);
        static bool matchMethod(const String_t& method, const char* allowed);
        static bool matchHeaders(const afl::net::HeaderTable& headers, const char* required);
    };

} }

#endif
