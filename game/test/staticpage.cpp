/**
  *  \file game/test/staticpage.cpp
  *  \brief Class game::test::StaticPage
  */

#include "game/test/staticpage.hpp"
#include "afl/net/http/pagerequest.hpp"
#include "afl/net/http/pageresponse.hpp"

game::test::StaticPage::StaticPage(String_t contentType,
                                   afl::base::ConstBytes_t content)
    : m_contentType(contentType),
      m_content(content)
{ }

bool
game::test::StaticPage::isValidMethod(const String_t& method) const
{
    return method == "GET" || method == "HEAD";
}

bool
game::test::StaticPage::isValidPath() const
{
    return false;
}

void
game::test::StaticPage::handleRequest(afl::net::http::PageRequest& /*in*/, afl::net::http::PageResponse& out)
{
    out.headers().set("Content-Type", m_contentType);
    out.body().handleFullData(m_content);
}
