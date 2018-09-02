/**
  *  \file server/interface/hostfileclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTFILECLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTFILECLIENT_HPP

#include "server/interface/hostfile.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class HostFileClient : public HostFile {
     public:
        explicit HostFileClient(afl::net::CommandHandler& commandHandler);

        virtual String_t getFile(String_t fileName);
        virtual void getDirectoryContent(String_t dirName, InfoVector_t& result);
        virtual Info getFileInformation(String_t fileName);
        virtual void getPathDescription(String_t dirName, InfoVector_t& result);

        static Info unpackInfo(const afl::data::Value* p);
        static void unpackInfos(const afl::data::Value* p, InfoVector_t& result);
        
     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
