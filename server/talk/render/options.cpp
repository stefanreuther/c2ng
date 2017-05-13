/**
  *  \file server/talk/render/options.cpp
  */

#include "server/talk/render/options.hpp"

server::talk::render::Options::Options()
    : m_baseUrl(),
      m_format("raw")
{
    // ex RenderState::RenderState
}

void
server::talk::render::Options::setBaseUrl(const String_t& baseUrl)
{
    // ex RenderState::setBaseUrl
    m_baseUrl = baseUrl;
}

const String_t&
server::talk::render::Options::getBaseUrl() const
{
    // ex RenderState::getBaseUrl
    return m_baseUrl;
}

void
server::talk::render::Options::setFormat(const String_t& format)
{
    // ex RenderState::setFormat
    m_format = format;
}

const String_t&
server::talk::render::Options::getFormat() const
{
    return m_format;
}

void
server::talk::render::Options::updateFrom(const server::interface::TalkRender::Options& other)
{
    if (const String_t* p = other.baseUrl.get()) {
        setBaseUrl(*p);
    }
    if (const String_t* p = other.format.get()) {
        setFormat(*p);
    }
}
