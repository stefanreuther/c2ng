/**
  *  \file server/talk/render/mailrenderer.hpp
  *  \brief Mail renderer
  */
#ifndef C2NG_SERVER_TALK_RENDER_MAILRENDERER_HPP
#define C2NG_SERVER_TALK_RENDER_MAILRENDERER_HPP

#include "server/talk/linkparser.hpp"
#include "server/talk/root.hpp"
#include "server/talk/textnode.hpp"

namespace server { namespace talk { namespace render {

    class Options;

    /** Render text.

        Mail output is produced for two purposes:
        - notification mails
        - NNTP messages

        Those are subtly different.
        Whereas notifications preserve some more forum specifics,
        NNTP messages are converted into a format which a skilled NNTP user prefers.
        In particular, post references are turned into message-Id links,
        and forum references are turned into newsgroup names.

        @param node      Text to render
        @param lp        LinkParser instance
        @param opts      Options
        @param root      Root (for LinkFormat and DB access)
        @param forNNTP   true if this message is to be published on NNTP; false for mail
        @return formatted text */
    String_t renderMail(const TextNode& node, const LinkParser& lp, const Options& opts, Root& root, bool forNNTP);

} } }

#endif
