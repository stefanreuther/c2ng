/**
  *  \file server/talk/render/options.hpp
  */
#ifndef C2NG_SERVER_TALK_RENDER_OPTIONS_HPP
#define C2NG_SERVER_TALK_RENDER_OPTIONS_HPP

#include "afl/string/string.hpp"
#include "server/interface/talkrender.hpp"

namespace server { namespace talk { namespace render {

    /** Renderer state, untrusted part.
        Untrusted attributes are provided by the user:
        - base URL
        - format
        Those can be set with each renderer invocation. */
    class Options {
     public:
        Options();

        void setBaseUrl(const String_t& baseUrl);
        const String_t& getBaseUrl() const;

        void setFormat(const String_t& format);
        const String_t& getFormat() const;

        void updateFrom(const server::interface::TalkRender::Options& other);

     private:
        String_t m_baseUrl;
        String_t m_format;
    };

} } }

#endif
