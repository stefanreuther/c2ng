/**
  *  \file util/doc/htmlrenderer.hpp
  *  \brief HTML renderer
  */
#ifndef C2NG_UTIL_DOC_HTMLRENDERER_HPP
#define C2NG_UTIL_DOC_HTMLRENDERER_HPP

#include "afl/io/xml/node.hpp"
#include "afl/string/string.hpp"

namespace util { namespace doc {

    class RenderOptions;

    /** Render documentation nodes as HTML.

        The generated HTML behaves as follows:
        - used tags: a, b, br, div (for image slices), dl, dt, em, h2, h3, h4,
          img, kbd, li, ol, p, pre, small, span, table, td, th, tr, tt, u, ul
        - used classes: color-X (replaces <font color=X>)

        @param nodes List of nodes making up a document
        @param opts  Render options
        @return Formatted HTML */
    String_t renderHTML(const afl::io::xml::Nodes_t& nodes, const RenderOptions& opts);

} }

#endif
