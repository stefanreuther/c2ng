/**
  *  \file server/talk/talkrender.cpp
  *  \brief Class server::talk::TalkRender
  */

#include "server/talk/talkrender.hpp"
#include "server/talk/render/render.hpp"
#include "server/talk/render/context.hpp"

server::talk::TalkRender::TalkRender(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

void
server::talk::TalkRender::setOptions(const Options& opts)
{
    // Update session's render options
    m_session.renderOptions().updateFrom(opts);
}

String_t
server::talk::TalkRender::render(const String_t& text, const Options& opts)
{
    // Render using temporary options
    server::talk::render::Options temporaryOptions(m_session.renderOptions());
    temporaryOptions.updateFrom(opts);

    // Context
    server::talk::render::Context ctx(m_root, m_session.getUser());
    return server::talk::render::renderText(text, ctx, temporaryOptions, m_root);
}

void
server::talk::TalkRender::check(const String_t& text, std::vector<Warning>& out)
{
    server::talk::render::Context ctx(m_root, m_session.getUser());
    return server::talk::render::renderCheck(text, ctx, m_root, out);
}
