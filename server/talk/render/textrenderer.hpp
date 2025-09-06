/**
  *  \file server/talk/render/textrenderer.hpp
  */
#ifndef C2NG_SERVER_TALK_RENDER_TEXTRENDERER_HPP
#define C2NG_SERVER_TALK_RENDER_TEXTRENDERER_HPP

#include "server/talk/linkparser.hpp"
#include "server/talk/textnode.hpp"

namespace server { namespace talk { namespace render {

    String_t renderText(TextNode* node, const LinkParser& lp);

} } }

#endif
