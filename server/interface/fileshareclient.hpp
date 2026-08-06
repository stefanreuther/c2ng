/**
  *  \file server/interface/fileshareclient.hpp
  *  \brief Class server::interface::FileShareClient
  */
#ifndef C2NG_SERVER_INTERFACE_FILESHARECLIENT_HPP
#define C2NG_SERVER_INTERFACE_FILESHARECLIENT_HPP

#include <memory>
#include "afl/net/commandhandler.hpp"
#include "server/interface/fileshare.hpp"

namespace server { namespace interface {

    /** Client for file sharing information.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class FileShareClient : public FileShare {
     public:
        /** Constructor.
            @param commandHandler Server connection. Lifetime must exceed that of the UserManagementClient. */
        explicit FileShareClient(afl::net::CommandHandler& hdl);

        // FileShare:
        virtual int32_t getSequenceNumber(String_t userId);
        virtual void listDirectories(String_t userId, afl::container::PtrVector<Info>& result);
        virtual void listGameInfo(String_t userId, afl::container::PtrVector<FileGame::GameInfo>& result);

        /** Unpack a value into an Info.
            @param p Value (usually a hash)
            @return Newly allocated Info */
        static std::auto_ptr<Info> unpackInfo(const afl::data::Value* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
