/**
  *  \file server/file/filegame.hpp
  */
#ifndef C2NG_SERVER_FILE_FILEGAME_HPP
#define C2NG_SERVER_FILE_FILEGAME_HPP

#include "server/interface/filegame.hpp"

namespace server { namespace file {

    class Session;
    class Root;

    class FileGame : public server::interface::FileGame {
     public:
        /** Constructor.
            \param session Session object (provides user context)
            \param root Root (provides file space, logging, config) */
        FileGame(Session& session, Root& root);

        virtual void getGameInfo(String_t path, GameInfo& result);
        virtual void listGameInfo(String_t path, afl::container::PtrVector<GameInfo>& result);
        virtual void getKeyInfo(String_t path, KeyInfo& result);
        virtual void listKeyInfo(String_t path, const Filter& filter, afl::container::PtrVector<KeyInfo>& result);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
