/**
  *  \file server/talk/render/htmlrenderer.hpp
  *  \brief HTML Rendering
  */
#ifndef C2NG_SERVER_TALK_RENDER_HTMLRENDERER_HPP
#define C2NG_SERVER_TALK_RENDER_HTMLRENDERER_HPP

#include "server/talk/root.hpp"
#include "server/talk/textnode.hpp"

namespace server { namespace talk { namespace render {

    class Context;
    class Options;

    /** Render HTML.
        @param node   Text to render
        @param ctx    Context (for LinkParser, getUser())
        @param opts   Options
        @param root   Root (for LinkFormatter, DB access)
        @return result (HTML UTF-8 string) */
    String_t renderHTML(const TextNode& node, const Context& ctx, const Options& opts, Root& root);

} } }

#endif
