/**
  *  \file server/talk/render/textrenderer.hpp
  */
#ifndef C2NG_SERVER_TALK_RENDER_TEXTRENDERER_HPP
#define C2NG_SERVER_TALK_RENDER_TEXTRENDERER_HPP

#include "server/talk/textnode.hpp"
#include "server/talk/root.hpp"

namespace server { namespace talk { namespace render {

    class Context;

    String_t renderText(TextNode* node, const Context& ctx, Root& root);

} } }

#endif
