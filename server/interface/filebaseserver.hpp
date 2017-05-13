/**
  *  \file server/interface/filebaseserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_FILEBASESERVER_HPP
#define C2NG_SERVER_INTERFACE_FILEBASESERVER_HPP

#include "server/interface/filebase.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class FileBaseServer : public ComposableCommandHandler {
     public:
        FileBaseServer(FileBase& impl);
        ~FileBaseServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static Value_t* packInfo(const FileBase::Info& info);

     private:
        FileBase& m_implementation;
    };
    
} }

#endif
