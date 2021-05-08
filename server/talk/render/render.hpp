/**
  *  \file server/talk/render/render.hpp
  *  \brief Rendering
  */
#ifndef C2NG_SERVER_TALK_RENDER_RENDER_HPP
#define C2NG_SERVER_TALK_RENDER_RENDER_HPP

#include <memory>
#include "afl/string/string.hpp"
#include "server/talk/root.hpp"
#include "server/talk/textnode.hpp"

namespace server { namespace talk { namespace render {

    class Context;
    class Options;

    /** Render text.
        The source text has the form "TYPE:TEXT", where TYPE is a supported text format.
        The output format is a TYPE, optionally prefixed by one or more transformations.
        If the both TYPEs agree, or if the output format is "raw", rendering is a null operation.

        \param text Text (with type tag)
        \param ctx  Rendering context
        \param opts Rendering options (includes output format)
        \param root Service root
        \return formatted text */
    String_t renderText(const String_t& text, const Context& ctx, const Options& opts, Root& root);

    /** Render pre-parsed text.
        \param tree Parsed text
        \param ctx  Rendering context
        \param opts Rendering options (includes output format)
        \param root Service root
        \return formatted text */
    String_t renderText(std::auto_ptr<TextNode> tree, const Context& ctx, const Options& opts, Root& root);

} } }

#endif
