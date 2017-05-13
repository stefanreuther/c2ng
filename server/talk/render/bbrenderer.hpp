/**
  *  \file server/talk/render/bbrenderer.hpp
  */
#ifndef C2NG_SERVER_TALK_RENDER_BBRENDERER_HPP
#define C2NG_SERVER_TALK_RENDER_BBRENDERER_HPP

#include "server/talk/textnode.hpp"
#include "server/talk/root.hpp"
#include "server/talk/inlinerecognizer.hpp"

namespace server { namespace talk { namespace render {

    class Context;
    class Options;

    String_t renderBB(const TextNode& node, const Context& ctx, const Options& opts, Root& root, InlineRecognizer::Kinds_t kinds);

} } }

#endif
