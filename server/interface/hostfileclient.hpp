/**
  *  \file server/interface/hostfileclient.hpp
  *  \brief Class server::interface::HostFileClient
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTFILECLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTFILECLIENT_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/hostfile.hpp"

namespace server { namespace interface {

    /** Client for host file access.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class HostFileClient : public HostFile {
     public:
        /** Constructor.
            @param commandHandler Server connection. Lifetime must exceed that of the HostFileClient. */
        explicit HostFileClient(afl::net::CommandHandler& commandHandler);

        // HostFile:
        virtual String_t getFile(String_t fileName);
        virtual void getDirectoryContent(String_t dirName, InfoVector_t& result);
        virtual Info getFileInformation(String_t fileName);
        virtual void getPathDescription(String_t dirName, InfoVector_t& result);

        /** Unpack a serialized Info structure.
            @param p Value received from server
            @return Info */
        static Info unpackInfo(const afl::data::Value* p);

        /** Unpack an array of serialized Info structures.
            @param [in]  p       Value received from server; array of name/value pairs
            @param [out] result  Result values placed here */
        static void unpackInfos(const afl::data::Value* p, InfoVector_t& result);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
