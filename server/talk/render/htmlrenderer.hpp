/**
  *  \file server/talk/render/htmlrenderer.hpp
  */
#ifndef C2NG_SERVER_TALK_RENDER_HTMLRENDERER_HPP
#define C2NG_SERVER_TALK_RENDER_HTMLRENDERER_HPP

#include "server/talk/textnode.hpp"
#include "server/talk/root.hpp"

namespace server { namespace talk { namespace render {

    class Context;
    class Options;

    String_t renderHTML(const TextNode& node, const Context& ctx, const Options& opts, Root& root);

} } }

#endif
