/**
  *  \file server/interface/filegameclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_FILEGAMECLIENT_HPP
#define C2NG_SERVER_INTERFACE_FILEGAMECLIENT_HPP

#include "server/interface/filegame.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class FileGameClient : public FileGame {
     public:
        FileGameClient(afl::net::CommandHandler& commandHandler);
        ~FileGameClient();

        virtual void getGameInfo(String_t path, GameInfo& result);
        virtual void listGameInfo(String_t path, afl::container::PtrVector<GameInfo>& result);
        virtual void getKeyInfo(String_t path, KeyInfo& result);
        virtual void listKeyInfo(String_t path, afl::container::PtrVector<KeyInfo>& result);

        static void unpackGameInfo(GameInfo& result, const afl::data::Value* p);
        static void unpackKeyInfo(KeyInfo& result, const afl::data::Value* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
