/**
  *  \file server/talk/render/mailrenderer.hpp
  */
#ifndef C2NG_SERVER_TALK_RENDER_MAILRENDERER_HPP
#define C2NG_SERVER_TALK_RENDER_MAILRENDERER_HPP

#include "server/talk/textnode.hpp"
#include "server/talk/root.hpp"

namespace server { namespace talk { namespace render {

    class Context;
    class Options;

    String_t renderMail(TextNode* node, const Context& ctx, const Options& opts, Root& root, bool forNNTP);

} } }

#endif
