/**
  *  \file server/talk/render/bbrenderer.hpp
  *  \brief BBCode renderer
  */
#ifndef C2NG_SERVER_TALK_RENDER_BBRENDERER_HPP
#define C2NG_SERVER_TALK_RENDER_BBRENDERER_HPP

#include "server/talk/inlinerecognizer.hpp"
#include "server/talk/textnode.hpp"

namespace server { namespace talk { namespace render {

    class Context;
    class Options;

    /** Render text as BBCode.
        Produces code that replicates a copy of the given node, if possible, when parsed by BBParser.
        @param node  Node to render
        @param recog InlineRecognizer instance
        @param kinds InlineRecognizer options (for formatting smileys) */
    String_t renderBB(const TextNode& node, const InlineRecognizer& recog, InlineRecognizer::Kinds_t kinds);

} } }

#endif
