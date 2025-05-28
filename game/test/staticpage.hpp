/**
  *  \file game/test/staticpage.hpp
  *  \brief Class game::test::StaticPage
  */
#ifndef C2NG_GAME_TEST_STATICPAGE_HPP
#define C2NG_GAME_TEST_STATICPAGE_HPP

#include "afl/base/memory.hpp"
#include "afl/net/http/page.hpp"

namespace game { namespace test {

    /** Simple static test web page.
        This class is intended to be used with game::test::WebServer.
        It produces a static output, defined using a ConstBytes_t.
        It is intended for testing, and makes no attempt to be generic for production use. */
    class StaticPage : public afl::net::http::Page {
     public:
        /** Constructor.
            @param contentType Content-Type of page content (e.g. application/octet-stream).
            @param content Page content. */
        explicit StaticPage(String_t contentType, afl::base::ConstBytes_t content);

        // Page:
        virtual bool isValidMethod(const String_t& method) const;
        virtual bool isValidPath() const;
        virtual void handleRequest(afl::net::http::PageRequest& in, afl::net::http::PageResponse& out);

     private:
        String_t m_contentType;
        afl::base::ConstBytes_t m_content;
    };

} }


#endif
