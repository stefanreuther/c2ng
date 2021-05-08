/**
  *  \file game/proxy/inboxadaptor.hpp
  *  \brief Inbox Adaptors
  */
#ifndef C2NG_GAME_PROXY_INBOXADAPTOR_HPP
#define C2NG_GAME_PROXY_INBOXADAPTOR_HPP

#include "afl/base/closure.hpp"
#include "game/proxy/mailboxadaptor.hpp"
#include "game/session.hpp"

namespace game { namespace proxy {

    typedef afl::base::Closure<MailboxAdaptor*(Session&)> InboxAdaptor_t;

    /** Make (creator for) inbox adaptor.
        Use with RequestSender<Session>::makeTemporary to create a RequestSender<MailboxAdaptor>
        that talks to the current turn's inbox.
        \return newly allocated closure */
    InboxAdaptor_t* makeInboxAdaptor();

    /** Make (creator for) planet's inbox messages.
        Use with RequestSender<Session>::makeTemporary to create a RequestSender<MailboxAdaptor>
        that talks to the messages associated with the given planet.
        \param planetId Id
        \return newly allocated closure */
    InboxAdaptor_t* makePlanetInboxAdaptor(Id_t planetId);

    /** Make (creator for) ship's inbox messages.
        Use with RequestSender<Session>::makeTemporary to create a RequestSender<MailboxAdaptor>
        that talks to the messages associated with the given ship.
        \param shipId Id
        \return newly allocated closure */
    InboxAdaptor_t* makeShipInboxAdaptor(Id_t shipId);

} }

#endif
