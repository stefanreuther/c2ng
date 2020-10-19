/**
  *  \file server/interface/filegameclient.hpp
  *  \brief Class server::interface::FileGameClient
  */
#ifndef C2NG_SERVER_INTERFACE_FILEGAMECLIENT_HPP
#define C2NG_SERVER_INTERFACE_FILEGAMECLIENT_HPP

#include "server/interface/filegame.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    /** Game file client.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class FileGameClient : public FileGame {
     public:
        /** Constructor.
            \param commandHandler Server connection. Lifetime must exceed that of the FileGameClient. */
        explicit FileGameClient(afl::net::CommandHandler& commandHandler);

        /** Destructor. */
        ~FileGameClient();

        // FileGame:
        virtual void getGameInfo(String_t path, GameInfo& result);
        virtual void listGameInfo(String_t path, afl::container::PtrVector<GameInfo>& result);
        virtual void getKeyInfo(String_t path, KeyInfo& result);
        virtual void listKeyInfo(String_t path, const Filter& filter, afl::container::PtrVector<KeyInfo>& result);

        /** Unpack GameInfo from transferred object.
            \param [out] result result
            \param [in]  p      received object */
        static void unpackGameInfo(GameInfo& result, const afl::data::Value* p);

        /** Unpack KeyInfo from transferred object.
            \param [out] result result
            \param [in]  p      received object */
        static void unpackKeyInfo(KeyInfo& result, const afl::data::Value* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
