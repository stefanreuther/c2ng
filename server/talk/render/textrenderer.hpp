/**
  *  \file server/talk/render/textrenderer.hpp
  *  \brief Text renderer
  */
#ifndef C2NG_SERVER_TALK_RENDER_TEXTRENDERER_HPP
#define C2NG_SERVER_TALK_RENDER_TEXTRENDERER_HPP

#include "server/talk/linkparser.hpp"
#include "server/talk/textnode.hpp"

namespace server { namespace talk { namespace render {

    /** Render node as plaintext.

        This function is mainly used for generating abstracts.
        Unlike TextNode::getTextContent(), it fills in game names etc.
        Unlike renderMail(), it only produces raw text output, no markup.

        @param node  Node
        @param lp    Link parser
        @return text */
    String_t renderPlainText(const TextNode& node, const LinkParser& lp);

} } }

#endif
