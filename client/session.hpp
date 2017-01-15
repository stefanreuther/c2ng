/**
  *  \file client/session.hpp
  */
#ifndef C2NG_CLIENT_SESSION_HPP
#define C2NG_CLIENT_SESSION_HPP

#include "ui/root.hpp"
#include "client/si/userside.hpp"
#include "afl/base/uncopyable.hpp"

namespace client {

    /** Client session.
        This is the user-interface side of a game session which bundles everything needed to build a GUI that talks to the game session.
        It lives in the user-interface thread. */
    class Session : afl::base::Uncopyable {
     public:
        /** Constructor.
            \param root User-interface root (includes graphics engine with dispatcher back to our thread).
            \param gameSender Sender to send requests to the game session.
            \param tx Translator */
        Session(ui::Root& root,
                util::RequestSender<game::Session> gameSender,
                afl::string::Translator& tx,
                util::MessageCollector& console,
                afl::sys::Log& mainLog);

        /** Get user-interface root.
            \return root */
        ui::Root& root();

        /** Get game session sender.
            This can be used to send requests to the game session.
            \return sender */
        util::RequestSender<game::Session> gameSender();

        /** Get client session dispatcher.
            This dispatcher can be used to send requests back to this session.
            This is the same as root().engine().dispatcher().
            \return dispatcher */
        util::RequestDispatcher& dispatcher();

        /** Get script interface user side.
            \return interface user side */
        client::si::UserSide& interface();

        /** Get translator.
            \return translator */
        afl::string::Translator& translator();

     private:
        ui::Root& m_root;
        client::si::UserSide m_interface;
        afl::string::Translator& m_translator;
    };

}

#endif
